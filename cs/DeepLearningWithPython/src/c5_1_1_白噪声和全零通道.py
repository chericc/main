# 5.1.1 泛化：无意义特征对训练的影响
#
# 书中原文（doc/5.1-泛化-机器学习的目标.md）：
#   「对于 MNIST 数据集，将 784 个白噪声维度连接到现有的 784 个维度中，从而创建一个新的数据集。
#     连接无意义的特征根本不影响数据所包含的信息，我们只是添加了一些内容。
#     人类分类时不会受到这些变换的影响。」
#
# 本脚本做三次对照实验（网络结构与超参完全相同，只有输入维度不同）：
#   1. baseline：原始 784 维像素。
#   2. +784 全零通道：维度翻倍但不增加任何信息。
#      因为输入恒为 0，对应权重的梯度 dL/dw = dL/dz * 0 = 0，训练全程不动，
#      对输出也没有贡献，所以训练出的模型应与 baseline 几乎一样
#      （微小差异只来自 Glorot 初始化尺度随输入维度变化、以及浮点求和顺序）。
#   3. +784 白噪声通道：人类看同样是"多了一点没用的东西"，但模型会变差。
#      第一层参数从 400k 涨到 800k，模型有了更多机会去拟合与标签无关的随机涨落
#      （虚假相关性）；同时等量噪声稀释了有效信号，在同样的轮数下收敛也更慢。
#      于是验证曲线抖动更大、测试精度下降。
#
# 结论：连接无意义特征不改变数据的信息量，但改变了模型的容量与它能看到的特征空间，
#      所以任何多出来的输入维度都是过拟合的入口。
#
# ------------------------------------------------------------------ 关于速度
# 本机 i5-1135G7（4 核 8 线程）实测：
#   * TensorFlow 的 intra-op 线程池默认就用满所有核心（cpu/wall ≈ 2.8x），
#     手动设 TF_NUM_INTRAOP_THREADS / tf.config.threading.set_* 没有收益。
#   * XLA（jit_compile=True）对这种小 Dense 反而更慢。
#   * 真正有效的三条：
#       1) batch_size 从 128 提到 256：矩阵乘效率更高，约 1.5x；
#       2) 三个实验互相独立 -> 用多进程并行跑（每个进程 4 个线程），约 1.5~2x；
#       3) 训练集指标直接取 history 最后一轮的训练态结果，省掉一次全量 evaluate。
# 所以「多线程」的正确姿势是并行的**进程**，而不是继续给单个模型加线程。
# 三个实验的随机种子固定，并行与串行得到的结果完全一致。

import os

# 必须在导入 keras/tensorflow 之前设置，否则不生效（c2_1 里这一行放在了 import 之后）
os.environ["CUDA_VISIBLE_DEVICES"] = "-1"

import numpy as np
import keras
import tensorflow as tf
from keras import layers
from keras.datasets import mnist

# ---------------------------------------------------------------- 可调参数
# 都可用同名环境变量覆盖，便于试跑，例如：EPOCHS=2 PARALLEL=0 python 5_1_1_白噪声和全零通道.py
SEED = 42           # 固定随机种子，保证三次实验的初始化与 batch 顺序一致
EPOCHS = int(os.environ.get("EPOCHS", 10))
BATCH_SIZE = int(os.environ.get("BATCH_SIZE", 256))  # 128 更贴近书中设定但慢 ~1.5x
HIDDEN = int(os.environ.get("HIDDEN", 512))
NUM_CLASSES = 10
VAL_SIZE = 10000
PARALLEL = os.environ.get("PARALLEL", "1") != "0"    # 1=多进程并行跑三个实验
THREADS_PER_WORKER = 4                               # 并行时每个进程的 intra-op 线程数

EXPERIMENTS = [
    ("baseline (784 dims)", "baseline"),
    ("+784 zero channels (1568 dims)", "zeros"),
    ("+784 noise channels (1568 dims)", "noise"),
]
DISPLAY = {mode: name for name, mode in EXPERIMENTS}


