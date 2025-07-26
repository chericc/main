from keras.datasets import mnist

(train_images, train_labels), (test_images, test_labels) = mnist.load_data()

my_slice = train_images[10:100]
print(my_slice.shape) # (90, 28, 28)

my_slice = train_images[10:100, :, :]
print(my_slice.shape) # (90, 28, 28)

my_slice = train_images[10:100, 0:28, 0:28]
print(my_slice.shape) # (90, 28, 28)

my_slice = train_images[:, 14:, 14:]
print(my_slice.shape) # (60000, 14, 14)

my_slice = train_images[:, 7:-7, 7:-7]
print(my_slice.shape) # (60000, 14, 14)