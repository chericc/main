import numpy as np

def naive_add_maatrix_and_vector(x: np.array, y: np.array):
    assert len(x.shape) == 2
    assert len(y.shape) == 1
    assert x.shape[1] == y.shape[0]
    x = x.copy()
    for i in range(x.shape[0]):
        for j in range(x.shape[1]):
            x[i, j] = y[j]
    return x

X = np.random.random((2, 10)) # x是一个形状为(2, 10)的随即矩阵
y = np.random.random((10, )) # y是一个形状为(10, )的随即向量

t = naive_add_maatrix_and_vector(X, y)
print('t:{}'.format(t))
'''
t:[[0.64092893 0.19903429 0.403889   0.43535951 0.42948944 0.15512728
  0.17466031 0.05717715 0.88403348 0.52440238]
 [0.64092893 0.19903429 0.403889   0.43535951 0.42948944 0.15512728
  0.17466031 0.05717715 0.88403348 0.52440238]]
'''

print('y:{}'.format(y))
'''
y:[0.64092893 0.19903429 0.403889   0.43535951 0.42948944 0.15512728
 0.17466031 0.05717715 0.88403348 0.52440238]
'''

y = np.expand_dims(y, axis=0) # y的形状变为(1, 10)

print('y:{}'.format(y))
'''
y:[[0.64092893 0.19903429 0.403889   0.43535951 0.42948944 0.15512728
  0.17466031 0.05717715 0.88403348 0.52440238]]
'''

Y = np.concatenate([y] * 2, axis=0) # 将y沿着轴0重复32次后得到Y，其形状为(2, 10)

print('Y:{}'.format(Y))
'''
Y:[[0.64092893 0.19903429 0.403889   0.43535951 0.42948944 0.15512728
  0.17466031 0.05717715 0.88403348 0.52440238]
 [0.64092893 0.19903429 0.403889   0.43535951 0.42948944 0.15512728
  0.17466031 0.05717715 0.88403348 0.52440238]]
'''