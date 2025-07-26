
import time
import numpy as np

def naive_relu(x: np.array):
    assert len(x.shape) == 2
    x = x.copy()
    for i in range(x.shape[0]):
        for j in range(x.shape[1]):
            x[i,j] = max(x[i,j], 0)
    return x

def naive_add(x: np.array, y: np.array):
    assert len(x.shape) == 2
    assert x.shape == y.shape
    x = x.copy()
    for i in range(x.shape[0]):
        for j in range(x.shape[1]):
            x[i, j] += y[i, j]
    return x

x = np.random.random((20, 100))
y = np.random.random((20, 100))

t0 = time.time()
for _ in range(1000):
    z = naive_add(x, y)
    z = naive_relu(x)
print('Took(naive imp): {} s'.format(time.time() - t0))

t0 = time.time()
for _ in range(1000):
    z = x + y
    z = np.maximum(z, 0.)
print('Took(numpy imp): {} s'.format(time.time() - t0))