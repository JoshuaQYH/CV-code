#include "CImg.h"
#include <iostream>
#include<math.h>
#include<string>

/**********************************
*	Author: qiuyihao
*	Date:2018/09/11
*	Task: The first homework of CV 
**********************************/

#define cimg_display_type 0;
#define PI 3.1415926535

using namespace std;
using namespace cimg_library;

/****************************************************************************
* 将作业中的每一个步骤操作进行了封装，
* 而且在绘图函数方面使用CImg库函数以及像素遍历操作实现
* 每一个步骤的方法采用前缀step + n指明题号n
* 第3，4，5步有两种方法实现，后缀有区分withCImgFunc和noCImgFunc
* 然后使用 exe_hw_withCImgFunction和exe_hw_noCImgFunction()自动执行2~6个步骤。
*****************************************************************************/

class drawBmp {
private:
	CImg<unsigned char> SrcImg;     //用于保存所要操作图像的对象
	string textOnBmp;               //图片上的字样，用于区分有无调用CImg函数
	string savedFileName;           //指明保存图片的文件名
	string initialFileName;         //初始化时图片的文件名
public:
	//自定义构造函数，依靠文件名来指定操作的图片，同时display
	drawBmp(string name) {
		try
		{
			initialFileName = name;
			savedFileName = "2";
			SrcImg.load_bmp(name.c_str());
			//step1 读取并显示图像
			SrcImg.display("original image");
		}
		catch (const std::exception&)
		{
			cout << "当前路径没有该图片" << endl;
		}
	}
	
	// （调用绘图函数）自动执行第二，三，四，五步骤
	void exe_hw_withCImgFunction() {
		reset();
		this->textOnBmp = "Call drawing function";
		this->savedFileName = "2_withCImgFunction.bmp";
		step2_changedColor();
		step3_drawBlueCircle_withCImgFunc();
		step4_drawYellowCircle_withCImgFunc();
		step5_drawBlueLine_withCImgFunc();
		step6_saveBmp();
	}

	//（不调用绘图函数） 自动执行第二，三，四，五，步骤
	void exe_hw_noCImgFunction() {
		reset();
		this->textOnBmp = "No call drawing function";
		this->savedFileName = "2_noCImgFunction.bmp";
		step2_changedColor();
		step3_drawBlueCircle_noCImgFunc();
		step4_drawYellowCircle_noCImgFunc();
		step5_drawBlueLine_noCImgFunc();
		step6_saveBmp();
	}

	// 重新加载原始图片
	void reset() {
		SrcImg.load_bmp(initialFileName.c_str());
	}

	// 白色区域变为红色 黑色区域变为绿色
	void step2_changedColor() {
		//step1 得到图像的长度宽度
		int width = SrcImg._width;
		int height = SrcImg._height;
		cout << "The width of the image is " << width << endl;
		cout << "The height of the image is " << height << endl;

		//step2 扫描图像，判断颜色
		//白色区域变为红色，黑色区域变为绿色
		// x，y 代表图像坐标，坐标原点位于左上角，横向为x轴，纵向为y轴
		cimg_forXY(SrcImg, x, y) {
			// 白色区域变为红色
			if (SrcImg(x, y, 0) == 255 && SrcImg(x, y, 1) == 255 && SrcImg(x, y, 2) == 255) {
				SrcImg(x, y, 0) = 255;
				SrcImg(x, y, 1) = SrcImg(x, y, 2) = 0;
			}
			// 黑色区域变为绿色
			else if (SrcImg(x, y, 0) == 0 && SrcImg(x, y, 1) == 0 && SrcImg(x, y, 2) == 0) {
				SrcImg(x, y, 0) = 0;
				SrcImg(x, y, 1) = 255;
				SrcImg(x, y, 2) = 0;
			}
		}
		// 显示变色结果
		SrcImg.display("Changed color from image");
	}

	// 画蓝色的圆，参数列表为 圆心坐标x=50，y=50，半径=30，颜色，透明度
	void step3_drawBlueCircle_withCImgFunc() {
		unsigned char blue[] = { 0, 0, 255 };
		this->SrcImg.draw_circle(50, 50, 30, blue, 1);
		this->SrcImg.display("draw blue circle");
	}

