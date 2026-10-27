# 5.3.1 调节关键的梯度下降参数：学习率"不对"是什么样子
#
# 书中原文（doc/5.3-改进模型拟合.md）：
#   「有时训练不开始，或者过早停止。损失保持不变。这个问题总是可以解决的……
#     出现这种问题时，问题总是出在梯度下降过程的配置：
#     优化器、模型权重初始值的分布、学习率或批量大小。」
#   「降低或提高学习率。学习率过大，可能会导致权重更新超出正常拟合的范围。
#     学习率过小，则可能导致训练过于缓慢，以至于几乎停止。」
#
# 本脚本只动一个旋钮：RMSprop 的 learning_rate。网络结构、初始化、批量大小、
# 数据、随机种子全部相同，于是拿到 3 条只有学习率不同的曲线，
# 把书里那两句话变成看得见的东西：
#   * 学习率过大 -> 每步都跳出几十个初始化的身位，loss 一开始就飞走
#     （几十上百，甚至 nan），精度停在 10% 的随机水平 = "训练不开始"。
#   * 学习率过小 -> 15 轮下来 loss 几乎不动，曲线像一条水平线 = "过早停止"。
#
# 为什么在 RMSprop 上这个现象格外干净：
#   RMSprop 把梯度除以它的滑动 RMS，每个坐标的更新量级 ≈ learning_rate，
#   而不是 ≈ learning_rate * |梯度|。所以学习率几乎就等于"每步权重走多远"，
#   而 Glorot 初始化的权重本身只有 ~0.05 的量级 —— 学习率取 1 或 10，
#   等于一步跨过几十个初始化的身位，梯度下降当然回不到谷底。
#   换成 SGD 时同样的道理要乘上梯度的尺度，现象就没这么直观。
#
# 对照基线：10 类问题的随机水平是 loss = ln(10) ≈ 2.3026、acc = 10%。
# 读图时先看这两条线：曲线贴在 ln(10) 上不动 -> 模型什么都没学到。
#
# ---------------------------------------------------------------- 运行
#   python c5_3_1_学习率不对.py
# 可用同名环境变量覆盖默认值，例如：
#   EPOCHS=5 TRAIN_N=5000 LRS=1e-6,1e-3,10 python c5_3_1_学习率不对.py
# 默认 20k 训练样本 x 15 轮 x 3 个学习率，纯 CPU 上十几秒量级。

import os

# 必须在导入 keras/tensorflow 之前设置（无 GPU 时也保留，省得 TF 去初始化 CUDA）
os.environ["CUDA_VISIBLE_DEVICES"] = "-1"
os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "2")

import numpy as np
import keras
from keras import layers
from keras.datasets import mnist

# ---------------------------------------------------------------- 可调参数
SEED = int(os.environ.get("SEED", 42))
EPOCHS = int(os.environ.get("EPOCHS", 15))
BATCH_SIZE = int(os.environ.get("BATCH_SIZE", 128))
TRAIN_N = int(os.environ.get("TRAIN_N", 20000))   # 只为了跑得快；全量 60k 结论一样
VAL_N = 10000
HIDDEN = int(os.environ.get("HIDDEN", 256))
# 只取三档，跨 9 个数量级（括号里是本机实测，20k x 15 轮）：
#   1e-8  太小（1e-3 少打了三个零）：loss 真的贴在 ln(10) 上不动，
#         acc 停在 10%，权重几乎没离开初始化（moved ≈ 0.2%）。
#   1e-3  RMSprop 默认值，合适的参照点（loss→0.01，acc≈1.00）。
#   1     太大：首轮权重一步就被推到初始化的几十倍，
#         loss 先爆到几百再卡在 ln(10) 附近来回摆，acc 卡在 ~0.16。
# 想看中间的过渡（1e-6 / 1e-4 / 1e-2 / 1e-1 / 10），直接覆盖这个环变量：
#   LRS=1e-6,1e-4,1e-3,1e-2,1e-1,1,10 python c5_3_1_学习率不对.py
# 注意 RMSprop 把更新量归一化，所以这里的“学习率”近似等于每步的权重位移；
# Glorot 初始化的权重只有 ~0.04 的量级，1e-1 以上都在跨步跳过谷底。
LRS = [float(v) for v in
       os.environ.get("LRS", "1e-8,1e-3,1").split(",")]

