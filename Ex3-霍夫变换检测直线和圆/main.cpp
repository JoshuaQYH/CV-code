#include <iostream>
#include "CImg.h"
#include "Canny.h"
#include "HoughForLine.h"
#include "HoughForCircle.h"

using namespace std;

int main() {
	
    HoughForCircle hc;
	hc.setDebug(true);
	hc.hough("B1.bmp");
	
	HoughForLine hl1;
	//hl1.setBoost(3);
	//hl1.sethighAndlow(100, 110);
	//hl1.hough("A1.bmp");
	HoughForLine hl1;
	hl1.sethighAndlow(20, 30);
	hl1.hough("A1.bmp");
	
	/*
	HoughForLine hl2;
	hl2.sethighAndlow(20, 30);
	hl2.hough("A2.bmp");

	HoughForLine hl3;
	hl3.sethighAndlow(80, 85);
	hl3.hough("A3.bmp");

	HoughForLine hl4;
	hl4.sethighAndlow(30, 50);
	hl4.hough("A4.bmp");

	HoughForLine hl5;
	hl5.sethighAndlow(90, 100);
	hl5.hough("A5.bmp");

	HoughForLine hl6;
	hl6.sethighAndlow(95, 100);
	hl6.hough("A6.bmp");
	*/



	system("pause");
}