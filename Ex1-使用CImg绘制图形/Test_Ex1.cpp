#include "CImg.h"
#include <iostream>
#include<math.h>
#include<string>
#include<vector>
#define cimg_display_type 0;
#define PI 3.1415926535

using namespace std;
using namespace cimg_library;

/**************************************************************************
*	Author: qiuyihao
*	Date:2018/09/14
*	Task: Test the drawing algorithm at the first homework of CV
*   method: 在drawBmpTest类中，封装了四个绘图函数，分别是各用两种方式
*          （调用CImg函数，以及自己实现的方法）来测试画圆以及画直线的效果
*           然后分别对比两种算法之间的优劣和区别，试探究CImg 内部的实现方式
***************************************************************************/


class drawBmpTest {
private:
	CImg<unsigned char> SrcImg;     //用于保存所要操作图像的对象
	int width;						//保存图片的宽度
	int height;						//保存图片的高度
public: 
	drawBmpTest() {
		reset();  // 初始化或者重置
	}

	// 利用draw_circle函数画圆, 
	// 需要指明参数列表圆心位置(xpos,ypos)，以及半径r, 图形颜色
	void drawCircle_withCImgFunc(int xpos, int ypos, int r, const char* color) {
		// 判断坐标合法性
		if ((xpos + r > width) || (xpos - r < 0) || (ypos + r) > height || (ypos - r < 0)) {
			cout << "坐标不合法" << endl;
			return;
		}
		this->SrcImg.draw_circle(xpos, ypos, r, color, 1);
	//	this->SrcImg.display("draw circle with draw_circle function");
	}

	// 利用欧氏距离画圆
	// 参数列表圆心位置（xpos,ypos) 以及半径r, color
	void drawCircle_noCImgFunc(int xpos, int ypos, int r, const char* color) {
		// 判断坐标合法性
		if ((xpos + r > width) || (xpos - r < 0) || (ypos + r) > height || (ypos - r < 0)) {
			cout << "坐标不合法" << endl;
			return;
		}
		cimg_forXY(SrcImg, x, y) {
			// 判断坐标距离，小于30的，颜色改变
			if ((abs(x - xpos)*abs(x - xpos) + abs(y - ypos)* abs(y - ypos)) <= r*r) {
				SrcImg(x, y, 0) = color[0];
				SrcImg(x, y, 1) = color[1];
				SrcImg(x, y, 2) = color[2];
			}
		}
		//SrcImg.display("draw circle not CImg Function.");
	}

	//利用draw_line来画直线
	// 参数列表为起点坐标(startX,startY)，终点坐标(endX, endY)，color
	void drawLine_withCImgFunc(int startX, int startY, int endX,int endY, const char* color) {
		//判断坐标合法性
		if ((startX > width) || (startX < 0) || (startY > height) || (startY < 0)) {
			cout << "坐标不合法" << endl;
			return;
		}
		if ((endX > width) || (endX < 0) || (endY > height) || (endY < 0)) {
			cout << "坐标不合法" << endl;
			return;
		}
		unsigned char blue[] = { 0, 0, 255 };
		SrcImg.draw_line(startX, startY, endX, endY, color);
		//SrcImg.display("draw line with CImgfunction draw_line");
	}

	// 利用DDA算法 y = ax + b 利用x,y方向的增量来计算
	// 参数列表为起点坐标(startX,startY)，终点坐标(endX, endY) color 
	void drawLine_noCImgFunc(int startX, int startY, int endX, int endY, const char* color) {
		//判断坐标合法性
		if ((startX > width) || (startX < 0) || (startY > height) || (startY < 0) || (endX > width) || (endX < 0) || (endY > height) || (endY < 0)) {
			cout << "坐标不合法" << endl;
			return;
		}
		float x, y, xincre, yincre; // x y  坐标以及 x，y增量
		int k = abs(endX - startX);
		if (abs(endY - startY) > k) {
			k = abs(endY - startY);
		}
		//计算x,y增量
		xincre = (float)(endX - startX) / k;
		yincre = (float)(endY - startY) / k;
		// 起始横纵坐标
		x = startX;
		y = startY;
		// 扫描
		for (int i = 1; i < k; i++) {
			SrcImg(x, y, 0) = color[0];
			SrcImg(x, y, 1) = color[1];
			SrcImg(x, y, 2) = color[2];
			x += xincre;
			y += yincre;
		}
	}

