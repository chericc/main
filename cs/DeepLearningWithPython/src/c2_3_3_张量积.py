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

def naive_matrix_vector_dot(x: np.array, y: np.array):
    assert len(x.shape) == 2
    assert len(y.shape) == 1
    assert x.shape[1] == y.shape[0]
    z = np.zeros(x.shape[0])
    for i in range(x.shape[0]):
        for j in range(x.shape[1]):
            z[i] += x[i, j] * y[j]
    return z

def naive_matrix_vector_dot_v2(x: np.array, y: np.array):
    z = np.zeros(x.shape[0])
    for i in range(x.shape[0]):
        z[i] = naive_vector_dot(x[i, :], y)
    return z


def naive_matrix_dot(x: np.array, y: np.array):
    assert len(x.shape) == 2
    assert len(y.shape) == 2
    assert x.shape[1] == y.shape[0]
    z = np.zeros((x.shape[0], y.shape[1]))
    for i in range(x.shape[0]):
        for j in range(y.shape[1]):
            row_x = x[i, :]
            column_y = y[:, j]
            z[i, j] = naive_vector_dot(row_x, column_y)
    return z