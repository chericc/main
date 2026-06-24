from keras.datasets import imdb

(train_data, train_labels), (test_data, test_labels) = imdb.load_data(num_words = 10000)
# (train_data, train_labels), (test_data, test_labels) = imdb.load_data()

print(f"len={len(train_data)}")
print(f"len={len(train_labels)}")

# 得到的索引是 [word,id] 的形式
word_index = imdb.get_word_index()
# 将字典反转
reverse_word_index = dict(
    [(value, key) for (key, value) in word_index.items()]
)
# 对第一条评论，根据索引id逐个找到对应的单词，然后通过空格连接起来
decoded_review = " ".join(
    [reverse_word_index.get(i - 3, "?") for i in train_data[0]]
)

print(f"decoded_review:{decoded_review}")

import numpy as np

# multi-hot 编码方法，将索引值对应位置设为1
def vectorize_sequences(sequences, dimension=10000):
    results = np.zeros((len(sequences), dimension))
    for i, sequence in enumerate(sequences):
        for j in sequence:
            results[i, j] = 1.
    return results
x_train = vectorize_sequences(train_data)
x_test = vectorize_sequences(test_data)

print(f"x_train:{x_train}")

# 标签向量化（这里是底层存储格式的转化，转化为np数组，并且格式为float32）
y_train = np.asarray(train_labels).astype("float32")
y_test = np.asarray(test_labels).astype("float32")

print(f"x_train:{x_train}")

from tensorflow import keras
from tensorflow.keras import layers

model = keras.Sequential([
    layers.Dense(16, activation="relu"),
    layers.Dense(16, activation="relu"),
    layers.Dense(1, activation="sigmoid")
])

# 4.1.3 编译模型
model.compile(optimizer="rmsprop",
                loss="binary_crossentropy",
                metrics=["accuracy"])


# 4.1.4 留出验证集
x_val = x_train[:10000]
partial_x_train = x_train[10000:]
y_val = y_train[:10000]
partial_y_train = y_train[10000:]

print(f'x_val.len={len(x_val)}')
print(f'partial_x_train={len(partial_x_train)}')
'''
x_val.len=10000
partial_x_train=15000
'''

# 4.1.4 训练模型
history = model.fit(partial_x_train,
                    partial_y_train,
                    epochs=20,
                    batch_size=512,
                    validation_data=(x_val, y_val))
print(f'history.history.keys(): {history.history.keys()}')
'''
history.history.keys(): dict_keys(['accuracy', 'loss', 'val_accuracy', 'val_loss'])
'''

# 绘制训练损失和验证损失
import matplotlib.pyplot as plt
history_dict = history.history
loss_values = history_dict['loss']
val_loss_values = history_dict['val_loss']
epochs = range(1, len(loss_values) + 1)
plt.plot(epochs, loss_values, 'bo', label='training loss')
plt.plot(epochs, val_loss_values, 'b', label='validation loss')
plt.title('training and validation loss')
plt.xlabel('epochs')
plt.ylabel('loss')
plt.legend()
plt.show()

# 绘制训练精度和验证精度
plt.clf()
acc = history_dict['accuracy']
val_acc = history_dict['val_accuracy']
plt.plot(epochs, acc, 'bo', label='training acc')
plt.plot(epochs, val_acc, 'b', label='validation acc')
plt.title('traning and validation accuracy')
plt.xlabel('epochs')
plt.ylabel('accuracy')
plt.legend()
plt.show()

print(model.predict(x_test))
'''
[[0.04714732]
 [1.        ]
 [0.9980222 ]
 ...
 [0.01046927]
 [0.02241139]
 [0.96911156]]
'''