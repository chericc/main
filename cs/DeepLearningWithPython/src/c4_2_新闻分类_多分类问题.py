from tensorflow.keras.datasets import reuters


(train_data, train_lables), (test_data, test_labels) = \
    reuters.load_data(num_words = 10000)

print(f'train_data.len = {len(train_data)}')
print(f'test_data.len = {len(test_data)}')
'''
train_data.len = 8982
test_data.len = 2246
'''

