import tensorflow as tf

a = tf.constant(
    [[1,2,3],
     [0,1,2],
     [0,0,1]], dtype=tf.float32)
print(a)

b = tf.square(a)
print(b)
#tf.Tensor(
#[[1. 4. 9.]
# [0. 1. 4.]
# [0. 0. 1.]], shape=(3, 3), dtype=float32)

c = tf.sqrt(a)
print(c)
#tf.Tensor(
#[[1.        1.4142135 1.7320508]
# [0.        1.        1.4142135]
# [0.        0.        1.       ]], shape=(3, 3), dtype=float32)

d = a+a
print(d)
#[[2. 4. 6.]
# [0. 2. 4.]
# [0. 0. 2.]], shape=(3, 3), dtype=float32)

# 两个张量逐元素相乘
e = a * a
print("e: ", e)
#e:  tf.Tensor(
#[[1. 4. 9.]
# [0. 1. 4.]
# [0. 0. 1.]], shape=(3, 3), dtype=float32)

# 两个张量的积
f = tf.matmul(a, a)
print("f: ", f)
#f:  tf.Tensor(
#[[ 1.  4. 10.]
# [ 0.  1.  4.]
# [ 0.  0.  1.]], shape=(3, 3), dtype=float32)
