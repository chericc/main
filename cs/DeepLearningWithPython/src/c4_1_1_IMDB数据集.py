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

