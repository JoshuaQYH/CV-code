#include "Canny.h"
#include <iostream>
#include <string>


using namespace std;

int main(int argc, char *argv[]) {
	
	/* 测试 模糊因子对图像的影响，变化范围从 0.2 到 2*/
	Canny c;
	float blurFactor = 0.2;
	for (int i = 0; i < 9; i++) {
		c.setBoost(blurFactor);
		c.canny("lena.bmp");
		blurFactor += 0.2;
	}

	system("pause");
}