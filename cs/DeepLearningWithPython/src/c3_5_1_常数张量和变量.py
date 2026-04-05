import tensorflow as tf

# 全1张量或全0张量

x = tf.ones(shape=(2,3))
print(x)
# tf.Tensor(
# [[1. 1. 1.]
#  [1. 1. 1.]], shape=(2, 3), dtype=float32)

x = tf.zeros(shape=(2,3))
# print(x)
# [[0. 0. 0.]
#  [0. 0. 0.]], shape=(2, 3), dtype=float32)

# 随机张量

# 从均值为0、标准差为1的正态分布中抽取的随机张量
x = tf.random.normal(shape=(2,3),mean=0,stddev=1.)
print(x)
# tf.Tensor(
# [[ 0.80221695  1.5176442   0.4680181 ]
#  [-1.2655354   0.23147008  1.5174577 ]], shape=(2, 3), dtype=float32)

x = tf.random.uniform(shape=(2,3),minval=0,maxval=1)
print(x)
# tf.Tensor(
# [[0.7594527  0.9105085  0.28936458]
#  [0.46199334 0.9873706  0.29011333]], shape=(2, 3), dtype=float32)