RANDOM_LOSS = float(np.log(10))   # 10 类均匀猜测的交叉熵 = 随机水平
RANDOM_ACC = 0.10


# ---------------------------------------------------------------- 数据
def load_data():
    """MNIST 摊平成 784 维，前 VAL_N 个样本做验证集，其余训练（再按 TRAIN_N 截断）。

    注意验证集不参与训练，只是用来确认"训练集指标不动"不是我的错觉。
    """
    (x_all, y_all), _ = mnist.load_data()
    x_all = x_all.reshape((-1, 28 * 28)).astype("float32") / 255

    x_val, y_val = x_all[:VAL_N], y_all[:VAL_N]
    x_tr, y_tr = x_all[VAL_N:VAL_N + TRAIN_N], y_all[VAL_N:VAL_N + TRAIN_N]
    del x_all
    return (x_tr, y_tr), (x_val, y_val)


def build_model():
    """与 c2_1 同一个套路：Dense(relu) -> Dense(softmax)，只有学习率会变。"""
    model = keras.Sequential([
        keras.Input(shape=(28 * 28,)),
        layers.Dense(HIDDEN, activation="relu"),
        layers.Dense(10, activation="softmax"),
    ])
    return model


# ---------------------------------------------------------------- 回调
class Recorder(keras.callbacks.Callback):
    """每轮打印一行，同时记下权重范数与"相对初始化挪动了多少"。

    ||W1 - W1_init|| / ||W1_init|| 是给"学习率太小"准备的心电图：
    只有几个百分点，就说明优化器走了几百上千步，其实还在原地踏步。
    """

    def __init__(self, lr):
        super().__init__()
        self.lr = lr
        self.kernels = []
        self.w1_init = None
        self.w1_init_norm = 1.0
        self.norms = []    # 每轮结束时的 (||W1||, ||W2||)
        self.disp = []     # 每轮结束时的相对位移

    def on_train_begin(self, logs=None):
        self.kernels = [w for w in self.model.weights if w.name.endswith("kernel")]
        self.w1_init = np.array(self.kernels[0].numpy(), dtype="float64")
        self.w1_init_norm = float(np.linalg.norm(self.w1_init)) or 1.0

    def on_epoch_end(self, epoch, logs=None):
        logs = logs or {}
        w1 = np.array(self.kernels[0].numpy(), dtype="float64")
        w2 = np.array(self.kernels[1].numpy(), dtype="float64")
        self.norms.append((float(np.linalg.norm(w1)), float(np.linalg.norm(w2))))
        self.disp.append(float(np.linalg.norm(w1 - self.w1_init) / self.w1_init_norm))

        print(f"[lr={self.lr:<7g}] epoch {epoch + 1:2d}/{EPOCHS}: "
              f"loss={logs.get('loss', float('nan')):>10.4f} "
              f"acc={logs.get('accuracy', float('nan')):6.3f} "
              f"val_loss={logs.get('val_loss', float('nan')):>10.4f} "
              f"val_acc={logs.get('val_accuracy', float('nan')):6.3f} "
              f"||W1||/||W1_init||={self.norms[-1][0] / self.w1_init_norm:9.3f} "
              f"moved={self.disp[-1] * 100:7.2f}%", flush=True)


