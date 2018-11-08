#pragma once
#ifndef _HOUGH_FOR_CIRCLE_H_
#define _HOUGH_FOR_CIRCLE_H_

#include "Canny.h"
#include "CImg.h"
#include <vector>
#include <math.h>
#include <time.h>
class HoughForCircle {
private:
	/*点数据结构*/
	struct Point {                         /* 边缘点结构体,坐标（x,y) */
		int x;
		int y;
		float gradientDirectionValue;       /*边缘点的梯度值*/
		bool foundedFlag;                   /*投票的标志，为真则该点投票无效*/
	};
	vector<Point> edgePointSet;            /* 边缘点集合 */
	/*圆数据结构*/
	struct Circle {                        /* 圆结构体， 圆心（a,b), 半径 r*/
		int a;
		int b;
		int r;
	};
	vector<Circle> haveVerifiedCircleSet;    /*存储检测后符合条件的圆*/
	 
	/*图像检测相关对象*/
	Canny detector;                         /*边缘检测器*/
	CImg<short int> edgeImage;              /* 利用canny得到的边缘图 */
	CImg<short int> originImage;            /*原图*/
	CImg<short int> houghImage;             /* 霍夫变换后的图像结果*/
	CImg<short int> deltaXImage;            /*x,y方向偏导图*/
	CImg<short int> deltaYImage;         
	Circle beingVerifiedCircle;             /* 当前被检测的圆 */

	/*重要参数*/
	int voteNumThreshold;                          /* 真假圆的检测阈值*/
	float voteNumThresholdRate;                      /* 真假圆的检测系数，根据图像尺寸来确定*/

	int maxTimes;                                /*循环检测的最大次数，检测圆个数的终止标志*/
	float maxTimesRate;                            /*最大检测次数的相关系数，根据图像尺寸来确定最大检测次数*/
	int stopFlag;                                /*终止标志*/

	int upperOfLosing;                          /*检测相似圆的个数的上限*/

	int circleCenterDistanceThreshold;           /*选取点时，点梯度所在直线与另外一点中垂线交点的距离小于这个阈值，则认为这两点符合条件*/

	float edgeWidth;                             /*容许的边缘宽度值*/
	int circleCenterDistance;                    /*检测近似圆心的距离值*/
	

	/**/
	int numOfCircle;                           /*图片中圆的个数*/
	bool DEBUG;                                /* debug需要，为真打印执行语句*/

	int width;                                 /*图像宽度*/
	int height;                                /*图像高度*/


public:

	HoughForCircle() {
		voteNumThreshold = 0;
		voteNumThresholdRate = 0.60;  //voteNumThreshold = ceil( 2 * PI * r * rate);
		maxTimesRate = 0.1;
		edgeWidth = 2;
		upperOfLosing = 1000;
		stopFlag = 0;
		circleCenterDistance = 10;   
		DEBUG = false;
		circleCenterDistanceThreshold = 20;
		srand((unsigned int)time(0));
	}

	/*修改参数*/
	void setRate(float r) {
		maxTimesRate = r;
	}

	void setDebug(bool tf) {
		DEBUG = tf;
	}

	void setEdgeWidth(int e) {
		edgeWidth = e;
	}

	/*根据图像名，保存图片*/
	void saveImage(string fileName, CImg<short int> img);

	/*读取边缘二值图，并且将边缘点坐标存储下来,同时计算梯度，也保存梯度信息*/
	void saveEdgePoint(CImg<short int> edgeImage, vector<Point>* edgePointSet);

	/*判断三点是否共线，共线返回真，共线需要重新选择*/
	bool judgeOnLine(int x1, int y1, int x2, int y2, int x3, int y3);

	/*挑选三个边缘点，计算参数圆心位置(a,b)，半径r*/
	void calculate_a_b_r(Circle* beingVerifiedCircle, vector<Point> edgePointSet, vector<Circle> haveVerifiedCircleSet);

	/*判断随机得到的圆是否与当前检测成功圆相近，如果相近那么重新挑选*/
	bool chooseCircle(Circle begingVerifiedCircle, vector<Circle> haveVerifiedCircle);

	/*利用步骤3的圆心位置半径，判断所有边缘点到圆的距离，进行投票
	票数大于一定值时认为是真实的圆，继续步骤3，虚假圆就继续步骤3.到一定的圆数就终止查找*/
	void vote(int voteNumThreshold, vector<Point> edgePointSet, Circle * beingVerifiedCircle, vector<Circle> haveVerifiedCircleSet);

	/*输出参数方程*/
	void inputCircleEquation(vector<Circle> haveVerifiedCircleSet);

	/* 根据参数方程绘制圆形*/
	void drawCircleByEquation(vector<Circle> haveVerifiedCircleSet, CImg<short int> * houghImage);

	/*画出图像圆的边缘*/
	void drawCircleEdge(vector<Point> edgePointSet, vector<Circle> haveVerifiedCircleSet, CImg<short int>* houghImage);

	/*汇总步骤功能函数*/
	void hough(string fileName);

};


#endif // !_HOUGH_FOR_CIRCLE_H_
