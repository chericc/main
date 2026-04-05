# c_2_3.py
import numpy as np

# 矩阵，也叫2阶张量或2维张量
# 矩阵有两个轴
x = np.array([[5, 78, 2, 34, 0],
             [6, 79, 3, 35, 1],
             [7, 80, 4, 36, 2]])
print(x)
print(x.ndim) # 2