import numpy as np

x = np.random.random((10))
y = np.random.random((10))

print('x: {}'.format(x))
print('y: {}'.format(y))

z = np.dot(x, y)
print('z=np.dot(x, y): {}'.format(z))

'''
'''

def naive_vector_dot(x: np.array, y: np.array):
    assert len(x.shape) == 1
    assert len(y.shape) == 1
    assert x.shape[0] == y.shape[0]
    z = 0.0
    for i in range(x.shape[0]):
        z += x[i] * y[i]
    return z

dotz = naive_vector_dot(x, y)
print('dotz: {}'.format(dotz))

