from keras.datasets import mnist

(train_images, train_labels), (test_images, test_labels) = mnist.load_data()

batch = train_images[:128]
batch = train_images[128:256]

n = 3
batch  = train_images[128 * n, 128 * (n + 1)]