	// 用于打印窗口标题
	void display(string str) {
		SrcImg.display(str.c_str());
	}
	// 重新加载图片资源
	void reset() {
		try
		{
			SrcImg.load("1.bmp");
			width = SrcImg._width;
			height = SrcImg._height;
			cimg_forXY(SrcImg, x, y) {
				SrcImg(x, y, 0) = 0;
				SrcImg(x, y, 1) = 0;
				SrcImg(x, y, 2) = 0;
			}

		}
		catch (const std::exception&)
		{
			cout << "当前路径不存在图片 1.bmp" << endl;
		}
	}

};


int stringToInt(string num) {
	if (num == "1") return 1;
	if (num == "2") return 2;
	if (num == "3") return 3;
	if (num == "4") return 4;
	if (num == "5") return 5;
	if (num == "6") return 6;
	if (num == "7") return 7;
}

int main() {
	drawBmpTest testCase;
	vector<char*> colorSet;
	char yellow[] = {255, 255, 0};
	char blue[] = { 0, 0, 255};
	char red[] = { 255, 0, 0};
	char green[] = { 0,255, 0 };
	char white[] = { 255,255,255 };
	colorSet.push_back(yellow);
	colorSet.push_back(blue);
	colorSet.push_back(red);
	colorSet.push_back(green);
	colorSet.push_back(white);

	string choose;
	cout << "请输入相应的数字执行相应操作" << endl;
	cout << "1. CImg中draw_circle函数自身对比,改变半径大小进行比较" << endl;
	cout << "2. 自己实现的draw_circle_noCImgFunc函数自身对比，改变半径大小进行比较" << endl;
	cout << "3. CImg中draw_line函数自身对比,改变长度和偏移角度进行比较" << endl;
	cout << "4. 自己实现的draw_line_noCimgFunc函数自身对比,改变长度和偏移角度进行比较" << endl;
	cout << "5. CImg和自己实现的draw_circle函数进行对比" << endl;
	cout << "6. CImg和自己实现的draw_line函数进行对比" << endl;
	cout << "7. 结束" << endl;
	int num = 0;
	while (1) {
		cin >> choose;
		switch (stringToInt(choose))
		{
			// 测试样例
		case 1:
			for (int i = 1; i < 21; i++) {
				testCase.drawCircle_withCImgFunc(200, 200, 105 - i * 5,colorSet.at(i%5) );
			}
			testCase.display("draw circle with cimg function");
			testCase.reset();
			break;
		case 2:
			for (int i = 1; i < 21; i++) {
				testCase.drawCircle_noCImgFunc(200, 200, 105 - i * 5, colorSet.at(i % 5));
			}
			testCase.display("draw circle not with cimg function");
			testCase.reset();
			break;
		case 3:
			for (int i = 1; i < 10; i++) {
				for (int j = 1; j < 10; j++) {
					//cout << ++num << endl;
					testCase.drawLine_withCImgFunc(25, 25, i * 25,j * 25, colorSet.at(i % 5));
				}
			}
			testCase.display("draw line with cimg funciton");
			testCase.reset();
			break;
		case 4:
			for (int i = 1; i < 10; i++) {
				for (int j = 1; j < 10; j++) {
					//cout << ++num << endl;
					testCase.drawLine_noCImgFunc(25, 25, i * 25, j * 25, colorSet.at(i % 5));
				}
			}
			testCase.display("draw line not with cimg funciton");
			testCase.reset();
			break;
		case 5:
			for (int i = 1; i <= 20; i++) {
				testCase.drawCircle_withCImgFunc(100, 100, 110 - i * 5, colorSet.at(i % 5));
			}
			for (int i = 1; i <= 20; i++) {
				testCase.drawCircle_noCImgFunc(300, 300, 110 - i * 5, colorSet.at(i % 5));
			}
			testCase.display("comparision of drawing circles");
			testCase.reset();
			break;
		case 6:
			for (int i = 1; i <= 20; i++) {
				testCase.drawLine_withCImgFunc(200, 200, 50,i * 20, colorSet.at(i % 5));
			}
			for (int i = 1; i <= 20; i++) {
				testCase.drawLine_noCImgFunc(200, 200, 350, i * 20, colorSet.at(i % 5));
			}
			testCase.display("comparision of drawing line");
			testCase.reset();
			break;
		case 7:
			exit(0);
			break;
		default:
			break;
		}
	}
	
}