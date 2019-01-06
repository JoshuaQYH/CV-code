#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "CImg.h"

using namespace std;
using namespace cimg_library;

class DigitSpilt
{
	CImg<double> originImage;
	CImg<double> adaptiveImage;
	int border = 20;
public:

	// 自适应阈值分割
	CImg<double> adaptiveThreshold(CImg<double> a4Paper);

	//对断裂字符进行修复和扩张
	CImg<double> dilation(CImg<double> image);

	// 对分割图像进行垂直直方图投影
	CImg<double> hor_projection(CImg<double> digitImage);

	// 获取单行图像
	CImg<double> lineImage(int y1, int y2, CImg<double> digitImage);

	// 对垂直分割后的图像进行竖直直方图投影
	CImg<double> ver_projection(CImg<double> digitImage);

	// 对水平直方图进行划分，得到多行数字的上下界
	vector<int> hor_partion(CImg<double> histogram);

	// 计算竖直直方图的划分线，划分单个字符
	vector<int> ver_partion(CImg<double> histogram);

	//对单行图像进行连通分量标记 分割字符，返回字符的最左x和最右x的列表。
	vector<int> areaLabel(CImg<double> image);

	// 根据竖线，切分每一行图像
	vector<CImg<double>> cutImage(CImg<double> horImage, vector<int> horLineSet);
};

