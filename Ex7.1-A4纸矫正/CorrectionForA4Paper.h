#pragma once
/*******************************************************

Author: qiuyihao

Contact: 576261090@qq.com

Date created: 18/12/16

Date finished: 

功能说明：输入一张含有A4纸的照片
		1. 通过图像分割提取图像边缘
		2. 霍夫变化得到A4纸四个边缘
		3. 对A4纸进行矫正,并输出结果

*******************************************************/

#include <iostream>
#include <string>
#include "CImg.h"
#include <stdlib.h>
#include <vector>
#include <time.h>
#include <math.h>
#include "utils.h"
using namespace std;
using namespace cimg_library;

class CorrectionForA4Paper
{
	CImg <double> originImage;   /*存储原图*/
	
	CImg <double> resultImage;   /*存储结果输出图像*/

	CImg <double> houghSpaceImage; /*霍夫空间图像*/


	bool PRINT;                  /*打印标志*/

	int otsuThreshold;           /*Ostu阈值分割法的分割阈值*/
	int houghThreshold;          /*霍夫空间的阈值*/
	int houghPointDistance;      /*霍夫空间中点的距离差值小于该值，视为同一个点*/

	// Polar coordinate intersection at x  
	const int CrossX(int theta, int distance, int x) {
		double angle = (double)theta*cimg::PI / 180.0;
		double m = -cos(angle) / sin(angle);
		double b = (double)distance / sin(angle);
		return m * x + b;
	}

	// Polar coordinate intersection at y  
	const int CrossY(int theta, int distance, int y) {
		double angle = (double)theta*cimg::PI / 180.0;
		double m = -cos(angle) / sin(angle);
		double b = (double)distance / sin(angle);
		return ((double)(y - b) / m);
	}

	const int calDistance(Point a, Point b) {
		return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
	}

public:
	CorrectionForA4Paper() {
		otsuThreshold = 1;
		houghThreshold = 300;
		houghPointDistance = 50;
		PRINT = true;
	}

	/*转为灰度图像*/
	CImg<double> ToGrayImage(CImg<double> image);

	/*修改ostu法初始阈值*/
	void setOstThreshold(int threshold);

	/*Ostu法获取最佳阈值*/
	int getBestThresholdWithOStu(CImg<double> image);

	/*根据最佳阈值进行分割*/
	CImg<double> getSegmentedImage(CImg<double> image, int threshold);

	/*基于8邻域的方法提取二值化图像边缘*/
	CImg<double> getEdgeImage(CImg<double> image);

	/*基于霍夫变换，在参数空间中进行投票*/
	CImg<double> houghTransform(CImg<double> image);

	/* 在投票好的霍夫空间中，筛选出四个最亮点，作为A4纸的的线, 最后返回相邻四个角点的坐标*/
	vector<Point> getLinesFromHoughSpace(CImg<double> houghSpaceImage, CImg<double> _orignImage, int threshold);

	/*根据A4纸角点，提取A4纸边缘, 返回可以正确连成矩形的四个顺序点*/
	vector<Point> drawEdgeOnA4Paper(CImg<double>& drawImage, vector<Point> pointSet);

	/*根据原图和四个点，进行仿射变换得到A4纸矫正的结果*/
	CImg<double> correctA4Paper(CImg<double> originImage, vector<Point> pointSet);

	/*整个功能的入口，输入待矫正图像，输出并保存矫正后的图像*/
	void Correction(string inputImageName, string outputImageName);

};

