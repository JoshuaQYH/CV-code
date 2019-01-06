#include <iostream>
#include "Canny.h"
#include "utils.h"
#include "CorrectionForA4Paper.h"
#include "DigitSpilt.h"
#include <direct.h>
#include <fstream>
#include <sstream>
#include "TrainDigit.h"  // 训练接口
#include <opencv2/highgui/highgui.hpp>

using namespace std;
cv::Ptr<cv::ml::SVM> MySvm;


// cimg
void cimg2Mat(cv::Mat* mat, CImg<double> img) {
	mat = new cv::Mat(img._width, img._height, CV_32FC1);
	for (int i = 0; i < img._width; i++) {
		for (int j = 0; j < img._height; j++) {
			if (img(i, j) > 0) mat->at<float>(i, j) = 1;
			else 
				mat->at<float>(i, j) = img(i, j);
		}
	}
}
                                                      /// 16340186  phone id
void getNum(string inputImage, string directory,vector<string> labelString) {
		cout << inputImage << endl;
		string studentID = inputImage.substr(inputImage.size() - 12, 8);
		cout << studentID << endl;
		ofstream outFile;
		outFile.open("data.csv", ios::app);
		outFile << ",\n";
		outFile << studentID + ".jpg" << ",";
	
		int totalNum = (labelString[0].size() + labelString[1].size() + labelString[2].size());
		int rightNum = 0;

	try {
	
		Canny c;
		CorrectionForA4Paper correct;
		DigitSpilt ds;

		// 边缘图
		CImg<short int> edgeImage = c.canny(inputImage);
		edgeImage.display("canny 检测边缘");
		// 矫正后的A4纸
		CImg<double> a4Paper = correct.correct(inputImage, edgeImage);
		a4Paper.display("矫正的A4纸");

		// 保存矫正后的A4纸
		string a4name = "./result/A4/" + studentID;
		_mkdir("./result/A4/");
		a4name = a4name + ".bmp";
		a4Paper.save(a4name.c_str());

		// 四个角点坐标
		vector<Point> orderPoints = correct.getOrderPoints();
		
		for (int i = 0; i < orderPoints.size(); i++) {
			outFile << "(" << orderPoints[i].x << "  " << orderPoints[i].y << "),";
		}

		// a4 纸灰度化
		CImg<double> grayA4Paper = correct.ToGrayImage(a4Paper);

		// 自适应阈值处理
		CImg<double> adaptiveImg = ds.adaptiveThreshold(grayA4Paper);
		//adaptiveImg.display("自适应阈值处理");

		// 膨胀运算，修复断裂字符
		CImg<double> dilatedImg = ds.dilation(adaptiveImg);
		dilatedImg = ds.dilation(dilatedImg);
		//dilatedImg.display("膨胀运算");

		// 消除短边缘
		CImg<double> pureImg = dilatedImg;
		c.cutShortEdge(&dilatedImg, &pureImg, 3);
		//pureImg.display("去除短边缘。");

		// 水平投影
		CImg<double> verImage = ds.hor_projection(pureImg);
		//verImage.display();

		// 计算多条横向划分线
		vector<int> horLineSet = ds.hor_partion(verImage);

		// 分多行行数据处理
		// 得到从左到右，从上到下的单个数字字符
		int orderNum = 0; //  字符在文件夹的序号
		for (int i = 0; i < horLineSet.size(); i++) {
			if (horLineSet[i] == -1) continue;
			int row = horLineSet[i];
			double color[] = { 0,255,255 };
			for (int j = 0; j < a4Paper._width; j++) {
				a4Paper(j, horLineSet[i], 0) = 255;
				a4Paper(j, horLineSet[i], 1) = 0;
				a4Paper(j, horLineSet[i], 2) = 0;
			}
		}

		double red[] = { 255, 0, 0 };
		//a4Paper.display();
 
		int predictFlag = 0;
		for (int i = 0; i < horLineSet.size(); i = i + 2) {
			cout << horLineSet[i] << " " << horLineSet[i + 1] << endl;
			
			// 0 学号  1 手机号 2身份证
			string labels;
			if (predictFlag < 3) {
				labels = labelString[predictFlag];
				predictFlag++;
			}
			else {
				break; 
			}
			
			if (horLineSet[i] == -1) continue;

			// 划分子图
			CImg<double> horizonImage = ds.lineImage(horLineSet[i], horLineSet[i + 1], adaptiveImg);

			// 竖直投影
			CImg<double> horImage = ds.ver_projection(horizonImage);

			// 竖直划分, 绘图
			vector<int> verLineSet = ds.ver_partion(horImage);
			for (int j = 0; j < verLineSet.size(); j = j + 2) {
				a4Paper.draw_line(verLineSet[j], horLineSet[i], verLineSet[j], horLineSet[i + 1], red);
				a4Paper.draw_line(verLineSet[j + 1], horLineSet[i], verLineSet[j + 1], horLineSet[i + 1], red);
			}

			// 划分得到子图
			vector<CImg<double>> digitImage = ds.cutImage(horizonImage, verLineSet);

			string predictNum1 = ""; // 预测的字符串 学号
			string predictNum2 = ""; // 预测的字符串 手机号
			string predictNum3 = ""; // 预测的字符串 身份证

			for (int j = 0; j < digitImage.size(); j++) {

				if (j > labels.size()) break;

				char savePath[50] = { 0 };
				char dir[50] = { 0 };

				string DIR = "";
				if (predictFlag == 1) DIR = "learnID";
				if (predictFlag == 2) DIR = "phoneNum";
				if (predictFlag == 3) DIR = "identity";

				sprintf_s(dir, "./%s/%s/", directory.c_str(), studentID.c_str());
				_mkdir(dir);
				// 创建切割字符的文件夹
				sprintf_s(dir, "./%s/%s/%s/", directory.c_str(), studentID.c_str(), DIR.c_str());
				_mkdir(dir);
				cout << dir << endl;
				// 保存切割后的字符
				sprintf_s(savePath, "./%s/%s/%s/%d.bmp", directory.c_str(), studentID.c_str(), DIR.c_str(), orderNum++);

				CImg<double> img = digitImage[j].resize(28, 28);
				img = ds.dilation(img);
				img = ds.dilation(img);
				img = ds.dilation(img);


				// 转为 mat
				img.save(savePath);
				cv::Mat* mat = nullptr;
				cv::Mat* tmp = nullptr;
				mat = new cv::Mat(1,img._width * img._height, CV_32FC1);
				tmp = new cv::Mat(img._height, img._width, CV_32FC1);

				for (int i = 0; i < img._height; i++) {
					for (int j = 0; j < img._width; j++) {
						if (img(j, i) > 0) {
							mat->at<float>(0, i * img._width + j) = float(1.0);
							tmp->at<float>(i, j) = float(1.0);
						}
						else {
							mat->at<float>(0, i * img._width + j) = float(0.0);
							tmp->at<float>(i, j) = float(0.0);
						}
							
					}
				}
				//cv::namedWindow("digit", CV_WINDOW_AUTOSIZE);
				//cv::imshow("digit", *tmp);
				//cout << tmp->size().width << " " << tmp->size().height << endl;
				//cv::waitKey(0);
				//cv::namedWindow("digit", CV_WINDOW_AUTOSIZE);
				//cv::imshow("digit", *mat);
				//cv::waitKey(0);
				//cout << *mat << endl;
				//cimg2Mat(src_img, tmpImg);
				
				int predictValue = predictWithSVM(*mat, MySvm);
				string predictStr = to_string(predictValue);
				if (predictValue + '0' == labels[j]) rightNum++;

				if (predictFlag == 1) predictNum1 += predictStr;  // 保存每一行的预测值
				if (predictFlag == 2) predictNum2 += predictStr;  // 保存每一行的预测值
				if (predictFlag == 3) predictNum3 += predictStr;  // 保存每一行的预测值
				delete mat;
				mat = NULL;
				cout << "predicted value:" << predictValue << " label:" << labels[j] << endl;
			}
			if (predictFlag == 1) outFile << predictNum1 << ",";   // 保存每一行的预测值
			if (predictFlag == 2) outFile << predictNum2 << ",";// 保存每一行的预测值
			if (predictFlag == 3) outFile << predictNum3 << ","; // 保存每一行的预测值
			
			if (predictFlag == 1) cout << "该行预测数字为" << predictNum1 << " 实际值为：" << labels << endl;
			if (predictFlag == 2) cout << "该行预测数字为" << predictNum2 << " 实际值为：" << labels << endl;
			if (predictFlag == 3)cout << "该行预测数字为" << predictNum3 << " 实际值为：" << labels << endl;
			
		}
		float accuracyRate = rightNum * 1.0 / totalNum;
		cout << "该张A4纸识别正确个数为 " << rightNum << endl;
		cout << "总共切割得到的字符数为:" << orderNum << endl;
		cout << "准确率为：" << accuracyRate << endl;
		//a4Paper.display();
		a4name = "./result/spilit/" + studentID;
		a4name = a4name + ".bmp";
		_mkdir("./result/spilit/");
		a4Paper.save(a4name.c_str());

		string Str = to_string(accuracyRate);
		outFile << "\"accuracy rate:" + Str << "\",";
		//outFile << endl;
		outFile.close();

	}
	
	catch (exception e) {
		cout << e.what() << endl;
		throw e;
		float accuracyRate = rightNum * 1.0 / totalNum;
		stringstream ss;
		ss >> accuracyRate;
		string Str;
		ss << Str;
		outFile << "\"ERROR"<<  "\",";
		//outFile << endl;
		outFile.close();
	}
	outFile << ",\n";
}