# ---------------------------------------------------------------- 单个学习率
def run_one(lr, data):
    """从头训练一次。每个学习率都用同一个种子重新初始化，保证只差在 lr 上。"""
    (x_tr, y_tr), (x_val, y_val) = data

    keras.utils.set_random_seed(SEED)
    keras.backend.clear_session()

    model = build_model()
    model.compile(optimizer=keras.optimizers.RMSprop(learning_rate=lr),
                  loss="sparse_categorical_crossentropy",
                  metrics=["accuracy"])

    rec = Recorder(lr)
    history = model.fit(x_tr, y_tr,
                        epochs=EPOCHS,
                        batch_size=BATCH_SIZE,
                        validation_data=(x_val, y_val),
                        callbacks=[rec],
                        verbose=0)
    hist = history.history
    out = {"lr": lr,
           "loss": list(hist["loss"]),
           "acc": list(hist["accuracy"]),
           "val_loss": list(hist["val_loss"]),
           "val_acc": list(hist["val_accuracy"]),
           "norms": rec.norms,
           "disp": rec.disp,
           "w1_ratio": rec.norms[-1][0] / rec.w1_init_norm if rec.norms else float("nan")}
    del model
    keras.backend.clear_session()
    return out


# ---------------------------------------------------------------- 判定
def verdict(r):
    """根据曲线本身给一个机械的结论，免得“看不出来”变成“我说了算”。

    只靠 acc 和 loss 的单调性是分不清“太大”和“太小”的：两者都能让 acc
    停在随机水平附近。真正能区分方向的是权重范数：太大 -> ||W1|| 离初始化
    几十上百倍，太小 -> ||W1|| 几乎不动。
    """
    loss = np.asarray(r["loss"], dtype="float64")
    acc = np.asarray(r["acc"], dtype="float64")
    ratio = float(r["w1_ratio"])
    moved = float(r["disp"][-1])

    if not np.isfinite(loss).all():
        first_bad = int(np.argmax(~np.isfinite(loss))) + 1
        return f"发散：第 {first_bad} 轮 loss 变成 nan/inf"
    if not np.isfinite(ratio):
        return "发散：权重涨到溢出"

    if acc[-1] < 0.5 and ratio > 20:
        return (f"学习率太大：权重被推到初始化的 {ratio:.0f} 倍，"
                f"loss 贴着 ln(10) 来回摆 (acc={acc[-1]:.2f})")
    if acc[-1] < 0.5 and moved < 0.02:
        return (f"过早停止：{EPOCHS} 轮总共只挪动初始化的 {moved * 100:.3f}%，"
                f"学习率太小 (acc={acc[-1]:.2f})")
    if loss[-1] > loss[0]:
        return "没有下降：最后一轮 loss 比第一轮还高（在来回振荡）"
    if acc[-1] < 0.5:
        return f"在学但很慢：acc={acc[-1]:.2f}，{EPOCHS} 轮远远不够"
    if acc[-1] < 0.90:
        return "太慢：在学，但同一个轮数预算内远未拟合"
    if ratio > 20:
        return f"能学，但权重范数已涨到初始化的 {ratio:.0f} 倍（学习率偏大）"
    return "正常"


