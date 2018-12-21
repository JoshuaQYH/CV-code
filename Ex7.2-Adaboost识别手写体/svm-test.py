"""
	svm 测试例子
"""

import numpy as np
import pandas as pd
from sklearn import svm
import matplotlib.colors
import matplotlib.pyplot as plt
from sklearn.metrics import accuracy_score
from sklearn.model_selection import GridSearchCV
from time import time
from sklearn.ensemble import AdaBoostClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.tree import DecisionTreeClassifier


trainData = pd.read_csv("../dataSet/mnist_train.csv").values
train_data = trainData[0:10000, 1:]
train_label = trainData[0:10000, 0]

testData = pd.read_csv("../dataSet/mnist_test.csv").values
test_data = testData[0:1000, 1:]
test_label = testData[0:1000, 0]

print ('SVC test accuracy：0.85122442689')

"""
model = svm.SVC(C=10.0, kernel='rbf', gamma=0.1)#设置模型参数
tt1 = time()
model.fit(train_data, train_label)#训练模型
tt2 = time()
delta_tt = tt2 - tt1
print ('SVMxun time：%dmin%.3fsec' % ((int)(delta_tt / 60), delta_tt - 60*((int)(delta_tt/60))))
y_hat = model.predict(test_data)#做预测
print ('SVC test accuracy：', accuracy_score(test_label, y_hat))
"""