#include "TrainDigit.h"

double cost_time;
clock_t start_time;
clock_t end_time;


int reverseInt(int i) {
	unsigned char c1, c2, c3, c4;

	c1 = i & 255;
	c2 = (i >> 8) & 255;
	c3 = (i >> 16) & 255;
	c4 = (i >> 24) & 255;

	return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
}

cv::Mat read_Mnist_Image(string fileName) {
	int magic_number = 0;
	int number_of_images = 0;
	int n_rows = 0;
	int n_cols = 0;

	cv::Mat DataMat;

	ifstream file(fileName, ios::binary);
	if (file.is_open())
	{
		cout << "成功打开图像集 ... \n";

		file.read((char*)&magic_number, sizeof(magic_number));
		file.read((char*)&number_of_images, sizeof(number_of_images));
		file.read((char*)&n_rows, sizeof(n_rows));
		file.read((char*)&n_cols, sizeof(n_cols));
		//cout << magic_number << " " << number_of_images << " " << n_rows << " " << n_cols << endl;

		magic_number = reverseInt(magic_number);
		number_of_images = reverseInt(number_of_images);
		n_rows = reverseInt(n_rows);
		n_cols = reverseInt(n_cols);
		cout << "MAGIC NUMBER = " << magic_number
			<< " ;NUMBER OF IMAGES = " << number_of_images
			<< " ; NUMBER OF ROWS = " << n_rows
			<< " ; NUMBER OF COLS = " << n_cols << endl;

		//-test-
		//number_of_images = testNum;
		//输出第一张和最后一张图，检测读取数据无误
		cv::Mat s = cv::Mat::zeros(n_rows, n_rows * n_cols, CV_32FC1);
		cv::Mat e = cv::Mat::zeros(n_rows, n_rows * n_cols, CV_32FC1);

		cout << "开始读取Image数据......\n";
		start_time = clock();
		DataMat = cv::Mat::zeros(number_of_images, n_rows * n_cols, CV_32FC1);
		for (int i = 0; i < number_of_images; i++) {
			for (int j = 0; j < n_rows * n_cols; j++) {
				unsigned char temp = 0;
				file.read((char*)&temp, sizeof(temp));

				if (temp > 0) temp = 1.0;
				float pixel_value = float(temp + 0.0);
				//float pixel_value = float((temp + 0.0) / 255.0);
				DataMat.at<float>(i, j) = pixel_value;

				//打印第一张和最后一张图像数据
				if (i == 0) {
					s.at<float>(j / n_cols, j % n_cols) = pixel_value;
				}
				else if (i == number_of_images - 1) {
					e.at<float>(j / n_cols, j % n_cols) = pixel_value;
				}
			}
		}
		end_time = clock();
		cost_time = (end_time - start_time) / CLOCKS_PER_SEC;
		cout << "读取Image数据完毕......" << cost_time << "s\n";

	}
	file.close();
	return DataMat;
}

cv::Mat read_Mnist_Label(string fileName) {
	int magic_number;
	int number_of_items;

	cv::Mat LabelMat;

	ifstream file(fileName, ios::binary);
	if (file.is_open())
	{
		cout << "成功打开Label集 ... \n";

		file.read((char*)&magic_number, sizeof(magic_number));
		file.read((char*)&number_of_items, sizeof(number_of_items));
		magic_number = reverseInt(magic_number);
		number_of_items = reverseInt(number_of_items);

		cout << "MAGIC NUMBER = " << magic_number << "  ; NUMBER OF ITEMS = " << number_of_items << endl;

		//-test-
		//number_of_items = testNum;
		//记录第一个label和最后一个label
		unsigned int s = 0, e = 0;

		cout << "开始读取Label数据......\n";
		start_time = clock();
		LabelMat = cv::Mat::zeros(number_of_items, 1, CV_32SC1);
		for (int i = 0; i < number_of_items; i++) {
			unsigned char temp = 0;
			file.read((char*)&temp, sizeof(temp));
			LabelMat.at<unsigned int>(i, 0) = (unsigned int)temp;

			//打印第一个和最后一个label
			if (i == 0) s = (unsigned int)temp;
			else if (i == number_of_items - 1) e = (unsigned int)temp;
		}
		end_time = clock();
		cost_time = (end_time - start_time) / CLOCKS_PER_SEC;
		cout << "读取Label数据完毕......" << cost_time << "s\n";

		cout << "first label = " << s << endl;
		cout << "last label = " << e << endl;
	}
	file.close();
	return LabelMat;
}


/*输入图片大小为 28×28*/
int predictWithSVM(cv::Mat digitImage, cv::Ptr<cv::ml::SVM> mySvm) {
	// 创建分类器并设置参数
	cout << "Predicting ..." << endl;
	if (mySvm.empty()) {
		mySvm = cv::ml::SVM::create();
		mySvm = cv::ml::SVM::load("svm.xml");
	}
	
	float res = mySvm->predict(digitImage);
	return (int)res;
}


