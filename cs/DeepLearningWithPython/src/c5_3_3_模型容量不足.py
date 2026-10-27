# 5.3.3 提高模型容量：容量不足时，连过拟合都做不到
#
# 书中原文（doc/5.3-改进模型拟合.md）：
#   「如果你成功得到了一个能够拟合的模型，验证指标正在下降，而且模型似乎具有一定的
#     泛化能力，那么恭喜你：你就快要成功了。接下来，你需要让模型过拟合。
#     比如在 MNIST 上训练的一个简单的 logistic 回归模型。
#     验证指标似乎保持不变，或者改进的非常缓慢，而不是达到峰值后扭转方向。
#     验证损失达到了 0.26，然后就保持不变。」
#   「如果无法实现过拟合，可能是因为模型的表示能力存在问题：你需要一个容量更大的模型。
#     若要提高模型的表示能力，你可以添加更多的层、使用更大的层（拥有更多参数的层），
#     或者使用更适合当前问题的层类型。」
#
# 本脚本就是这段话的对照实验：同一批数据、同一个优化器、同样的轮数，
# 只把"容量"从 logistic 回归一路加到 Dense(512)，观察三件事：
#   1. 容量不足（logistic 回归）：训练集本身都没拟合住（train acc 停在 ~0.92），
#      val loss 从第几轮起就基本不动了，全程没有峰值、没有回升 ——
#      这就是"改进非常缓慢，而不是达到峰值后扭转方向"。
#   2. 容量加大：train 与 val 的差距（gap）开始拉开，val loss 出现最低点后回升。
#   3. 容量继续加大：gap 更大、val loss 的谷底更早出现，
#      于是你终于拥有了一个可以拿来做过拟合抑制的模型。
#
# 判断依据不能只看"精度高不高"：容量不足的模型 val 精度看起来也还行（~0.92），
# 但它 train 精度同样上不去 —— 训练集都没记住，说明是表示能力不够，不是泛化问题。
# 所以每个配置都同时记录 gap 和 val loss 的谷底位置，summary 里按这两个数给结论。
#
# ---------------------------------------------------------------- 运行
#   python c5_3_3_模型容量不足.py
# 可用同名环境变量覆盖默认值，例如：
#   EPOCHS=10 TRAIN_N=5000 python c5_3_3_模型容量不足.py
#   HIDDEN_SIZES=0,64 python c5_3_3_模型容量不足.py
# 默认 20k 训练样本 x 25 轮 x 4 个容量档，纯 CPU 上几十秒量级。

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
EPOCHS = int(os.environ.get("EPOCHS", 25))
BATCH_SIZE = int(os.environ.get("BATCH_SIZE", 128))
TRAIN_N = int(os.environ.get("TRAIN_N", 20000))   # 20k 比 60k 更容易看到过拟合
VAL_N = 10000
# 容量阶梯：0 = 没有隐藏层，就是 logistic 回归（书中那个 val loss 卡在 0.26 的模型）。
# 16 只是"比 logistic 强一点点"，128 是刚好的档，512 用来展示真正过拟合。
HIDDEN_SIZES = [int(v) for v in
                os.environ.get("HIDDEN_SIZES", "0,16,128,512").split(",")]


# ---------------------------------------------------------------- 数据
def load_data():
    """MNIST 摊平成 784 维；前 VAL_N 个样本做验证集。

    验证集只用来量 gap（train vs val），不参与任何选择 ——
    本节要看的正是"训练集能不能拟合住"和"验证曲线有没有回头"。
    """
    (x_all, y_all), _ = mnist.load_data()
    x_all = x_all.reshape((-1, 28 * 28)).astype("float32") / 255

    x_val, y_val = x_all[:VAL_N], y_all[:VAL_N]
    x_tr, y_tr = x_all[VAL_N:VAL_N + TRAIN_N], y_all[VAL_N:VAL_N + TRAIN_N]
    del x_all
    return (x_tr, y_tr), (x_val, y_val)


def build_model(hidden):
    """hidden=0 -> 单个 softmax 层（logistic 回归）；否则一层 relu 再接 softmax。

    注意这里刻意不加上 dropout / 正则：5.3.3 的任务是"先做到能过拟合"，
    正则化是 5.4 才登场的工具，现在加上就等于自己把峰值削掉。
    """
    if hidden == 0:
        model = keras.Sequential([
            keras.Input(shape=(28 * 28,)),
            layers.Dense(10, activation="softmax"),
        ])
    else:
        model = keras.Sequential([
            keras.Input(shape=(28 * 28,)),
            layers.Dense(hidden, activation="relu"),
            layers.Dense(10, activation="softmax"),
        ])
    model.compile(optimizer=keras.optimizers.RMSprop(learning_rate=1e-3),
                  loss="sparse_categorical_crossentropy",
                  metrics=["accuracy"])
    return model