int main() {
	//加载标签
	//trainAndPredict();
	MySvm = cv::ml::SVM::create();
	loadSVM(&MySvm);
	
	ifstream labelFile("labels.csv", ios::in);
	string lineStr;
	vector<vector<string>> dataLabels;
	while (getline(labelFile, lineStr)) {
		//cout << lineStr;
		vector<string> lineLabel;
		string num;
		int dotFlag = 0;  // , 号标记
		//cout << lineStr << endl;
		for (int i = 0; i < lineStr.size(); i++) {
			if (dotFlag == 0) {
				if (lineStr[i] == ',') {
					lineLabel.push_back(num);
					num = "";
					dotFlag = 1;
				}
				else {
					num += lineStr[i];
				}
			}
			else if (dotFlag == 1) {
				if (lineStr[i] == ',') {
					lineLabel.push_back(num);
					num = "";
					dotFlag = 2;
				}
				else {
					num += lineStr[i];
				}
			}
			else if (dotFlag == 2) {
				if (lineStr[i] == ',') {
					lineLabel.push_back(num);
					num = "";
					dotFlag = 3;
				}
				else {
					num += lineStr[i];
				}
			}
			else if(dotFlag == 3) {
				break;
			}
		}
		dataLabels.push_back(lineLabel);
	}

	
	vector<string> filesPath;
	// 获取预测图片的路径
	for (int i = 0; i < dataLabels.size(); i++) {
		if (dataLabels[i][0] == "") break;
		string path = "./testData/";
		path += dataLabels[i][0] + ".bmp";
		filesPath.push_back(path);
	}
	//getNum("./testData/15331178.bmp", "result", dataLabels[5]);
	vector<string> errorFileList;
	for (int i = 0; i < filesPath.size() - 1; i++) {
		try {
			// 图片路径  识别结果存放目录  数据标签
			cout << "开始识别 " << filesPath[i] << endl;
			getNum(filesPath[i], "result", dataLabels[i]);
		}
		catch (exception e) {
			cout << e.what() << endl;
			cout << filesPath[i] + "图像检测错误,检测下一张。" << endl;
			errorFileList.push_back(filesPath[i]);
		}
	}
	
	system("pause");
	
}