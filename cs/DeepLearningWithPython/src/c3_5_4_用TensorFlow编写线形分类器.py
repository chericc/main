import numpy as np
import matplotlib.pyplot as plt

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