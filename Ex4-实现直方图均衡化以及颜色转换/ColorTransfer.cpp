#include "ColorTransfer.h"


CImg<double> ColorTransfer::RGB2LAB(CImg<unsigned int> rgbImage)
{
	int w = rgbImage._width;
	int h = rgbImage._height;
	CImg<double> labImage(w, h, 1, 3, 0);
	cimg_forXY(rgbImage, x, y) {
		// 归一化
		double r = max(1.0 * rgbImage(x, y, 0) / 255, 1.0 / 255);
		double g = max(1.0 * rgbImage(x, y, 1) / 255, 1.0 / 255);
		double b = max(1.0 *rgbImage(x, y, 2) / 255, 1.0 / 255);
		
		// rgb -> lms
		double L = 0.3811 * r + 0.5783 * g + 0.0402 * b;
		double M = 0.1967 * r + 0.7244 * g + 0.0782 * b;
		double S = 0.0241 * r + 0.1288 * g + 0.8444 * b;
		L = log10(L);
		M = log10(M);
		S = log10(S);
		// lms -> l alpha beta
		double l = 0, alpha = 0, beta = 0;
		double tmp_L = L + M + S;
		double tmp_M = L + M - 2 * S;
		double tmp_S = L - M;
		l = tmp_L * sqrt(3) / 3.0;
		alpha = tmp_M * sqrt(6) / 6.0;
		beta = tmp_S * sqrt(2) / 2.0;

		labImage(x, y, 0) = l;
		labImage(x, y, 1) = alpha;
		labImage(x, y, 2) = beta;
	}
	if (show) labImage.display("rgb2lab");
	return labImage;
}

CImg<double> ColorTransfer::colorTransferOnLAB(CImg<double> sourceImage, CImg<double> targetImage)
{
	int w = sourceImage._width;
	int h = sourceImage._height;

	CImg<double> meanOfSource(sourceImage._spectrum, 1, 1, 1, 0);  // width 3 height 1 depth 1， initValue 0
	CImg<double> stdDeviationOfSource(sourceImage._spectrum, 1, 1, 1, 0);
	CImg<double> meanOfTarget(targetImage._spectrum, 1, 1, 1, 0);
	CImg<double> stdDeviationOfTarget(targetImage._spectrum, 1, 1, 1, 0);

	CImg<double> resultImage(w, h, 1, 3, 0);

	unsigned long numOfPixels = w * h;
	// for mean of source 三重循环，宽，高，通道
	cimg_forXYC(sourceImage, x, y, c) {
		meanOfSource(c) += sourceImage(x, y, c);  // 求和
	}
	cimg_forX(meanOfSource, c) {
		meanOfSource(c) /= numOfPixels;
	}
	// for std deviation of source
	cimg_forXYC(sourceImage, x, y, c) {
		stdDeviationOfSource(c) += (sourceImage(x, y, c) - meanOfSource(c)) * (sourceImage(x, y, c) - meanOfSource(c));
	}
	cimg_forX(stdDeviationOfSource, c) {
		stdDeviationOfSource(c) = sqrt(stdDeviationOfSource(c) / numOfPixels);
	}

	// for mean of target 三重循环，宽，高，通道
	numOfPixels = targetImage._width * targetImage._height;
	cimg_forXYC(targetImage, x, y, c) {
		meanOfTarget(c) += targetImage(x, y, c);  // 求和
	}
	cimg_forX(meanOfTarget, c) {
		meanOfTarget(c) /= numOfPixels;
	}
	// for std deviation of source
	cimg_forXYC(targetImage, x, y, c) {
		stdDeviationOfTarget(c) += (targetImage(x, y, c) - meanOfTarget(c)) * (targetImage(x, y, c) - meanOfTarget(c));
	}
	cimg_forX(stdDeviationOfTarget, c) {
		stdDeviationOfTarget(c) = sqrt(stdDeviationOfTarget(c) / numOfPixels);
	}

	cimg_forXYC(resultImage, x, y, c) {
		resultImage(x, y, c) = sourceImage(x, y, c) - meanOfSource(c); // 原图通道值/通道值均值 = tmp
		resultImage(x, y, c) = resultImage(x, y, c) *   // （目标图通道标准差/原图通道标准差）* tmp + 目标图通道均值
			stdDeviationOfTarget(c) / stdDeviationOfSource(c) + meanOfTarget(c);
	}
	if (show) resultImage.display("lab transfer");
	return resultImage;
}


CImg<unsigned int> ColorTransfer::LAB2RGB(CImg<double> labImage)
{
	int w = labImage._width;
	int h = labImage._height;

	CImg<unsigned int> rgbImage(w, h, 1, 3, 0);

	cimg_forXY(labImage, x, y) {
		 //rgb -》 lab 逆过程
		double l = labImage(x, y, 0);
		double alpha = labImage(x, y, 1);
		double beta = labImage(x, y, 2);
		// lab -> lms
		double L = (sqrt(3.0) / 3.0) * l + (sqrt(6.0) / 6.0) * alpha + (sqrt(2.0) / 2.0) * beta;
		double M = (sqrt(3.0) / 3.0) * l + (sqrt(6.0) / 6.0) * alpha -(sqrt(2.0) / 2.0) * beta;
		double S = (sqrt(3.0) / 3.0) * l - (2* sqrt(6.0) / 6.0) * alpha;
		L = pow(10.0, L);
		M = pow(10.0, M);
		S = pow(10.0, S);
		// lms -> rgb
		double R = 4.4679 * L - 3.5873 * M + 0.1193 * S;
		double G = -1.2186 * L + 2.3809 * M - 0.1624 * S;
		double B = 0.0497 * L - 0.2439 * M + 1.2045 * S;
		// 映射到0 - 255
		rgbImage(x, y, 0) = max(min(R * 255, 255.0), 0.0);
		rgbImage(x, y, 1) = max(min(G * 255, 255.0), 0.0);
		rgbImage(x, y, 2) = max(min(B * 255, 255.0), 0.0);
	}
	
	return rgbImage;
}

CImg<unsigned int> ColorTransfer::colorTransfer(string sourceImageFileName, string targetImageFileName)
{
	CImg<unsigned int> rgb_source;
	CImg<unsigned int> rgb_target;
	rgb_source.load_bmp(sourceImageFileName.c_str());
	rgb_target.load_bmp(targetImageFileName.c_str());
	if (rgb_source._spectrum != 3) {
		cout << sourceImageFileName << "不是rgb图，不能转换" << endl;
	}
	if (rgb_target._spectrum != 3) {
		cout << targetImageFileName << "不是rgb图，不能转换" << endl;
	}
	if(show) rgb_source.display("source");
	if (show)rgb_target.display("target");
	CImg<unsigned int> result = LAB2RGB(colorTransferOnLAB(RGB2LAB(rgb_source),RGB2LAB(rgb_target)));
	if(show) result.display("transfered result");
	char FileName[100] = {};
	sprintf_s(FileName, "%d_colorTransfer_%s", number++, sourceImageFileName.c_str());
	result.save(FileName);
	return result;
}
