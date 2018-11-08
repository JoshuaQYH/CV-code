#include "HistogramEqualization.h"
// 功能：彩色图像灰度化
// 输入：彩色bmp图像
// 输出：灰度bmp图像
CImg<unsigned int> HistogramEqualization::Rgb2Gray(CImg<unsigned int> RGBImage){
	CImg<unsigned int> GrayImage(RGBImage._width, RGBImage._height, 1, 1, 0);
	cimg_forXY(RGBImage, x, y) {
		int r = RGBImage(x, y, 0);
		int g = RGBImage(x, y, 1);
		int b = RGBImage(x, y, 2);
		GrayImage(x, y) = 0.299 * r + 0.587 * g + 0.114 * b;
	}
	return GrayImage;
}

CImg<unsigned int> HistogramEqualization::DrawHistogram(CImg<unsigned int> image)
{
	CImg<int> HistImg =image.histogram(256, 0, 255);
	if(show) HistImg.display("Histogram1",2);
	return HistImg;
}

// 功能：计算图像分布直方图
// 输入：输入单通道bmp图像
// 输出：灰度直方图
CImg<unsigned int> HistogramEqualization::GetHistogram(CImg<unsigned int> InputImage){
	CImg<unsigned int> histogram(256, 1, 1, 1, 0);
	cimg_forXY(InputImage, x, y) ++histogram[(int)InputImage(x, y)];
	return histogram;
}

// 功能：对灰度直方图进行均衡化
// 输入：单通道CImg对象, 灰度图对象
// 输出：均衡化结果
CImg<unsigned int> HistogramEqualization::HistogramEqualizationMethod
(CImg<unsigned int> InputImage,CImg<unsigned int>  Histogram)
{
	int L = 256; //灰度级
	int NumOfPixels = (InputImage)._width * (InputImage)._height;

	double CumulativeDistributionFunction[256] = { 0 }; // 灰度直方图的累积分布
	double EqualizedResult[256] = { 0 };               //  均衡化结果映射函数
	/*直方图就对应于概率密度函数pdf，
	而概率分布函数cdf就是直方图的累积和，
	即概率密度函数的积分
	积分*（L(=255)-1)*/
	
	/*均衡化关键步骤*/
	int count = 0;
	cimg_forX(Histogram, x) {
		count += Histogram[x];  // 累计求和
		CumulativeDistributionFunction[x] = 1.0 * count / NumOfPixels;  //计算概率
		EqualizedResult[x] = round(CumulativeDistributionFunction[x] * (L - 1)); // 计算概率密度，round四舍五入
	}

	/*输出均衡化结果*/
	CImg<unsigned int> OutputImage((InputImage)._width, (InputImage)._height, 1, 1, 0);
	cimg_forXY(OutputImage, x, y) // calculate histogram equalization result
		OutputImage(x, y, 0) = EqualizedResult[(InputImage)(x, y)];
	return OutputImage;
}

/*传入彩色图，转灰度图均衡化结果接口函数*/
CImg<unsigned int> HistogramEqualization::HistogramEqualizationOnGrayImage(string ImageFileName)
{
	CImg<unsigned int> hist_euq_img;
	hist_euq_img.load_bmp(ImageFileName.c_str());
	CImg<unsigned int> GrayImage = Rgb2Gray(hist_euq_img);
	char FileName[100] = {};
	sprintf_s(FileName, "%d_gray_%s", number, ImageFileName.c_str());
	if (show) GrayImage.display("灰度图");
	GrayImage.save(FileName);
	(DrawHistogram(GrayImage));
	CImg<unsigned int> EqualizedImage = HistogramEqualizationMethod(GrayImage, GetHistogram(GrayImage));
	if (show) EqualizedImage.display("灰度图均衡化");
	sprintf_s(FileName, "%d_Equalized_%s", number++, ImageFileName.c_str());
	EqualizedImage.save(FileName);
	DrawHistogram(EqualizedImage);
	return EqualizedImage;
}

/*功能： 分别对彩色图的三个通道分开求解均衡化结果，然后合并*/
/*输入： 均衡化图像文件名*/
/*输出： 彩色图均衡化结果*/
CImg<unsigned int> HistogramEqualization::Hist_Equal_ColorImage_OneColorChannel(string ImageFileName)
{
	CImg<unsigned int> ColorImage;
	ColorImage.load_bmp(ImageFileName.c_str());
	CImg<unsigned int> R_Channel = HistogramEqualizationMethod(ColorImage.get_channel(0),
		GetHistogram(ColorImage.get_channel(0)));
	CImg<unsigned int> G_Channel = HistogramEqualizationMethod(ColorImage.get_channel(1),
		GetHistogram(ColorImage.get_channel(1)));
	CImg<unsigned int> B_Channel = HistogramEqualizationMethod(ColorImage.get_channel(2), 
		GetHistogram(ColorImage.get_channel(2)));

	CImg<unsigned int> EqualizedImage = ColorImage;
	cimg_forXY(ColorImage, x, y) {
		EqualizedImage(x, y, 0) = R_Channel(x, y, 0);
		EqualizedImage(x, y, 1) = G_Channel(x, y, 0);
		EqualizedImage(x, y, 2) = B_Channel(x, y, 0);
	}
	if (show) EqualizedImage.display("通道独立均衡化结果");
	char FileName[100] = {};
	sprintf_s(FileName, "%d_OneChannels_equalize_%s", number++, ImageFileName.c_str());
	EqualizedImage.save(FileName);
	return EqualizedImage;
}