# ---------------------------------------------------------------- 数据
def add_channels(x, mode):
    """把 784 维输入扩成 1568 维：后一半是白噪声或全零。

    直接分配新数组再切片赋值，避免 concatenate 把内存峰值再抬高一倍。
    """
    if mode == "baseline":
        return x

    n, dim = x.shape
    out = np.zeros((n, dim * 2), dtype="float32")
    out[:, :dim] = x
    if mode == "noise":
        # 与像素同量级 [0, 1) 的均匀白噪声；与标签完全无关
        rng = np.random.default_rng(SEED)
        out[:, dim:] = rng.random((n, dim), dtype="float32")
    return out


def build_model(input_dim):
    """与 c2_1 完全相同的网络结构，只是输入维度可变。"""
    model = keras.Sequential([
        keras.Input(shape=(input_dim,)),
        layers.Dense(HIDDEN, activation="relu"),
        layers.Dense(NUM_CLASSES, activation="softmax"),
    ])
    model.compile(optimizer="rmsprop",
                  loss="sparse_categorical_crossentropy",
                  metrics=["accuracy"])
    return model


class EpochLogger(keras.callbacks.Callback):
    """并行模式下每个进程自己打印进度，行首带实验名以免日志交错后看不懂。"""

    def __init__(self, tag):
        super().__init__()
        self.tag = tag

    def on_epoch_end(self, epoch, logs=None):
        logs = logs or {}
        print(f"[{self.tag}] epoch {epoch + 1:2d}/{EPOCHS}: "
              f"loss={logs.get('loss', 0):.4f} acc={logs.get('accuracy', 0):.4f} "
              f"val_loss={logs.get('val_loss', 0):.4f} val_acc={logs.get('val_accuracy', 0):.4f}",
              flush=True)


# ---------------------------------------------------------------- 单个实验
def run_experiment(mode, threads=0):
    """训练并评估一个实验。必须是模块级函数，ProcessPoolExecutor 才能 pickle 它。"""

    # 线程数要在执行第一个 TF 算子之前设置，否则不生效
    if threads:
        tf.config.threading.set_intra_op_parallelism_threads(threads)
        tf.config.threading.set_inter_op_parallelism_threads(1)

    keras.utils.set_random_seed(SEED)

    (train_images, train_labels), (test_images, test_labels) = mnist.load_data()
    x_all = train_images.reshape((60000, 28 * 28)).astype("float32") / 255
    x_test_raw = test_images.reshape((10000, 28 * 28)).astype("float32") / 255
    del train_images, test_images

    # 从训练集里划出验证集，用来看训练/验证曲线何时分离（过拟合的迹象）
    x_tr, y_tr = add_channels(x_all[VAL_SIZE:], mode), train_labels[VAL_SIZE:]
    x_val, y_val = add_channels(x_all[:VAL_SIZE], mode), train_labels[:VAL_SIZE]
    x_te = add_channels(x_test_raw, mode)
    del x_all, x_test_raw

    model = build_model(x_tr.shape[1])
    input_dim, params = x_tr.shape[1], model.count_params()
    print(f"[{mode}] input_dim={input_dim} params={params:,}", flush=True)

    history = model.fit(x_tr, y_tr,
                        epochs=EPOCHS,
                        batch_size=BATCH_SIZE,
                        validation_data=(x_val, y_val),
                        callbacks=[EpochLogger(mode)],
                        verbose=0)

    # 训练指标直接取最后一轮的训练态值，避免再跑一遍全量 evaluate（省 ~15% 时间）
    test_loss, test_acc = model.evaluate(x_te, test_labels, verbose=0)
    hist = history.history

    # 释放内存（3 个进程并行时 1568 维数组不小）
    del x_tr, x_val, x_te
    keras.backend.clear_session()

    return {"name": DISPLAY[mode], "mode": mode,
            "input_dim": input_dim, "params": params,
            "train_acc": hist["accuracy"][-1], "train_loss": hist["loss"][-1],
            "test_acc": test_acc, "test_loss": test_loss,
            "acc_curve": hist["accuracy"],
            "val_acc_curve": hist["val_accuracy"],
            "loss_curve": hist["loss"],
            "val_loss_curve": hist["val_loss"]}


