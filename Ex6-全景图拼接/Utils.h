#pragma once
/*************************************************************************

Author: qiuyh

Creating Date: 18/11/27

Description: Implement some global functions and variable.

***************************************************************************/
#include <iostream>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <iomanip>
#include "CImg.h"


using namespace std;
using namespace cimg_library;

#define DEBUG 1


// 存储单应矩阵
struct HomographyMatrix {
	float a11, a12, a13, a21, a22, a23, a31, a32;
	HomographyMatrix(float _a11, float _a12, float _a13, float _a21, 
		float _a22, float _a23, float _a31, float _a32):
		a11(_a11), a12(_a12), a13(_a13), a21(_a21), a22(_a22),
		a23(_a23),a31(_a31), a32(_a32){}

	void print() {
		cout <<setw(18) << a11 << setw(18) << a12 << setw(18) << a13 << endl;
		cout << setw(18) <<  a21 << setw(18) << a22 << setw(18) << a23 << endl;
		cout <<setw(18) <<  a31 << setw(18) << a32 << setw(18) << 1 << endl;
	}
};


// 传入图片文件夹路径，读取所有要拼接图片的路径，返回一个存放路径的向量。
vector<string>& ReadImageFileName(string imageFileFolderPath, vector<string>& imageFilePathVector);

void getAllFiles(string path, vector<string> &files);

//  计算随机值
int random(int min, int max);

// 计算单应矩阵变形后的x坐标
float get_warped_x(float x, float y, HomographyMatrix H);

// 计算单应矩阵变形后y的坐标
float get_warped_y(float x, float y, HomographyMatrix H);

// 转化为球坐标
CImg<float> cylinderProjection(const CImg<float> &src);