/*传入三个通道图像计算平均亮度直方图*/
CImg<unsigned int> HistogramEqualization::GetAverageHistogram
(CImg<unsigned int> img1, CImg<unsigned int> img2, CImg<unsigned int> img3)
{
	CImg<unsigned int> histogram(256, 1, 1, 1, 0);
	cimg_forXY(img1, x, y) {
		++histogram[(int)img1(x, y)];
		++histogram[(int)img2(x, y)];
		++histogram[(int)img3(x, y)];
	}
	cimg_forX(histogram, pos) histogram(pos) /= 3;
	return histogram;
}


// 功能：对彩色图进行直方图均衡化，先求三个通道的平均直方图
// 输入： 彩色图文件名
// 输出： 均衡化结果
CImg<unsigned int> HistogramEqualization::Hist_Equal_ColorImage_ThreeColorChannels(string ImageFileName)
{
	CImg<unsigned int> ColorImage;
	ColorImage.load_bmp(ImageFileName.c_str());
	CImg<unsigned int> R_Channel = ColorImage.get_channel(0);
	CImg<unsigned int> G_Channel = ColorImage.get_channel(1);
	CImg<unsigned int> B_Channel = ColorImage.get_channel(2);
	CImg<unsigned int> AverageHistogram = GetAverageHistogram(R_Channel, G_Channel, B_Channel);

	double CumulativeDistributionFunction[256] = { 0 }; // 灰度直方图的累积分布
	double EqualizedResult[256] = { 0 };          // 均衡化结果图
	int count = 0;
	int L = 256;
	int NumOfPixels = ColorImage._height * ColorImage._width;
	cimg_forX(AverageHistogram, pos) {
		count += AverageHistogram[pos];
		CumulativeDistributionFunction[pos] = 1.0 * count / NumOfPixels;
		EqualizedResult[pos] = round(CumulativeDistributionFunction[pos] * (L - 1));
	}
	CImg<unsigned int> EqualizedImage = ColorImage;
	cimg_forXY(EqualizedImage, x, y) {
		EqualizedImage(x, y, 0) = EqualizedResult[R_Channel(x, y)];
		EqualizedImage(x, y, 1) = EqualizedResult[G_Channel(x, y)];
		EqualizedImage(x, y, 2) = EqualizedResult[B_Channel(x, y)];
	}
	if (show) EqualizedImage.display("三通道直方图平均后进行均衡化的结果");
	char FileName[100] = {};
	sprintf_s(FileName, "%d_Threechannels_equalize_%s", number++, ImageFileName.c_str());
	EqualizedImage.save(FileName);
	return EqualizedImage;
}


