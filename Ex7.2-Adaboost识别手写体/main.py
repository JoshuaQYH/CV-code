from AdaboostWithDT import AdaboostWithDT
from adaboostDTClf import AdaboostClassiferWithDT
import numpy as np 
import pandas as pd
from time import time
from sklearn.metrics import accuracy_score

trainData = pd.read_csv("../dataSet/mnist_train.csv").values
train_data = trainData[0:60000, 1:]
train_label = trainData[0:60000, 0]

"""
testData = pd.read_csv("../dataSet/mnist_test.csv").values
test_data = testData[0:10000, 1:]
test_label = testData[0:1000, 0]
"""

testData = pd.read_csv("digit.csv").values
test_data = testData[0:10, 1:]
test_label = testData[0:10, 0]


clf = AdaboostClassiferWithDT(n_estimators = 80, learn_rate = 0.05,
    max_depth = 5, min_samples_split=5, min_samples_leaf=5)
print("Train1...")
start = time()
clf.fit(train_data, train_label)
end = time()
t = end - start

print ("DT test accuracy: " , clf.score(test_data,test_label))
print ('AdaBoost-DT train：%dmin%.3fsec' %  ((int)(t/60), t-60*(int)(t/60)))

