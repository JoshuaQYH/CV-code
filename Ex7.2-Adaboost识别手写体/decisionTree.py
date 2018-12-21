import numpy as np 
import pandas as pd
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score
from time import time
from sklearn import preprocessing

trainData = pd.read_csv("../dataSet/mnist_train.csv").values
train_data = trainData[0:60000, 1:]
train_label = trainData[0:60000, 0]
#print (train_data)
#print(train_label)

testData = pd.read_csv("../dataSet/mnist_test.csv").values
test_data = testData[0:10000, 1:]
test_label = testData[0:10000, 0]

clf1 = DecisionTreeClassifier(max_depth = 5, min_samples_split=5, min_samples_leaf=5);

print("Train1...")
start = time()
clf1.fit(train_data, train_label)
end = time()
t = end - start

predict_result = clf1.predict(test_data)
print ("DT test1 accuracy: " , accuracy_score(test_label, predict_result))

"""
clf2 = DecisionTreeClassifier();
#clf2 = DecisionTreeClassifier(max_depth=10, min_samples_split=7, min_samples_leaf=7);

print("Train2...")
start = time()
clf2.fit(train_data, train_label)
end = time()
t = end - start

predict_result = clf2.predict(test_data)
print ("DT test2 accuracy: " , accuracy_score(test_label, predict_result))
"""