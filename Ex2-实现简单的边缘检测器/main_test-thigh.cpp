#include "Canny.h"
#include <iostream>
#include <string>


using namespace std;

int main(int argc, char *argv[]) {
	
	/* 测试 高阈值对图像的影响，变化范围从 40 到 200*/
	Canny c;
	float tlow = 30;
	float thigh = 40;
	for (int i = 0; i < 8; i++) {
		c.setTLowandTHigh(thigh, tlow);
		c.canny("lena.bmp");
		thigh += 20;
	}
	system("pause");
}