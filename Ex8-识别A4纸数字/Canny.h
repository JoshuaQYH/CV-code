#pragma once
/*****************************************************************
Author: qiuyh
Date: 2018/10/12
Purpose: 实现 canny-edge-detector
*******************************************************************/

#ifndef CANNY_H
#define CANNY_H
#include <iostream>
#include <cstdlib>
#include <cmath>
#include "CImg.h"
#include <string>
using namespace std;
using namespace cimg_library;
#define PI 3.141592
#define POSSIBLEEDGE 255
#define NOTEDGE 0


class Canny {
private:
	string inputFileName;                    /*源图片文件的名称*/
	char output[100];                        /*检测后待保存的图片名称*/

	float sigma;                             /*高斯滤波标准差*/
	float tLow;	                             /*确定低阈值的参数*/
	float tHigh;                             /*确定高阈值的参数*/

	int highthreshold;	                     /*高阈值*/
	int lowthreshold;                        /*低阈值*/

	float  BOOSTBLURFACTOR;                  // 增加模糊因子
	int width;                               /*所有图像的宽*/
	int height;                              /*所有图像的高*/

	CImg<short int> SrcImage;                /*初始图像*/
	CImg<short int> grayImage;               /*初始灰度图对象*/
	CImg<short int> smoothImage;		     /*高斯平滑后图像对象*/
	CImg<short int> deltaXImage;             /*x方向偏导图像对象*/
	CImg<short int> deltaYImage;             /*y方向偏导图像对象*/
	CImg<short int> dirRadianImage;          /*梯度方向图像对象*/
	CImg<short int> magnitudeImage;          /*梯度幅值图像对象*/
	CImg<short int> nonMaxSuppImage;         /*非最大化抑制后的图像对象*/
	CImg<short int> resultEdgeImage;         /*边缘检测后的图像对象*/
	CImg<short int> cutShortEdgeImage;       /*删除短边缘后的图像*/
	bool DEBUG;                              /*若为真，处于debug模式，打印信息*/
	int number;                              /*一次canny后结果文件的序号*/
										     // 定义一个坐标点
	struct XY {
		int col;
		int row;
	};

public:
	// 初始化类，主要是检测的参数
	Canny() {
		DEBUG = false;
		sigma = 1;   // sig  1.0 ~ 1.4
		number = 1;
		tLow = 10;   // 6 7
		tHigh = 60;
		BOOSTBLURFACTOR = 0.2485;  // 0.28 // sobel 算子 6 // 0.35
		//所有图对象进行初始化
	}
	
	// n为真打印信息
	void ForDebug(bool n) {
		DEBUG = n;  
	}
	
	// 修改标准差
	void setSigma(float n) {
		sigma = n;
	}
	// 修改高低阈值
	void setTLowandTHigh(float thigh, float tlow) {
		tHigh = thigh;
		tLow = tlow;
	}
	// 修改高斯模糊因子
	void setBoost(float h) {
		BOOSTBLURFACTOR = h;
	}

	/*读取pgm灰度图,本实验读取一个彩色的BMP图像inputFileName然后再转换为灰度图pgm格式的image*/
	int readGrayImage(string inputFileName);

	/*写入bmp灰度图，将检测边缘后的结果输入到输出图像中，显示*/
	int writeGrayImage(string outputFileName, CImg<short int> image);

	/*高斯平滑，对灰度图进行降噪，去除高频的噪声，此处使用可分离的高斯滤波器，先行求卷积，再列求卷积*/
	void gaussianSmooth(CImg<short int>* image,float sigma, CImg<short int>* smootheDim);

	/*算出高斯核*/
	void makeGaussianKernel(float sigma, float **kernel, int *windowsize);

	/*计算xy方向的导数， 求出x, y方向的偏导数图*/
	void derrivativeXY(CImg<short int> *smootheDim, 
		CImg<short int> * delta_x, CImg<short int>  * delta_y);

	/*计算 梯度方向的水平夹角*/
	double angle_radians(double x, double y);

	/* 计算梯度方向，求出梯度方向图*/
	void radianDirection(CImg<short int> *delta_x,CImg<short int>* delta_y,
		CImg<short int>* dir_radians, int xdirtag, int ydirtag);

	/*计算xy梯度幅值*/
	void magnitudeXY(CImg<short int>* delta_x, 
		CImg<short int>* delta_y, CImg<short int>* magnitude);

	/* 非最大化抑制*/
	void nonMaxSuppression(CImg<short int>* magnitude,
		CImg<short int>* delta_x, CImg<short int>* delta_y, CImg<short int> *nonMaxSupp);
	
	/*滞后阈值法*/
	void applyHysteresis(CImg<short int> *magnitude, CImg<short int> *nonMaxSupp,
		float tlow, float thigh, CImg<short int> *edge);

	/*边缘连接*/ 
	void cutShortEdge(CImg<double> *edge, CImg<double>* resultEdgeImage, int cutLen);

	/*汇总函数*/
	CImg<short int> canny(string inputFileName);
	
};


#endif