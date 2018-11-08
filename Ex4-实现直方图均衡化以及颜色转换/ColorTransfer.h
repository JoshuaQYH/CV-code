#ifndef COLOR_TRANSFER_H
#define COLOR_TRANSFER_H
#include <iostream>
#include <vector>
#include <string>
#include "CImg.h"
using namespace std;
using namespace cimg_library;

/*************************************************************

	Author: qiuyh

	Contact: 576261090@qq.com

	Date: 18/11/6

	Description: Histogram equalization on gray and color image.

**************************************************************/



class ColorTransfer
{
	int number;  // 图像文件序号
	int show;   // 为1 显示图像
public:
	ColorTransfer() {
		number = 0;
		show = 1;
	}
	// RGB  CImg对象转lab空间
	CImg<double> RGB2LAB(CImg<unsigned int> rgbImage);
	// 传入转换图和色彩传递图
	CImg<double> colorTransferOnLAB(CImg<double> sourceImage, CImg<double> targetImage);
	// lab 2 lab
	CImg<unsigned int> LAB2RGB(CImg<double> labImage);
	// 颜色传递接口函数
	CImg<unsigned int> colorTransfer(string sourceImageFileName, string targetImageFileName);
};

#endif