/*功能：在HSI空间下进行均衡化，然后再转回RGB空间*/
CImg<double> HistogramEqualization::Hist_Equal_ColorImage_HSISpace(string ImageFileName)
{
	CImg<unsigned int> ColorImage;
	ColorImage.load(ImageFileName.c_str());
	// 获取RGB通道图像,并归一化
	CImg<double> R_Channel = ColorImage.get_channel(0)* 1.0 / 255.0;
	CImg<double> G_Channel = ColorImage.get_channel(1) * 1.0 / 255.0;
	CImg<double> B_Channel = ColorImage.get_channel(2) * 1.0 / 255.0;
	
	int w = ColorImage._width;
	int h = ColorImage._height;

	// HSI 
	CImg<double> Hue(w, h, 1, 1, 0);
	CImg<double> Saturation(w, h, 1, 1, 0);
	CImg<double> Intensity(w, h, 1, 1, 0);

	// for calculate Hue
	CImg<double> theta(w, h, 1, 1, 0);
	cimg_forXY(Hue, x, y) {
		double numerator = R_Channel(x, y, 0) - G_Channel(x, y, 0) + R_Channel(x, y, 0) - B_Channel(x, y, 0);
		double denominator = 2.0 * sqrt((R_Channel(x, y, 0) - G_Channel(x, y, 0)) * (R_Channel(x, y, 0) - G_Channel(x, y, 0))
			+ (R_Channel(x, y, 0) - B_Channel(x, y, 0))*(G_Channel(x, y, 0) - B_Channel(x, y, 0)));
		theta(x, y, 0) =  acos(numerator / denominator);
	}

	cimg_forXY(Hue, x, y) {
		if (G_Channel(x, y, 0) >= B_Channel(x, y, 0)) {
			Hue(x, y, 0) = theta(x, y, 0);
		}
		else {
			Hue(x, y, 0) = 2 * cimg::PI - theta(x, y, 0);
		}
	}

	// calculate saturation and intensity
	cimg_forXY(Intensity, x, y) { 
		double deno = (R_Channel(x, y, 0) + G_Channel(x, y, 0) + B_Channel(x, y, 0));
		if (deno == 0) deno = 1e-12f;  // 取极小，近似0
		Saturation(x, y, 0) = 1.0 - 3.0 * cimg::min(R_Channel(x, y, 0), B_Channel(x, y, 0), G_Channel(x, y, 0))
			/ deno;
		Intensity(x, y, 0) = 1.0 * (R_Channel(x, y, 0) + G_Channel(x, y, 0) + B_Channel(x, y, 0)) / 3.0;
		if (Saturation(x, y, 0) == 0) Hue(x, y, 0) = 0;  // 黑色
	}
	CImg<unsigned int> tmp_Intensity(w, h, 1, 1, 0);  // 亮度图像 0~255
	cimg_forXY(tmp_Intensity, x, y) {
		tmp_Intensity(x, y, 0) = floor(Intensity(x, y, 0) * 255);
	}
	
	CImg<unsigned int> I_Histogram = GetHistogram(tmp_Intensity); // 亮度直方图,由于rgb通道值/255，需要扩大255
	tmp_Intensity = HistogramEqualizationMethod(tmp_Intensity, I_Histogram); // 亮度直方图均衡化
	cimg_forXY(Intensity, x, y) {
		Intensity(x, y, 0) = tmp_Intensity(x, y, 0) * 1.0 / 255.0;
	}

	CImg<double> EqualizedImage(w, h, 1, 3, 0);

	cimg_forXY(Intensity, x, y) {
		if (Hue(x, y, 0) < 2 * cimg::PI / 3 && Hue(x, y, 0) >= 0) { //RG sector
			EqualizedImage(x, y, 2) = Intensity(x, y, 0) * (1 - Saturation(x,y,0));
			EqualizedImage(x, y, 0) = Intensity(x, y, 0) *(1 + Saturation(x, y, 0) * cos(Hue(x, y, 0)) / cos(cimg::PI / 3 - Hue(x,y,0)));
			EqualizedImage(x, y, 1) = 3 * Intensity(x, y, 0) - (EqualizedImage(x,y,2) + EqualizedImage(x, y, 0));
		}
		else if (Hue(x, y, 0) < 4 * cimg::PI / 3 && Hue(x, y, 0) >= 2 * cimg::PI / 3) { //GB sector
			EqualizedImage(x, y, 0) = Intensity(x, y, 0) * (1 - Saturation(x, y, 0));
			EqualizedImage(x, y, 1) = Intensity(x, y, 0) *(1 + Saturation(x, y, 0) * cos(Hue(x, y, 0) - 2*cimg::PI/3) / cos(cimg::PI - Hue(x, y, 0)));
			EqualizedImage(x, y, 2) = 3 * Intensity(x, y, 0) - (EqualizedImage(x, y, 1) + EqualizedImage(x, y, 0));
		}
		else if (Hue(x, y, 0) <= 2 * cimg::PI  && Hue(x, y, 0) >= 4 * cimg::PI / 3) { // BR sector
			EqualizedImage(x, y, 1) = Intensity(x, y, 0) * (1 - Saturation(x, y, 0));
			EqualizedImage(x, y, 2) = Intensity(x, y, 0) *(1 + Saturation(x, y, 0) * cos(Hue(x, y, 0) - 4 * cimg::PI / 3) / cos(5 * cimg::PI / 3 - Hue(x, y, 0)));
			EqualizedImage(x, y, 0) = 3 * Intensity(x, y, 0) - (EqualizedImage(x, y, 1) + EqualizedImage(x, y, 2));
		}
	}
	
	CImg<unsigned int> resultImage(w, h, 1, 3, 0);
	cimg_forXY(EqualizedImage, x, y) {
		resultImage(x, y, 0) = 255 * min(max(EqualizedImage(x, y, 0), 0.0), 1.0);
		resultImage(x, y, 1) = 255 * min(max(EqualizedImage(x, y, 1), 0.0), 1.0);
		resultImage(x, y, 2) = 255 * min(max(EqualizedImage(x, y, 2), 0.0), 1.0);
	}
	//resultImage = resultImage.normalize(0, 255);
	if (show) resultImage.display("HSI空间下均衡化的结果");
	char FileName[100] = {};
	//sprintf_s(FileName, "%d_HSI_equalize_%s", number++, ImageFileName.c_str());
	//resultImage.save(FileName);
	return resultImage;
}


