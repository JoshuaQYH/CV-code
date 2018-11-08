#ifndef  HISTOGRAM_EQUALIZATION_H
#define  HISTOGRAM_EQUALIZATION_H

#include <iostream>
#include <vector>
#include <string>
#include "CImg.h"
using namespace std;
using namespace cimg_library;

/*************************************************************

	Author: qiuyh

	Contact: 576261090@qq.com

	Date: 18/11/5

	Description: Histogram equalization on gray and color image.

**************************************************************/


class HistogramEqualization
{
	int number; // 文件编号
	int show;   // 为1显示图片  
public:
	HistogramEqualization() {
		number = 0;
		show = 1;
	}

	/*画出图像的灰度直方图*/
	CImg<unsigned int> DrawHistogram(CImg<unsigned int> image);

	/*彩色图转为灰度图*/
	CImg<unsigned int> Rgb2Gray(CImg<unsigned int> RGBImage);

	/*直方图各灰度值像素数量计算*/
	CImg<unsigned int> GetHistogram(CImg<unsigned int> InputImage);

	/*直方图均衡化*/
	CImg<unsigned int> HistogramEqualizationMethod(CImg<unsigned int> InputImage, CImg<unsigned int>  Histogram);

	/*灰度图均衡化接口函数*/
	CImg<unsigned int> HistogramEqualizationOnGrayImage(string ImageFileName);

	/*彩色图均衡化-基于单颜色通道的方法*/
	CImg<unsigned int> Hist_Equal_ColorImage_OneColorChannel(string ImageFileName);

	/* 计算得到三个通道的平均直方图*/
	CImg<unsigned int> GetAverageHistogram(CImg<unsigned int> img1, CImg<unsigned int> img2, CImg<unsigned int> img3);

	/*彩色图均衡化-基于三个颜色通道平均灰度直方图的方法*/
	CImg<unsigned int> Hist_Equal_ColorImage_ThreeColorChannels(string ImageFileName);

	/*彩色图均衡化-基于HSI空间下的直方图均衡*/
	CImg<double> Hist_Equal_ColorImage_HSISpace(string ImageFileName);
};
#endif // ! HISTOGRAM_EQUALIZATION_H

