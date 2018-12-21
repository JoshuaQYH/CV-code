"""
    Description: 以决策树分类器为弱分类器，使用adaboost进行分类器增强
    Author: qiuyihao
    Date: 18/11/19
"""
import numpy as np
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score
from sklearn.base import ClassifierMixin 
from sklearn.utils import  check_array, check_X_y, check_random_state
from sklearn.ensemble.weight_boosting import BaseWeightBoosting 
from numpy.core.umath_tests import inner1d

class AdaboostClassiferWithDT(ClassifierMixin,BaseWeightBoosting):
    """
    @ self.param:
        train_num: 训练的迭代的次数
        n_estimators: 分类器的个数，等于迭代的次数
        learning_rate:  学习率，在确定最优（误分率最小的）分类器的时候使用
        train_data: 保存的训练数据
        train_labels: 保存训练标签
        train_samples_num: 训练样本的数目
        vector_columns: 样本特征向量的维度
        classifer_sets: 学习得到的弱分类器集合
        sample_weights: 学习得到的样本的权重集合
        alpha: 弱分类器的权重集合
        base.estimator: 使用的弱分类器，默认为决策树分类器
        random_state: 用于产生随机的弱分类器的种子
        estimator_params: 分类器的参数
    @ self.function:
        fit: 训练数据接口
        _boost: 训练弱分类器接口
        predict：预测某一样本接口
        score：输入预测数据，输出测试准确率
    """

    def __init__(self, n_estimators = 50, learn_rate = 0.01,max_depth = 5,min_samples_split=5, min_samples_leaf=5):
        self.train_num = n_estimators        # 迭代次数
        self.n_estimators = n_estimators   # 学习到的弱分类器的个数
        self.learning_rate = learn_rate         # 学习率

        base_estimator = DecisionTreeClassifier(max_depth=max_depth, 
            min_samples_split=min_samples_split, min_samples_leaf=min_samples_leaf)  # 注明分类器的类型
        
        # self.random_state = None                    # 用于产生初始的决策树分类器

        # 0.05  5 5 5   0.89 
        # 0.05  8 8 8   0.91
        # 0.5   5 5 5   0.84
        # 0.05  10 10 10 0.8996
        # 0.01  10 10 10 0.91     
        # 0.01  8 8 8   0.89
        # 0.10  8 8 8   0.90

        self.base_estimator_ = base_estimator   
        self.random_state = None                 
        self.estimator_params = tuple()
        self.estimators_ = []                         # 弱分类器的集合

    def init_args(self, train_data, train_labels):
        self.train_data = train_data
        self.train_labels = train_labels 
        self.train_samples_num, self.vector_columns = train_data.shape # 驯良样本数目, 样本特征向量的维度 

        self.classifier_sets = []  # 学得的弱分类器的集合

        self.sample_weights = [1.0 / self.train_samples_num] * self.train_samples_num # 训练样本的权重集合

        self.alpha = []  # 弱分类器的权重集合

    # 输入误分率，计算在这一轮迭代中的alpha值，即当前习得的分类器的权重值
    def cal_alpha(self, error):
        return 0.5 * np.log((1 - error) / error)

    #计算规范化因子，在更新权重的时候会使用到
    def cal_normalize_factor(self,weights,alpha,classifiers):  # 第i个样本的classifers[i] 表示分类结果
        return sum([weights[i] * np.exp(-1 * alpha * self.train_labels[i] * classifiers[i]) for i in range(self.train_samples_num)])    


    # 更新样本的权重值
    def cal_weight(self, alpha, classifier, normalize_factor):
        for i in range(self.train_samples_num):
           self.sample_weights[i] = self.sample_weights[i] * np.exp(-1 * alpha * self.train_labels[i] * classifier[i]) / normalize_factor


    # 训练弱分类器
    """
        输入：训练样本，标签，样本权重
        输出：分类器，误分因子， 分类结果
    """
    def _boost(self, iboost, X, y, sample_weight, random_state):
        
        # 随机生成决策树分类器
        estimator = self._make_estimator(random_state=random_state)

        estimator.fit(X, y, sample_weight=sample_weight)

       
        if iboost == 0:
            self.classes_ = getattr(estimator, 'classes_', None)
            self.n_classes_ = len(self.classes_)

        y_predict_proba = estimator.predict_proba(X)
        y_predict = self.classes_.take(np.argmax(y_predict_proba, axis=1),
                                       axis=0)
        # 不正确的分类结果
        incorrect = y_predict != y

        # 误分率
        estimator_error = np.mean(
            np.average(incorrect, weights=sample_weight, axis=0))

        # 如果分类结果误分率为0，可以跳出训练
        if estimator_error <= 0:
            return sample_weight, 1., 0.

        # Construct y coding as described in Zhu et al [2]:
        #
        #    y_k = 1 if c == k else -1 / (K - 1)
        #
        # where K == n_classes_ and c, k in [0, K) are indices along the second
        # axis of the y coding with c being the index corresponding to the true
        # class label.
        n_classes = self.n_classes_
        classes = self.classes_
        y_codes = np.array([-1. / (n_classes - 1), 1.])
        y_coding = y_codes.take(classes == y[:, np.newaxis])

        proba = y_predict_proba  # alias for readability
        proba[proba < np.finfo(proba.dtype).eps] = np.finfo(proba.dtype).eps

    
        # 分类器的权重，使用multi-class AdaBoost 算法
        estimator_weight = (-1. * self.learning_rate
                                * (((n_classes - 1.) / n_classes) *
                                   inner1d(y_coding, np.log(y_predict_proba))))


        # 如果下一步需要继续训练，那么增加样本权重
        if not iboost == self.n_estimators - 1:
            # 只更新正权重，不更新负权重
            sample_weight *= np.exp(estimator_weight *
                                    ((sample_weight > 0) |
                                     (estimator_weight < 0)))

        self.classifier_sets.append(estimator)
        self.alpha.append(estimator_weight)

        return sample_weight, 1., estimator_error

    """
        训练接口:
        输入： 训练样本集， 标签集
    """
    def fit(self, X, y):
        self.init_args(X, y)
        random_state = check_random_state(self.random_state) 

        self.estimators_ = []
        self.estimator_weights_ = np.zeros(self.n_estimators, dtype = np.float64)
        self.estimator_errors_ = np.ones(self.n_estimators, dtype=np.float64)
        
        sample_weight=None
        if sample_weight is None:
            # 初始样本权重
            sample_weight = np.empty(X.shape[0], dtype=np.float64)
            sample_weight[:] = 1. / X.shape[0]
        else:
            sample_weight = check_array(sample_weight, ensure_2d=False)
            # 权重正则化
            sample_weight = sample_weight / sample_weight.sum(dtype=np.float64)

            # 检查样本权重和是否为负
            if sample_weight.sum() <= 0:
                raise ValueError(
                    "Attempting to fit with a non-positive "
                    "weighted number of samples.")

        for iboost in range(self.train_num):  # 训练次数
            # 训练得到弱分类器（决策树分类器）
            # 样本权重集合，弱分类器的权重，弱分类器的误分因子
            sample_weight, estimator_weight, estimator_error = self._boost(iboost, X, y, sample_weight, random_state)
            
            if sample_weight is None:
                break

            self.estimator_weights_[iboost] = estimator_weight
            self.estimator_errors_[iboost] = estimator_error

            if estimator_error == 0: # 误分率为0，结束
                break
            
            sample_weight_sum = np.sum(sample_weight)
            if sample_weight_sum <= 0:  # 整体样本权重为负时，终止
                break
           
        return self 

    """
        预测接口：
        输入：一个样本数据
        输出：预测结果
    """
    def predict(self, test_data):
        result = 0
        voteList = [0 for i in range(10)]  
        for i in range(len(self.classifier_sets)):
            classifer = self.classifier_sets[i]
            test_value = np.array(test_data).reshape(1, len(test_data)).tolist() # 二维形式
            predicted_value =int(classifer.predict(test_value))
            voteList[predicted_value] += self.estimator_weights_[i]  # 投票的思想 按分类器权值投票
        result = voteList.index(max(voteList))
        return result


    """
        使用测试数据和标签评估
    """
    def score(self, test_data, test_labels):
        right_num = 0
        for i in range(len(test_data)):
            if self.predict(test_data[i]) == test_labels[i]:
                right_num += 1
        return right_num / len(test_data)