def print_summary(results, train_n):
    print(f"\n{'=' * 100}\n"
          f"summary (epochs={EPOCHS}, batch_size={BATCH_SIZE}, "
          f"train={train_n}, seed={SEED})\n{'=' * 100}")
    header = (f"{'learning_rate':>13}{'loss@1':>10}{'loss@last':>11}"
              f"{'train acc':>11}{'val acc':>10}{'||W1||/init':>13}{'moved':>9}")
    print(header)
    print("-" * 100)
    for r in results:
        print(f"{r['lr']:>13g}{r['loss'][0]:>10.4f}{r['loss'][-1]:>11.4f}"
              f"{r['acc'][-1]:>11.4f}{r['val_acc'][-1]:>10.4f}"
              f"{r['w1_ratio']:>13.3f}{r['disp'][-1] * 100:>8.2f}%")
    print("-" * 100)
    print(f"{'random guess':>13}{RANDOM_LOSS:>10.4f}{RANDOM_LOSS:>11.4f}"
          f"{RANDOM_ACC:>11.4f}{RANDOM_ACC:>10.4f}")
    print("-" * 100)

    for r in results:
        print(f"  lr={r['lr']:<7g} -> {verdict(r)}")

    print("\n读法：")
    print("  太小：loss 是水平线，moved 只有几个百分点 -> 优化器走了一千多步，")
    print("        其实还没离开初始化；调大学习率即可（加轮数只是“用时间换”，不是修配置）。")
    print("  太大：首轮 loss 就飞到几百上千（甚至 nan），||W1|| 涨到初始化的几十上百倍 ->")
    print("        权重更新超出正常拟合的范围；降 1~2 个数量级，或先加大批量让梯度更稳")
    print("        （本文件只演示学习率这一个旋钮）。")
    print("  注意 loss 高不代表没学：学习率偏小时模型可能是“自信地猜错” ——")
    print("        acc 在涨、loss 却贴着 ln(10)，所以两个指标要一起看。")
    print("  同样一批数据、同一个网络，唯一变的就是这一个旋钮 ——")
    print("    所以训练不开始时，先怀疑优化配置而不是“数据没信息”“模型不够大”。")


# ---------------------------------------------------------------- 画图
def _finite(seq):
    """把 nan/inf 原样留着（matplotlib 会断开线段），只统计有限值用于定坐标范围。"""
    a = np.asarray(seq, dtype="float64")
    return a[np.isfinite(a)]


def plot_curves(results):
    import matplotlib.pyplot as plt

    colors = [plt.cm.tab10(i % 10) for i in range(len(results))]
    epochs_range = np.arange(1, EPOCHS + 1)

    fig, axes = plt.subplots(1, 3, figsize=(16, 4.6))

    # 1) 训练 / 验证 loss，对数坐标：学习率太小的"水平线"和对数尺度刚好配
    ax = axes[0]
    for r, c in zip(results, colors):
        ax.plot(epochs_range, r["loss"], "-o", ms=3, color=c, label=f"lr={r['lr']:g}")
        ax.plot(epochs_range, r["val_loss"], ":", lw=1.2, color=c, alpha=0.6)
    ax.axhline(RANDOM_LOSS, ls="--", lw=1, color="gray")
    ax.annotate("random guess: ln(10)", xy=(epochs_range[-1], RANDOM_LOSS),
                xytext=(-4, 4), textcoords="offset points",
                ha="right", fontsize=8, color="gray")
    ax.set_yscale("log")
    all_loss = np.concatenate([_finite(r["loss"]) for r in results] +
                              [_finite(r["val_loss"]) for r in results]) \
        if any(len(_finite(r["loss"])) for r in results) else np.array([1.0])
    if all_loss.size:
        ax.set_ylim(max(0.01, all_loss.min() / 3), all_loss.max() * 3)
    ax.set_title("loss (log scale): solid = train, dotted = val")
    ax.set_xlabel("epochs")
    ax.set_ylabel("loss")
    ax.grid(alpha=0.3, which="both")
    ax.legend(fontsize=7, ncol=2, loc="best")

    # 2) 训练 / 验证精度，线性坐标：大学习率的曲线永远贴着 10% 那条线
    ax = axes[1]
    for r, c in zip(results, colors):
        ax.plot(epochs_range, r["acc"], "-o", ms=3, color=c, label=f"lr={r['lr']:g}")
        ax.plot(epochs_range, r["val_acc"], ":", lw=1.2, color=c, alpha=0.6)
    ax.axhline(RANDOM_ACC, ls="--", lw=1, color="gray")
    ax.annotate("random guess: 10%", xy=(epochs_range[0], RANDOM_ACC),
                xytext=(3, 3), textcoords="offset points", fontsize=8, color="gray")
    ax.set_ylim(-0.02, 1.02)
    ax.set_title("accuracy: solid = train, dotted = val")
    ax.set_xlabel("epochs")
    ax.set_ylabel("accuracy")
    ax.grid(alpha=0.3)
    ax.legend(fontsize=7, ncol=2, loc="best")

    # 3) 第一层权重范数相对初始化的倍数：太大 -> 飞走，太小 -> 一直贴着 1.0
    ax = axes[2]
    for r, c in zip(results, colors):
        ratio = [n[0] / max(_init_norm(r), 1e-12) for n in r["norms"]]
        ax.plot(epochs_range, ratio, "-o", ms=3, color=c, label=f"lr={r['lr']:g}")
    ax.axhline(1.0, ls="--", lw=1, color="gray")
    ax.annotate("initialisation", xy=(epochs_range[0], 1.0),
                xytext=(3, 3), textcoords="offset points", fontsize=8, color="gray")
    ax.set_yscale("log")
    ax.set_title("||W1|| / ||W1_init||")
    ax.set_xlabel("epochs")
    ax.set_ylabel("ratio (log)")
    ax.grid(alpha=0.3, which="both")
    ax.legend(fontsize=7, ncol=2, loc="best")

    fig.suptitle("same data / same net / same seed — only the learning rate changes", fontsize=10)
    fig.tight_layout()
    plt.show()


