/**************************************************
* Author: qiuyh
* Date: 2018/10/19
* Notes:  对二值图像进行霍夫变换，反向输出到RGB图
****************************************************/
#ifndef _HOUGH_FOR_LINE_H_
#define _HOUGH_FOR_LINE_H_
#include <iostream>
#include <string>
#include <stdlib.h>
#include "CImg.h"
#include "Canny.h"
#include <math.h>
#include <vector>
#include <time.h>

#define PI 3.141592
using namespace cimg_library;
using namespace std;

class HoughForLine {
private:
	int number;                        /*图片编号*/
	int Width;                         /*图像宽度和高度*/
	int Height;              


	CImg<short int> detectedEdgeImage; /*边缘检测后的二值图像*/
	CImg<short int> deltaXImage;	   /*x方向的偏导图*/
	CImg<short int> deltaYImage;       /*y方向的偏导图*/
	CImg<short int> originImage;       /*原图*/

	struct dot {                       /*像素点坐标，梯度方向值*/
		int x;
		int y;    
		int gradientDirectionValue;  
		bool foundedFlag;               /*为真，则表示该点已经属于某一条直线，失去投票权了。*/
	};

	struct rgbLine{                     /*rgb空间直线斜率截距,投票数*/
		float k1;
		float k2;                       /* y = (k1 / k2) * x + b*/
		float b;   
		float x0;                      /*x0: 当k2等0 是， x = x0, 其他情况无意义*/
		bool isLine;
		float sita;   // 直线倾斜角
		int x_sita;   // 霍夫空间中的x轴坐标
		float dm;    // 直线到圆心的距离
		int y_dm;	 // 霍夫空间中的y轴坐标
		int voteNum;

	};

	vector<dot> edgeDotsSet;              /*存储边缘点*/
	vector<rgbLine> rgbLineSet;            /*存储rgb空间直线的斜率和截距,投票数*/

	struct Line {
		float sita;   // 直线倾斜角
  		int x_sita;   // 霍夫空间中的x轴坐标
		float dm;    // 直线到圆心的距离
		int y_dm;	 // 霍夫空间中的y轴坐标
		int voteNum;  
	};
	vector<Line> houghLineSet; 
	int houghWidthUnitNum;
	int houghHeightUnitNum;
	int houghWidth;
	int houghHeight;

	/*参数*/
	int minDistance;                      /*两点构成线的最小距离*/
	float minDistanceRate;                /* 根据边缘点数目确定最小距离的比例系数*/

	int similiarLineThreshold;           /*近似直线阈值*/
	int directionThreshold;               /*梯度方向阈值*/
	int slopeThreshold;                   /*斜率阈值*/

	int voteNumThreshold;                 /*投票阈值*/
	float voteNumThresholdRate;           /*根据边缘点数目，确定投票阈值的系数*/

	int MaxChooseLineNum;                 /*随机找点最大次数*/
	float MaxChooseLineRate;              /*随机找点的最大次数比例系数*/
	int limitedLine;

	Canny detector;                      /*边缘检测器*/
	int thigh;
	int tlow;
	float smoothFactor; 

	bool DEBUG; 
public:

	HoughForLine() {
		DEBUG = true;
		limitedLine = 4;
		directionThreshold = 1;    // 方向差别：筛选掉梯度差别大的点  
		slopeThreshold = 0.2;       // 斜率差别值：斜率差在该值以下，认为共线，如果出现经常不共线的情况可以考虑增大
		similiarLineThreshold = 10;  // 直线相似度: 出现较多重叠的直线的时候就需要变大
		
		// 根据图像尺寸确定两点构成线最短距离 	minDistance = sqrt(Width * Width + Height * Height) / 20;
		minDistanceRate = 0.1;
		voteNumThresholdRate = 0.005;      // 投票阈值根据图像高度宽度数目确定
		MaxChooseLineRate = 1;        // 根据图像高度宽度数目确定找点最大次数
		number = 1;
		srand((unsigned int)time(NULL));
		
		smoothFactor = 3;

		 houghWidthUnitNum = 180;
		 houghHeightUnitNum = 400 ;
		 houghWidth = PI;
		 houghHeight = 3000;

	}

	void setVoteNumRate(float r) {
		voteNumThresholdRate = r;
	}

	void setMaxChoose(float r) {
		MaxChooseLineRate = r;
	}
	void sethighAndlow(float low, float high) {
		thigh = high;
		tlow = low;
	}
	// 修改高斯模糊因子
	void setBoost(float h) {
		smoothFactor = h;
	}

	/*保存图片*/
	void saveImage(string ImageName, CImg<short int> image);

	/*存储边缘点*/
	void saveEdgePoint(CImg<short int> detectedEdgeImage, CImg<short int> deltaXImage,
		CImg<short int> deltaYImage, vector<dot>* edgeDotsSet);

	/*判断三点是否共线*/
	bool judgeOnTheSameLine(int x1, int y1, int x2, int y2, int x3, int y3);

	/*随机选取两个边缘点，构成投票值高的直线*/
	void chooseProperPointForLine(vector<dot>* edgeDotsSet);

	/*输出直线方程*/
	void outputLineEquation();

	/*根据方程绘制图像*/
	void drawLine(vector<rgbLine> rgbLineSet);

	/*绘制出直线方程上的边缘点*/
	void drawEdge(vector<dot> edgeDotsSet);

	/*画出角点*/
	void drawCrossPoint();

	/*执行整个hough变换的过程*/
	void hough(string inputFileName);

};

#endif