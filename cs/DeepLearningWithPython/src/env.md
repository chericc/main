# env

环境：conda `dlpy`（Python 3.11）
已验证版本：tensorflow-cpu 2.21.0 / keras 3.15.1 / numpy 2.4.6 / matplotlib 3.11.2

## 创建环境

USTC 的 anaconda 镜像会把包 302 跳转到 NJU，conda/libmamba 跟随失败
（`CondaHTTPError: HTTP 000 CONNECTION FAILED`），所以直接用南大源：

```bash
conda create -n dlpy python=3.11 -y \
  --override-channels -c https://mirror.nju.edu.cn/anaconda/pkgs/main
```

## 安装依赖

pip 源优先级（实测）：南大 6.8 MB/s > 阿里云 3.6 MB/s > 官方直连 46 KB/s。
走本地 7897 代理连 files.pythonhosted.org 会 TLS 握手失败，不要用代理装 pip 包。

```bash
/home/test/miniconda3/envs/dlpy/bin/python -m pip install tensorflow-cpu matplotlib \
  -i https://mirror.nju.edu.cn/pypi/web/simple --trusted-host mirror.nju.edu.cn
```

## 数据集

`keras.datasets.*.load_data()` 从 storage.googleapis.com 下载，国内直连会吊死，
只有这一步需要代理。文件落到 `~/.keras/datasets/`，下好一次以后就不用再联网。

```bash
mkdir -p ~/.keras/datasets
curl -x http://127.0.0.1:7897 -o ~/.keras/datasets/mnist.npz \
  https://storage.googleapis.com/tensorflow/tf-keras-datasets/mnist.npz
```

其它数据集同理，比如 imdb.npz / reuters.npz 也在同一目录下。

## 运行

```bash
conda activate dlpy
cd /home/test/code/main/cs/DeepLearningWithPython/src
python c2_1_初始神经网络.py
```

`c2_1_初始神经网络.py` 结果：`test_acc: 0.9792`（5 轮，与书中 0.979 一致）。

## 备注

- 机器无 GPU，脚本里 `os.environ["CUDA_VISIBLE_DEVICES"] = "-1"` 保留即可。
- 日志里的 `To enable the following instructions: AVX2 ...` 是官方 CPU 轮子未按本机指令集编译的提示，可忽略。
- 纯 NumPy 的脚本（c2_2_1 ~ c2_3_4）在 `photoie` 环境里也能跑；
  不要把 TF 装进 photoie，它依赖 `numpy<2.3`，会降级那里的 numpy 2.4.6。