# ---------------------------------------------------------------- 回调
class EpochLogger(keras.callbacks.Callback):
    """每轮打印一行；行首带容量标签，4 个配置串行跑时日志不至于混掉。"""

    def __init__(self, tag):
        super().__init__()
        self.tag = tag

    def on_epoch_end(self, epoch, logs=None):
        logs = logs or {}
        print(f"[{self.tag}] epoch {epoch + 1:2d}/{EPOCHS}: "
              f"loss={logs.get('loss', float('nan')):.4f} "
              f"acc={logs.get('accuracy', float('nan')):.4f} "
              f"val_loss={logs.get('val_loss', float('nan')):.4f} "
              f"val_acc={logs.get('val_accuracy', float('nan')):.4f}",
              flush=True)


# ---------------------------------------------------------------- 单个容量
def run_one(hidden, data):
    """训练一个容量档。各档共用同一个种子和同一份数据，只差在容量上。"""
    (x_tr, y_tr), (x_val, y_val) = data

    keras.utils.set_random_seed(SEED)
    keras.backend.clear_session()

    model = build_model(hidden)
    tag = "logistic" if hidden == 0 else f"dense{hidden}"
    params = model.count_params()
    print(f"--- {tag} (params={params:,}) ---", flush=True)

    history = model.fit(x_tr, y_tr,
                        epochs=EPOCHS,
                        batch_size=BATCH_SIZE,
                        validation_data=(x_val, y_val),
                        callbacks=[EpochLogger(tag)],
                        verbose=0)
    hist = history.history
    del model
    keras.backend.clear_session()

    return {"hidden": hidden,
            "name": tag,
            "params": params,
            "loss": list(hist["loss"]),
            "acc": list(hist["accuracy"]),
            "val_loss": list(hist["val_loss"]),
            "val_acc": list(hist["val_accuracy"])}


# ---------------------------------------------------------------- 判定
def val_turning_point(r):
    """返回验证损失的谷底轮次（1 起）与"回头幅度"。

    容量不足的模型谷底会落在最后一轮附近（曲线还在缓慢下降或已平），
    能过拟合的模型谷底靠前，之后明显回升 —— 这正是书里区分两者的那句话。
    """
    v = np.asarray(r["val_loss"], dtype="float64")
    if not np.isfinite(v).all():
        return int(np.argmin(np.nan_to_num(v, nan=np.inf))) + 1, 0.0
    best = int(np.argmin(v))
    rise = (v[-1] - v[best]) / max(v[best], 1e-12)
    return best + 1, float(rise)


def verdict(r):
    """按"训练集拟合住了没有"和"验证损失有没有回头"给出机械结论。"""
    acc, val_acc = r["acc"][-1], r["val_acc"][-1]
    gap = acc - val_acc
    best_ep, rise = val_turning_point(r)

    if acc < 0.97:
        return (f"容量不足：训练集都没拟合住（train acc={acc:.3f}），"
                f"val loss 谷底在第 {best_ep} 轮、之后只回头 {rise * 100:+.1f}%")
    if rise > 0.02 and best_ep < EPOCHS - 1:
        return (f"过拟合（这正是 5.3.3 想要的结果）：gap={gap * 100:+.1f} 个点，"
                f"val loss 第 {best_ep} 轮见底后回升 {rise * 100:+.1f}%")
    return f"刚好：能拟合住（train acc={acc:.3f}）但还没明显过拟合，gap={gap * 100:+.1f}"


def print_summary(results):
    print(f"\n{'=' * 108}\n"
          f"summary (epochs={EPOCHS}, batch_size={BATCH_SIZE}, seed={SEED})\n{'=' * 108}")
    header = (f"{'model':<10}{'params':>10}{'train acc':>11}{'val acc':>9}{'gap':>8}"
              f"{'train loss':>12}{'val loss':>10}{'val bottom':>12}{'rise':>8}")
    print(header)
    print("-" * 108)
    for r in results:
        best_ep, rise = val_turning_point(r)
        gap = r["acc"][-1] - r["val_acc"][-1]
        print(f"{r['name']:<10}{r['params']:>10,}"
              f"{r['acc'][-1]:>11.4f}{r['val_acc'][-1]:>9.4f}{gap:>+8.4f}"
              f"{r['loss'][-1]:>12.4f}{r['val_loss'][-1]:>10.4f}"
              f"{('ep ' + str(best_ep)):>12}{rise * 100:>7.1f}%")
    print("-" * 108)

    for r in results:
        print(f"  {r['name']:<10} -> {verdict(r)}")

    print("\n读法：")
    print("  train acc 上不去（比如 0.92）而 gap 接近 0 -> 不是泛化问题，是容量问题。")
    print("    此时 val loss 的谷底在最后一轮、之后不回头：没有峰值可看。")
    print("    修法是加容量（加层 / 加宽 / 换更适合的层类型），本文件用加宽演示。")
    print("  train acc 逼近 1.0、gap 拉开、val loss 见底后回升 -> 你终于能过拟合了，")
    print("    这才轮到 5.4 的正则化 / dropout / 早停上场。")
    print("  所以 5.3.3 的目标不是“精度最高”，而是“先拿到一个能过拟合的模型”。")