def _init_norm(r):
    """由 w1_ratio 与最后一轮的范数反推初始化范数，省得为画图多存一份数据。"""
    last_norm = r["norms"][-1][0] if r["norms"] else float("nan")
    if not np.isfinite(r["w1_ratio"]) or r["w1_ratio"] <= 0:
        return float("nan")
    return last_norm / r["w1_ratio"]


def plot_sweep(results):
    """把每个学习率压缩成一个点：横轴 log(lr)，纵轴最后一轮的 loss 与 acc。"""
    import matplotlib.pyplot as plt

    lrs = [r["lr"] for r in results]
    final_loss = [r["loss"][-1] for r in results]
    final_acc = [r["acc"][-1] * 100 for r in results]

    fig, ax = plt.subplots(figsize=(7.5, 4.8))
    ax.semilogx(lrs, final_loss, "o-", color="tab:red", label="final train loss")
    ax.set_yscale("log")
    ax.set_xlabel("learning rate")
    ax.set_ylabel("final train loss (log)", color="tab:red")
    ax.tick_params(axis="y", labelcolor="tab:red")
    ax.axhline(RANDOM_LOSS, ls="--", lw=1, color="gray")
    ax.annotate("ln(10): nothing learned", xy=(lrs[-1], RANDOM_LOSS),
                xytext=(-4, 4), textcoords="offset points",
                ha="right", fontsize=8, color="gray")
    ax.axvline(1e-3, ls="--", lw=1, color="tab:blue", alpha=0.6)
    ax.annotate("RMSprop default 1e-3", xy=(1e-3, 1.0), xycoords=("data", "axes fraction"),
                xytext=(4, -12), textcoords="offset points", fontsize=8, color="tab:blue")

    ax2 = ax.twinx()
    ax2.semilogx(lrs, final_acc, "s--", color="tab:green", label="final train acc")
    ax2.set_ylabel("final train accuracy (%)", color="tab:green")
    ax2.tick_params(axis="y", labelcolor="tab:green")
    ax2.axhline(RANDOM_ACC * 100, ls=":", lw=1, color="tab:green", alpha=0.5)
    ax2.set_ylim(-2, 102)

    ax.set_title(f"final metrics vs learning rate ({EPOCHS} epochs)")
    fig.tight_layout()
    plt.show()


# ---------------------------------------------------------------- 主流程
if __name__ == "__main__":
    data = load_data()
    print(f"train={len(data[0][0])} val={len(data[1][0])} hidden={HIDDEN} "
          f"batch={BATCH_SIZE} epochs={EPOCHS} seed={SEED}\n", flush=True)

    results = []
    for lr in LRS:
        print(f"--- learning_rate = {lr:g} ---", flush=True)
        results.append(run_one(lr, data))

    print_summary(results, len(data[0][0]))
    plot_curves(results)
    plot_sweep(results)
