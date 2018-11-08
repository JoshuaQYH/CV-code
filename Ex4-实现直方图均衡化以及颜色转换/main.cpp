#include "HistogramEqualization.h"
#include "CImg.h"
#include <iostream>
#include "ColorTransfer.h"

using namespace std;

int main() {
	
	HistogramEqualization h;
	h.HistogramEqualizationOnGrayImage("equalize2.bmp");
	h.Hist_Equal_ColorImage_OneColorChannel("equalize2.bmp");
	h.Hist_Equal_ColorImage_ThreeColorChannels("equalize2.bmp");
	h.Hist_Equal_ColorImage_HSISpace("equalize2.bmp");
	
	ColorTransfer ct;
	ct.colorTransfer("5-source.bmp", "5-target.bmp");


}