# ---------------------------------------------------------------- 画图
def _acc_ylim(results):
    """精度子图的下限跟着数据走：TRAIN_N 很小时 logistic 可能只有 0.7，
    写死 0.85 会把曲线切掉。"""
    vals = [v for r in results for v in r["acc"] + r["val_acc"]
            if np.isfinite(v)]
    return (max(0.0, min(vals) - 0.02), 1.005) if vals else (0.0, 1.005)


def plot_curves(results):
    import matplotlib.pyplot as plt

    colors = [plt.cm.viridis(i / max(len(results) - 1, 1)) for i in range(len(results))]
    epochs_range = np.arange(1, EPOCHS + 1)

    fig, axes = plt.subplots(1, 3, figsize=(16, 4.6))

    # 1) loss：实线 train、虚线 val。容量小的两条线一直贴在一起且压不下去；
    #    容量大的两条线先一起下降，然后分开（val 上扬）= 过拟合的开口。
    ax = axes[0]
    for r, c in zip(results, colors):
        ax.plot(epochs_range, r["loss"], "-", lw=1.8, color=c,
                label=f"{r['name']} train")
        ax.plot(epochs_range, r["val_loss"], "--", lw=1.4, color=c, alpha=0.8,
                label=f"{r['name']} val")
    ax.set_yscale("log")
    ax.set_title("loss: solid = train, dashed = val")
    ax.set_xlabel("epochs")
    ax.set_ylabel("loss (log)")
    ax.grid(alpha=0.3, which="both")
    ax.legend(fontsize=7, ncol=2, loc="best")

    # 2) accuracy：看 gap 怎么随容量张开
    ax = axes[1]
    for r, c in zip(results, colors):
        ax.plot(epochs_range, r["acc"], "-", lw=1.8, color=c, label=f"{r['name']} train")
        ax.plot(epochs_range, r["val_acc"], "--", lw=1.4, color=c, alpha=0.8,
                label=f"{r['name']} val")
    ax.set_ylim(*_acc_ylim(results))
    ax.set_title("accuracy: solid = train, dashed = val")
    ax.set_xlabel("epochs")
    ax.set_ylabel("accuracy")
    ax.grid(alpha=0.3)
    ax.legend(fontsize=7, ncol=2, loc="lower right")

    # 3) 只画验证损失，并标出每条的谷底：容量越大，谷底越早、回头的幅度越大
    ax = axes[2]
    for r, c in zip(results, colors):
        v = np.asarray(r["val_loss"], dtype="float64")
        best_ep, _ = val_turning_point(r)
        ax.plot(epochs_range, v, "-o", ms=3, color=c, label=r["name"])
        ax.plot([best_ep], [v[best_ep - 1]], "*", ms=13, color=c, mec="black", mew=0.4)
    ax.set_title("val loss with its minimum (star)")
    ax.set_xlabel("epochs")
    ax.set_ylabel("val loss")
    ax.grid(alpha=0.3)
    ax.legend(fontsize=7, loc="best")

    fig.suptitle("same data / same seed / same epochs — only the capacity changes",
                 fontsize=10)
    fig.tight_layout()
    plt.show()


def plot_capacity_sweep(results):
    """把每个容量压成一个点：横轴参数量（对数），纵轴最终的 train / val loss。"""
    import matplotlib.pyplot as plt

    params = [r["params"] for r in results]
    train_loss = [r["loss"][-1] for r in results]
    val_loss = [r["val_loss"][-1] for r in results]

    fig, ax = plt.subplots(figsize=(7.5, 4.8))
    ax.semilogx(params, train_loss, "o-", color="tab:blue", label="final train loss")
    ax.semilogx(params, val_loss, "s--", color="tab:red", label="final val loss")
    ax.set_xlabel("parameters (log scale) = capacity")
    ax.set_ylabel("final loss")
    ax.grid(alpha=0.3, which="both")
    for r, p, v in zip(results, params, val_loss):
        ax.annotate(r["name"], xy=(p, v), xytext=(0, 8), textcoords="offset points",
                    ha="center", fontsize=8, color="tab:red")

    lo, hi = min(params), max(params)
    ax.axvspan(lo / 1.5, lo * 11, color="tab:gray", alpha=0.12)
    ax.annotate("underfitting:\nboth losses stuck high", xy=(lo * 1.4, max(val_loss)),
                fontsize=8, color="dimgray", va="top")
    ax.set_title("capacity vs final loss: train keeps dropping, val turns around")
    ax.legend(fontsize=8)
    fig.tight_layout()
    plt.show()


# ---------------------------------------------------------------- 主流程
if __name__ == "__main__":
    data = load_data()
    print(f"train={len(data[0][0])} val={len(data[1][0])} "
          f"batch={BATCH_SIZE} epochs={EPOCHS} seed={SEED} "
          f"hidden={HIDDEN_SIZES}\n", flush=True)

    results = [run_one(h, data) for h in HIDDEN_SIZES]
    print_summary(results)
    plot_curves(results)
    plot_capacity_sweep(results)
