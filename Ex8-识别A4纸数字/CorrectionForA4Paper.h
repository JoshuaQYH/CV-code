#pragma once
/*******************************************************

Author: qiuyihao

Contact: 576261090@qq.com

*******************************************************/

#include <iostream>
#include <string>
#include "CImg.h"
#include <cstdlib>
#include <vector>
#include <ctime>
#include <cmath>
#include "utils.h"

using namespace std;
using namespace cimg_library;

#define MyPI 3.141592
#define A4width 420    //A4纸宽
#define A4height 594   //A4纸高
#define MyScalar 2       //A4纸矫正图像对于A4纸宽/高的倍数

class CorrectionForA4Paper
{
	CImg <double> originImage;   /*存储原图*/
	
	CImg <double> resultImage;   /*存储结果输出图像*/

	CImg <double> houghSpaceImage; /*霍夫空间图像*/

	vector<Point> orderPointSet;

	bool PRINT;                  /*打印标志*/

	int otsuThreshold;           /*Ostu阈值分割法的分割阈值*/
	int houghThreshold;          /*霍夫空间的阈值*/
	int houghPointDistance;      /*霍夫空间中点的距离差值小于该值，视为同一个点*/
	int _distance; 
	int _voteNum; 
	// Polar coordinate intersection at x  
	const int CrossX(int theta, int distance, int x) {
		double angle = (double)theta * MyPI / 180.0;
		double m = -cos(angle) / sin(angle);
		double b = (double)distance / sin(angle);
		return m * x + b;
	}

	// Polar coordinate intersection at y  
	const int CrossY(int theta, int distance, int y) {
		double angle = (double)theta * MyPI / 180.0;
		double m = -cos(angle) / sin(angle);
		double b = (double)distance / sin(angle);
		return ((double)(y - b) / m);
	}

	const int calDistance( Point a,Point b) {
		return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
	}

public:
	CorrectionForA4Paper() {
		otsuThreshold = 1;
		houghThreshold = 400; //250 
		houghPointDistance = 400;
		_distance = 300;
		_voteNum = 0;
		PRINT = true;
	}

	/*转为灰度图像*/
	CImg<double> ToGrayImage(CImg<double> image);

	vector<int> getInterpolationRGB(double scrX, double scrY);

	/*基于霍夫变换，在参数空间中进行投票*/
	CImg<double> houghTransform(CImg<double> image);

	/* 在投票好的霍夫空间中，筛选出四个最亮点，作为A4纸的的线, 最后返回相邻四个角点的坐标*/
	vector< Point> getLinesFromHoughSpace(CImg<double> houghSpaceImage, CImg<double> _orignImage, int threshold);

	/*根据A4纸角点，提取A4纸边缘, 返回可以正确连成矩形的四个顺序点*/
	vector< Point> drawEdgeOnA4Paper(CImg<double>& drawImage, vector< Point>PointSet);

	/*根据原图和四个点，进行仿射变换得到A4纸矫正的结果*/
	CImg<double> correctA4Paper(CImg<double> originImage, vector< Point>PointSet);

	/*整个功能的入口，输入待矫正图像，输出并保存矫正后的图像*/
	CImg<double> correct(string inputImageName, CImg<short int> edgeImage);

	vector< Point> getOrderPoints() {
		return orderPointSet;
	}
};

