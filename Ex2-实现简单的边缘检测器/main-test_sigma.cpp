#include "Canny.h"
#include <iostream>
#include <string>


using namespace std;

int main(int argc, char *argv[]) {


	/*1. 测试高斯半径的影响*/
	/*仅产生 高斯模糊图，梯度幅值图，非最大化抑制图，阈值处理图，剔除短边缘图*/
	Canny c;
	float sigma = 0.5;   // 高斯半径从0.5 到 5
	
	for (int i = 0; i < 10; i++) {
		c.setSigma(sigma);
		c.canny("lena.bmp");
		sigma += 0.5;
	}
	system("pause");
}