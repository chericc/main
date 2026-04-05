from keras.datasets import mnist
import keras
from keras import layers
import os

os.environ["CUDA_VISIBLE_DEVICES"] = "-1"

(train_images, train_labels), (test_images, test_labels) = mnist.load_data()

# 这个模型包含两个链接在一起的Dense层，每层都对输入数据做一些简单的张量运算，这些运算
# 都涉及权重张量。权重张量是该层的属性，里面保存了模型所学到的知识。
model = keras.Sequential([
    layers.Dense(512, activation="relu"),
    layers.Dense(10, activation="softmax")
])

# 模型编译
# 其中，sparse_categorical_crossentropy是损失函数，是用于学习权重张量的反馈信号，在训练过程中应使其最小化。
# 降低损失值是通过小批量随机梯度下降来实现的。梯度下降的具体方法由第一个参数给定，即rmsprop优化器。
model.compile(optimizer='rmsprop',
            loss='sparse_categorical_crossentropy',
            metrics=["accuracy"])

# 输入数据保存在float32类型的NumPy张量中，其形状分别为(60000, 784)（训练数据）和（）(10000, 784)（测试数据）
train_images = train_images.reshape((60000, 28 * 28))
train_images = train_images.astype("float32") / 255
test_images = test_images.reshape((10000, 28 * 28))
test_images = test_images.astype("float32") / 255

# 训练循环
# 模型开始在训练数据上进行迭代（每个小批量包含128个样本），共迭代5轮（在所有训练数据上迭代一次叫作一轮）。
# 对于每批数据，模型会计算损失相对于权重的梯度（利用反向传播算法，这一算法源自微积分的链式法则），并将权重沿着减小该批量对应损失值的方向移动。
# 5轮之后，模型共执行2345次梯度更新（每轮469次），模型损失值将变得足够小，使得模型能够以很高的精度对手写数字进行分类。
model.fit(train_images, train_labels, epochs = 5, batch_size = 128)

# simple test
test_digits = test_images[0:10]
predictions = model.predict(test_digits)
print(predictions[0])
print(test_labels[0])

# verify
test_loss, test_acc = model.evaluate(test_images, test_labels)
print(f'test_acc: {test_acc}')