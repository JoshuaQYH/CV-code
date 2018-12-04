#include "Utils.h"
#include "PanoImageStitching.h"



int main() {
	PanoImageStitching stitchTool;
	string imageFolder = "ImageData1\\";
	vector<string> imageFilePath;
	imageFilePath.push_back("ImageData1\\11.bmp");
	imageFilePath.push_back("ImageData1\\22.bmp");
	imageFilePath.push_back("ImageData1\\33.bmp");
	imageFilePath.push_back("ImageData1\\44.bmp");
	
	//imageFilePath.push_back("ImageData2\\1.bmp");
	
	/*
	imageFilePath.push_back("ImageData2\\3.bmp");
	imageFilePath.push_back("ImageData2\\4.bmp");
	imageFilePath.push_back("ImageData2\\5.bmp");
	imageFilePath.push_back("ImageData2\\6.bmp");
	imageFilePath.push_back("ImageData2\\7.bmp");
	imageFilePath.push_back("ImageData2\\8.bmp");
	imageFilePath.push_back("ImageData2\\9.bmp");
	imageFilePath.push_back("ImageData2\\10.bmp");
	imageFilePath.push_back("ImageData2\\11.bmp");
	imageFilePath.push_back("ImageData2\\12.bmp");
	*/

	//getAllFiles(imageFolder, imageFilePath);
	//ReadImageFileName(imageFolder, imageFilePath);
	if (DEBUG) cout << "读取所有图片名完成" << endl;
	
	stitchTool.ReadAllImage(imageFilePath);
	if (DEBUG) cout << "写入图片名到拼接器中完成" << endl;

	stitchTool.StartStitching();
	system("Pause");
}