# ---------------------------------------------------------------- 主流程
def run_all():
    """并行（多进程）或串行地跑完三个实验，返回顺序固定的结果列表。"""
    modes = [mode for _, mode in EXPERIMENTS]

    if not PARALLEL or len(modes) == 1:
        return [run_experiment(m) for m in modes]

    import multiprocessing as mp
    from concurrent.futures import ProcessPoolExecutor, as_completed

    cpu = os.cpu_count() or 1
    workers = min(len(modes), max(1, cpu // 2))
    threads = THREADS_PER_WORKER
    # 子进程是 spawn 出来的，会在自己的 import 阶段初始化 TF，所以线程数必须通过环境变量传下去
    os.environ["TF_NUM_INTRAOP_THREADS"] = str(threads)
    os.environ["TF_NUM_INTEROP_THREADS"] = "1"
    print(f"parallel: {workers} processes x {threads} intra-op threads "
          f"(cpu={cpu}, batch_size={BATCH_SIZE}, epochs={EPOCHS})\n", flush=True)

    ctx = mp.get_context("spawn")  # fork 会把父进程的 TF 线程池状态带进子进程，用 spawn 更安全
    by_mode = {}
    with ProcessPoolExecutor(max_workers=workers, mp_context=ctx) as pool:
        futures = {pool.submit(run_experiment, m, threads): m for m in modes}
        for fut in as_completed(futures):
            by_mode[futures[fut]] = fut.result()

    return [by_mode[m] for m in modes]


def print_summary(results):
    print(f"\n{'=' * 78}\n"
          f"summary (epochs={EPOCHS}, batch_size={BATCH_SIZE}, seed={SEED}, "
          f"parallel={PARALLEL})\n{'=' * 78}")
    header = (f"{'experiment':<32}{'dim':>6}{'params':>10}"
              f"{'train acc':>11}{'test acc':>10}{'test loss':>11}{'gap':>8}")
    print(header)
    print("-" * len(header))
    for r in results:
        gap = r["train_acc"] - r["test_acc"]
        print(f"{r['name']:<32}{r['input_dim']:>6}{r['params']:>10,}"
              f"{r['train_acc']:>11.4f}{r['test_acc']:>10.4f}"
              f"{r['test_loss']:>11.4f}{gap:>+8.4f}")
    print("-" * len(header))
    base = results[0]["test_acc"]
    for r in results[1:]:
        print(f"  {r['name']} vs baseline -> test acc {r['test_acc'] - base:+.4f}")

    print("\nexpected:")
    print("  + zero channels  -> ~= baseline (weights of zero inputs never move;")
    print("                      tiny gap only from init scale & float summation order)")
    print("  + noise channels -> lower test acc and wigglier val curve: slower convergence")
    print("                      plus fitting features unrelated to the label")


def plot_augmented_samples(indices=(0, 1, 2, 4)):
    """先看清楚"拼接 784 列"到底长什么样：原图 / 全零块 / 噪声块 / 拼接结果。

    这里一次性对 indices 这批样本做变换：因为 add_channels 内部用固定种子重设 rng，
    逐条调用会拿到同一组噪声，批量调用才能看到每行不同的噪声。
    """
    import matplotlib.pyplot as plt

    (train_images, train_labels), _ = mnist.load_data()
    x = train_images.reshape((-1, 28 * 28)).astype("float32") / 255

    sel = x[list(indices)]                    # (k, 784)
    dim = sel.shape[1]
    zero_block = add_channels(sel, "zeros")[:, dim:]
    noise_block = add_channels(sel, "noise")[:, dim:]

    print("\n--- 数据长什么样 ---")
    print(f"原始像素范围: [0.000, 1.000]  形状 (n, {dim})")
    print(f"噪声块范围:   [{noise_block.min():.3f}, {noise_block.max():.3f}]  "
          f"mean={noise_block.mean():.3f}  (与像素同量级，但与标签无关)")
    print(f"全零块范围:   [{zero_block.min():.3f}, {zero_block.max():.3f}]")
    print(f"拼接后形状:   ({len(indices)}, {dim * 2})  "
          f"前 {dim} 列=像素，后 {dim} 列=附加通道")
    print(f"第 1 个样本的标签 = {train_labels[indices[0]]}，"
          f"它的后 8 列值 = {np.round(noise_block[0, :8], 3)}")

    fig, axes = plt.subplots(len(indices), 4, figsize=(9, 2.2 * len(indices)))
    for row, idx in enumerate(indices):
        digit = sel[row]
        # 注意：直接 reshape(1568) 成 28x56 会按行主序变成"每行前半像素、后半噪声"的交错图，
        # 视觉上很有误导性，所以这里把两个 28x28 块左右拼起来看
        side_by_side = np.hstack([digit.reshape(28, 28), noise_block[row].reshape(28, 28)])
        panels = [
            (digit.reshape(28, 28), "binary", f"original pixels (784)\nlabel={train_labels[idx]}"),
            (zero_block[row].reshape(28, 28), "binary", "added zeros (784)"),
            (noise_block[row].reshape(28, 28), "gray", "added white noise (784)"),
            (side_by_side, "gray", "pixels | noise (784+784)"),
        ]
        for ax, (img, cmap, title) in zip(axes[row], panels):
            ax.imshow(img, cmap=cmap, vmin=0, vmax=1)
            ax.set_title(title, fontsize=8)
            ax.axis("off")
    fig.suptitle("what concatenating 784 extra columns looks like", fontsize=10)
    fig.tight_layout()
    plt.show()


def plot_results(results):
    import matplotlib.pyplot as plt

    epochs_range = range(1, EPOCHS + 1)
    colors = ["tab:blue", "tab:green", "tab:red"]
    short_names = [r["name"].split(" (")[0] for r in results]

    # 图 1：三个实验的训练/验证曲线，颜色区分实验，实线=验证集，虚线=训练集
    fig, axes = plt.subplots(1, 2, figsize=(12, 4.5))
    for ax, key_tr, key_va, ylabel in (
            (axes[0], "acc_curve", "val_acc_curve", "accuracy"),
            (axes[1], "loss_curve", "val_loss_curve", "loss")):
        for r, c, sn in zip(results, colors, short_names):
            ax.plot(epochs_range, r[key_tr], "--", color=c, alpha=0.5, label=f"{sn} (train)")
            ax.plot(epochs_range, r[key_va], "-o", color=c, markersize=4, label=f"{sn} (val)")
        ax.set_title(f"training and validation {ylabel}")
        ax.set_xlabel("epochs")
        ax.set_ylabel(ylabel)
        ax.grid(alpha=0.3)
        ax.legend(fontsize=7, loc="best")
    fig.tight_layout()
    plt.show()

    # 图 2：最终精度的柱状对比（放大 y 轴，否则看不出差距）
    plt.clf()
    x = np.arange(len(results))
    width = 0.35
    low = min(min(r["train_acc"], r["test_acc"]) for r in results) - 0.02
    plt.bar(x - width / 2, [r["train_acc"] for r in results], width, label="train acc")
    plt.bar(x + width / 2, [r["test_acc"] for r in results], width, label="test acc")
    plt.xticks(x, [f"{sn}\n({r['input_dim']} dims)" for r, sn in zip(results, short_names)],
               fontsize=8)
    plt.ylim(low, 1.0)
    plt.title("final accuracy: baseline vs zero channels vs noise channels")
    plt.ylabel("accuracy")
    plt.legend()
    plt.grid(axis="y", alpha=0.3)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    plot_augmented_samples()   # 先看一眼数据（不需要的话可以先注释掉）
    results = run_all()
    print_summary(results)
    plot_results(results)