	// 画蓝色的圆，参数列表为 圆心坐标x=50，y=50，半径=30，颜色黄，透明度
	void step3_drawBlueCircle_noCImgFunc() {
		unsigned char blue[] = { 0, 0, 255 };
		cimg_forXY(SrcImg, x, y) {
			// 判断坐标距离，小于30的，颜色变为蓝色
			if ((abs(x - 50)*abs(x - 50) + abs(y - 50)* abs(y - 50)) <= 900) {
				SrcImg(x, y, 0) = SrcImg(x, y, 1) = 0;
				SrcImg(x, y, 2) = 255;
			}
		}
		SrcImg.display("draw blue circle not CImg Function.");
	}

	// 画黄色的圆,参数列表为 圆心坐标x=50，y=50，半径=3，颜色黄，透明度
	void step4_drawYellowCircle_withCImgFunc() {
		unsigned char yellow[] = { 255, 255, 0 };
		SrcImg.draw_circle(50, 50, 3, yellow, 1);
		SrcImg.display("draw yellow circle");
	}

	// 画黄色的圆,参数列表为 圆心坐标x=50，y=50，半径=3，颜色黄，透明度
	void step4_drawYellowCircle_noCImgFunc() {
		unsigned char yellow[] = { 255, 255, 0 };
		cimg_forXY(SrcImg, x, y) {
			// 判断坐标距离，小于30的，颜色变为黄色
			if ((abs(x - 50)*abs(x - 50) + abs(y - 50)* abs(y - 50)) <= 9) {
				SrcImg(x, y, 0) = SrcImg(x, y, 1) = 255;
				SrcImg(x, y, 2) = 0;
			}
		}
		SrcImg.display("draw yellow circle not CImg Function");
	}

	//在图上绘制一条长为100 的直线段，起点坐标为(0, 0)，方向角为35 度，直线的颜色为蓝色。
	void step5_drawBlueLine_withCImgFunc() {
		//利用三角函数计算终点坐标
		int x0 = 100 * cos(35.0 * PI / 180);
		int y0 = 100 * sin(35.0 * PI / 180);
		//cout << x0 << "==========================" << y0 << endl;
		// 为了直线不被挡到，resetSrcImg。。
		reset();
		unsigned char blue[] = { 0, 0, 255 };
		SrcImg.draw_line(0, 0, x0, y0, blue);
		SrcImg.display("draw blue line");
	}

	//在图上绘制一条长为100 的直线段，起点坐标为(0, 0)，方向角为35 度，直线的颜色为蓝色。
	void step5_drawBlueLine_noCImgFunc() {
		// 为了直线不被挡到，resetSrcImg。。
		reset();
		//利用三角函数计算终点坐标
		unsigned char blue[] = { 0, 0, 255 };
		double k = tan(35.0 * PI / 180); //计算斜率
		int y = 0; //纵坐标
		// 采用DDA算法 参考链接 http://www.cnblogs.com/flying-roc/articles/1913385.html
		// 当斜率小于1时，步进方向为x轴，增量方向为y轴
		cimg_forX(SrcImg, x) {
			if (x < (100 * cos(35.0*PI / 180))) {
				y = (int)(k * 1.0 * x);
				SrcImg(x, y, 0) = SrcImg(x,y,1) = 0;
				SrcImg(x, y, 2) = 255;
			}
			
		}
		SrcImg.display("draw blue line not with  CImg fucntion");
	}

	// 保存图片（调用CImg函数）
	void step6_saveBmp() {
		// 放置文字
		unsigned char white[] = { 255,255,255 };
		// 在坐标(100, 100)处画一个白色文字
		SrcImg.draw_text(SrcImg._width / 2, 10, textOnBmp.c_str(), 2, white);
		SrcImg.save(savedFileName.c_str());
		cout << "The picture is saved as bmp. " << endl;
	}
};

int StringToInt(string num) {
	if (num == "1") return 1;
	if (num == "2") return 2;
	if (num == "3") return 3;
}




int main() {
	drawBmp bmp("1.bmp");
	cout << "图片已加载和显示" << endl;
	string num;
	cout << "请输入相应的数字执行相应操作" << endl;
	cout << "1. 自动执行任务2~6，其中3~5调用Cimg函数" << endl;
	cout << "2. 自动执行任务2~6，其中3~5使用遍历像素改变颜色实现" << endl;
	cout << "3. 结束" << endl;
	while (1) {
	cin >> num;
	int Integer = StringToInt(num);
	switch (Integer)
	{
		case 1:
			bmp.exe_hw_withCImgFunction();
		break;
		case 2:
			bmp.exe_hw_noCImgFunction();
		break;
		case 3:
			exit(0);
		default:
		break;
	}
	}
	return 0;
}