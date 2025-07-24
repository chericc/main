from keras.datasets import mnist
import matplotlib.pyplot as plt

(train_images, train_labels), (test_images, test_labels) = mnist.load_data()

print(train_images.ndim) # 3
print(train_images.shape) # (60000, 28, 28)
print(train_images.dtype) # uint8

print(train_labels[4])
digit = train_images[4]
plt.imshow(digit, cmap='binary')
plt.show()
