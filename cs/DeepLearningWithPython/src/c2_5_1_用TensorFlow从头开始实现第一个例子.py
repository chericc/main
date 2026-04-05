import tensorflow as tf
import math
import keras
from keras.datasets import mnist
import numpy as np
from tensorflow.keras import optimizers

# 实现一个简单的Python类NaiveDense，它创建了
# 两个TensorFlow变量W和b，并定义了一个__call__()方法
# 供外部调用，以实现上述变换。
class NaiveDense:
    def __init__(self, input_size: int, output_size: int, activation):
        self.activation = activation

        # 创建一个形状为(input_size, output_size)的矩阵W，并将其随机初始化
        w_shape = (input_size, output_size)
        w_initial_value = tf.random.uniform(shape=w_shape, minval=0, maxval=1e-1)
        self.W = tf.Variable(w_initial_value)

        # 创建一个形状为(output_size,)的零向量b
        b_shape = (output_size,)
        b_initial_value = tf.zeros(b_shape)
        self.b = tf.Variable(b_initial_value)

    # 前向传播
    def __call__(self, inputs):
        return self.activation(tf.matmul(inputs, self.W) + self.b)
    
    # 获取该层权重的快捷方法
    @property
    def weights(self):
        return (self.W, self.b)
    
# 创建一个NaiveSequential类，将这些层链接起来。它封装了一个层列表，
# 并定义了一个__call__()方法供外部调用。增额方法将按顺序调用输入的层。
class NaiveSequential:
    def __init__(self, layers):
        self.layers = layers

    def __call__(self, inputs):
        x = inputs
        for layer in self.layers:
            x = layer(x)
        return x
    
    @property
    def weights(self):
        weights = []
        for layer in self.layers:
            weights += layer.weights
        return weights

# 对mnist数据进行小批量迭代
class BatchGenerator:
    def __init__(self, images, labels, batch_size=128):
        assert len(images) == len(labels)
        self.index = 0
        self.images = images
        self.labels = labels
        self.batch_size = batch_size
        self.num_batches = math.ceil(len(images) / batch_size)

    def next(self):
        images = self.images[self.index : self.index + self.batch_size]
        labels = self.labels[self.index : self.index + self.batch_size]
        self.index += self.batch_size
        return images, labels


# 更新权重的目的，就是将权重沿着减小批量损失值的方向移动一小步。
# 移动幅度由学习率决定，它通常是一个很小的数
# 要实现update_weights函数，最简单的办法就是从每个权重中减去gradient * learning_rate
def update_weights(gradients, weights):
    learning_rate = 1e-3
    for g,w in zip(gradients, weights):
        w.assign_sub(g * learning_rate) # 相当于TwnsorFlow的 -=

# optimizer = tf.keras.optimizers.SGD(learning_rate=1e-3)
# def update_weights(gradients, weights):
#     optimizer.apply_gradients(zip(gradients, weights))

# 最难的一步就是训练步骤，即在一批数据上运行模型后更新模型权重。需要做到以下几点：
# （1）计算模型对图像的预测值
# （2）根据实际标签，计算这些预测值的损失值
# （3）计算损失相对于模型权重的梯度
# （4）将权重沿着梯度的反方向移动一小步
def one_training_step(model: NaiveSequential, images_batch, labels_batch):
    with tf.GradientTape() as tape:
        predictions = model(images_batch)
        lose_fn = tf.keras.losses.SparseCategoricalCrossentropy()
        per_sample_losses = lose_fn(labels_batch, predictions)
        average_loss = tf.reduce_mean(per_sample_losses)
    gradients = tape.gradient(average_loss, model.weights)
    update_weights(gradients, model.weights)
    return average_loss

def fit(model: NaiveSequential, images, labels, epochs, batch_size=128):
    for epoch_counter in range(epochs):
        print(f"Epoch {epoch_counter}")
        batch_generator = BatchGenerator(images=images, labels=labels, batch_size=batch_size)
        for batch_counter in range(batch_generator.num_batches):
            images_batch, labels_batch = batch_generator.next()
            loss = one_training_step(model, images_batch, labels_batch)
            if batch_counter % 100 == 0:
                print(f"loss at batch {batch_counter}: {loss: .2f}")

model = NaiveSequential([
    NaiveDense(input_size=28 * 28, output_size=512, activation=tf.nn.relu),
    NaiveDense(input_size=512, output_size=10, activation=tf.nn.softmax)
])
assert len(model.weights) == 4

(train_images, train_labels), (test_images, test_labels) = mnist.load_data()
train_images = train_images.reshape((60000, 28 * 28))
train_images = train_images.astype("float32") / 255
test_images = test_images.reshape((10000, 28 * 28))
test_images = test_images.astype("float32") / 255
fit(model, train_images, train_labels, epochs=10, batch_size=128)

# 评估模型

predictions = model(test_images)
predictions = predictions.numpy()
predicted_labels = np.argmax(predictions, axis=1)
matches = predicted_labels == test_labels
print(f"accuracy: {matches.mean(): .2f}")