bool loadSVM(cv::Ptr<cv::ml::SVM>* mySvm) {
	try {
		//svm = cv::ml::SVM::create();
		*mySvm = cv::ml::SVM::load("svm.xml");
		return true;
	}
	catch (exception e) {
		cout << e.what() << endl;
		return false;
	}
}

////////// 用mnist 来训练和测试

int trainAndPredict()
{
	cout << "训练数据请输入 1, 直接使用训练模型预测输入2" << endl;
	string flag = "";

	while (1) {
		cin >> flag;
		if (flag == "1" || flag == "2")
			break;
		else {
			cout << "输入1，2" << endl;
		}
	}

	// 创建分类器并设置参数
	cv::Ptr<cv::ml::SVM> SVM_params = cv::ml::SVM::create();

	if (flag == "1") {
		// 训练 加载模型
		// 读取训练样本的数据
		cv::Mat trainingDataMat;
		trainingDataMat = read_Mnist_Image("mnist_dataset/train-images.idx3-ubyte");

		//训练样本的响应值  
		cv::Mat responsesMat;
		responsesMat =read_Mnist_Label("mnist_dataset/train-labels.idx1-ubyte");

		////===============================创建SVM模型===============================////
	
		SVM_params->setType(cv::ml::SVM::C_SVC);     //C_SVC用于分类，C_SVR用于回归
		SVM_params->setKernel(cv::ml::SVM::RBF);  //LINEAR线性核函数。SIGMOID为高斯核函数

		// 注释掉部分对本项目不影响，影响因子只有两个
		//SVM_params->setDegree(0);            //核函数中的参数degree,针对多项式核函数;
		SVM_params->setGamma(0.01);       //核函数中的参数gamma,针对多项式/RBF/SIGMOID核函数; 
		//SVM_params->setCoef0(0);             //核函数中的参数,针对多项式/SIGMOID核函数；
		SVM_params->setC(10.0);              //SVM最优问题参数，设置C-SVC，EPS_SVR和NU_SVR的参数；
		//SVM_params->setNu(0);                //SVM最优问题参数，设置NU_SVC， ONE_CLASS 和NU_SVR的参数； 
		//SVM_params->setP(0);                 //SVM最优问题参数，设置EPS_SVR 中损失函数p的值. 
		//结束条件，即训练1000次或者误差小于0.01结束
		SVM_params->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 1000, 0.01));
		//Mat* responsesTransfer = new Mat(responsesMat->size().height, 1, CV_32FC1);

		//responsesMat->convertTo(*responsesMat, CV_32SC1);      类型为CV_32SC1，此处省略是因为读取的时候已指明该格式了。
		//trainingDataMat->convertTo(*trainingDataMat, CV_32F);  此处需要注意训练数据类型为 CV_32F

		//训练数据和标签的结合
		cout << "开始训练" << endl;
		cout << "训练数据长度" << trainingDataMat.size().width << " 高度 " << trainingDataMat.size().height << endl;
		cout << "标签数据长度" << responsesMat.size().width << " 高度 " << responsesMat.size().height << endl;

		//cv::Ptr<cv::ml::TrainData> tData = TrainData::create(*trainingDataMat, ROW_SAMPLE, *responsesMat);

		// 训练分类器
		SVM_params->train(trainingDataMat, cv::ml::ROW_SAMPLE, responsesMat);//训练
		SVM_params->save("svm.xml");
		
		
		cout << "训练好了！！！" << endl;
		
	}
	else if (flag == "2") {
		cout << "训练模型参数加载" << endl;
		SVM_params = cv::ml::SVM::load("svm.xml");
		//cout << SVM_params.empty() << endl;
	}
	


	cout << "-------SVM 开始预测-------------------------------" << endl;

	
	cv::Mat testData;
	cv::Mat tLabel;

	cout << "开始导入测试数据...\n";
	testData = read_Mnist_Image("mnist_dataSet/t10k-images.idx3-ubyte");
	tLabel = read_Mnist_Label("mnist_dataSet/t10k-labels.idx1-ubyte");
	cout << "成功导入测试数据！！！\n";


	float count = 0;
	cout << testData.rows << endl;
	for (int i = 0; i < testData.rows; i++) {
		cv::Mat sample = testData.row(i);
		float res = SVM_params->predict(sample);
		res = std::abs(res - tLabel.at<unsigned int>(i, 0)) <= FLT_EPSILON ? 1.f : 0.f;
		count += res;
	}
	
	cout << "正确的识别个数 count = " << count << endl;
	cout << "错误率为..." << (10000 - count + 0.0) / 10000 * 100.0 << "%....\n";

	
	//system("pause");
	return 0;
}

