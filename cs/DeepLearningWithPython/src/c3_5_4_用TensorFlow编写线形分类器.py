import numpy as np
import matplotlib.pyplot as plt
import tensorflow as tf

num_samples_per_class = 1000

# 生成第一个类别的点：1000个二维随机点。协方差矩阵为[[1,0.5],[0.5,1]]
# 对应于一个从左下方到右上方的椭圆形点云
# 形状是 (2000,2)
negative_samples = np.random.multivariate_normal(
    mean=[0,3],
    cov=[[1, 0.5], [0.5, 1]],
    size=num_samples_per_class
)
print(negative_samples)

# 生成第二个类别的点，协方差矩阵相同，
# 均值不同
positive_samples = np.random.multivariate_normal(
    mean=[3, 0],
    cov=[[1, 0.5], [0.5, 1]],
    size=num_samples_per_class
)

# 将两个类别堆叠成一个形状为 (2000, 2) 的数组
inputs = np.vstack((negative_samples, positive_samples)).astype(np.float32)

# 生成目标标签，元素为0或1，表示目标属于哪个类别
targets = np.vstack((np.zeros((num_samples_per_class, 1), dtype='float32'),
                     np.ones((num_samples_per_class, 1), dtype="float32")))

# 绘制两个类别
plt.scatter(inputs[:, 0], inputs[:, 1], c=targets[:, 0])
plt.show()

# 创建变量W和b，分别用随机值和零进行初始化
input_dim = 2 # 二维
output_dim = 1 # 输出预测是一个分数值
W = tf.Variable(initial_value=tf.random.uniform(shape=(input_dim, output_dim)))
b = tf.Variable(initial_value=tf.zeros(shape=output_dim))

# 前向传播函数
def model(inputs):
    return tf.matmul(inputs, W) + b

# 均方误差损失函数
def square_loss(targets, predictions):
    per_sample_losses = tf.square(targets - predictions)
    return tf.reduce_mean(per_sample_losses)

learning_rate = 0.1
def training_step(inputs, targets):
    with tf.GradientTape() as tape:
        predictions = model(inputs)
        loss = square_loss(targets=targets, predictions=predictions)
        grad_loss_wrt_W, grad_loss_wrt_b = tape.gradient(loss, [W, b])
        W.assign_sub(grad_loss_wrt_W * learning_rate)
        b.assign_sub(grad_loss_wrt_b * learning_rate)
        return loss
    
# 为简单起见，我们将进行批量训练，而不是小批量训练，即在所有数据上进行训练，而不是小批量地进行迭代。
for step in range(40):
    loss = training_step(inputs, targets)
    print(f"loss at step {step}: {loss: .4f}")

predictions = model(inputs)
plt.scatter(inputs[:, 0], inputs[:, 1], c=predictions[:, 0] > 0.5)
plt.show()

