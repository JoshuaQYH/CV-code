#include "Canny.h"
#include <iostream>
#include <string>


using namespace std;

int main(int argc, char *argv[]) {
	
	/* 测试低阈值对图像的影响，变化范围从 5 到 35*/
	Canny c;
	float tlow = 5;
	float thigh = 40;
	for (int i = 0; i < 7; i++) {
		c.setTLowandTHigh(thigh, tlow);
		c.canny("lena.bmp");
		tlow += 5;
	}
	system("pause");
}