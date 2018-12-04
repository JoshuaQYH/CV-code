#pragma once 
#include "Utils.h"


void getAllFiles(string path, vector<string> &files) {
	long    hFile = 0;
	//文件信息    
	struct _finddata_t fileinfo;
	string p;
	cout << "------------------------" << endl;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			if ((fileinfo.attrib &  _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					getAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else {
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				
			}
			cout << p.assign(path) << endl;
			cout << p.assign(path).append("\\").append(fileinfo.name) << endl;
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
	else {
		cout << p.assign(path) << " No such directory" << endl;
	}

}


// 读取文件夹中的所有文件名
vector<string>& ReadImageFileName(string imageFileFolderPath, vector<string>& imageFilePathVector)
{
	//文件句柄
	long   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo = { 0 };
	string p = "";
	if ((hFile = _findfirst(p.assign(imageFileFolderPath).append("\\*").c_str(), &fileinfo)) != -1)
	{
		cout << p.assign(imageFileFolderPath).append("\\*").c_str() << endl;
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					ReadImageFileName(p.assign(imageFileFolderPath).append("\\").append(fileinfo.name), imageFilePathVector);
			}
			else
			{
				imageFilePathVector.push_back(p.assign(imageFileFolderPath).append("\\").append(fileinfo.name));
			}
			cout << hFile << endl;
			cout << fileinfo.size << endl;
			

		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return imageFilePathVector;
}

int random(int min, int max)
{
	if (min > max) {
		int tmp = max;
		max = min;
		min = tmp;
	}
	return rand() % (max - min + 1) + min;
}




float get_warped_x(float x, float y, HomographyMatrix H) {
	return H.a11 * x + H.a12 * y + H.a13 * x * y + H.a21;
}

float get_warped_y(float x, float y, HomographyMatrix H) {
	return H.a22 * x + H.a23 * y + H.a31 * x * y + H.a32;
}



CImg<float> cylinderProjection(const CImg<float> &src) {
	
	int projection_width, projection_height;
	CImg<unsigned char> res(src.width(), src.height(), 1, src.spectrum(), 255);
	float r;
	float angle = 15.0;
	
	if (src.width() > src.height()) {
		projection_width = src.height();
		projection_height = src.width();

		r = (projection_width / 2.0) / tan(angle * cimg::PI / 180.0);

		for (int i = 0; i < res.width(); i++) {
			for (int j = 0; j < res.height(); j++) {
				float dst_x = j - projection_width / 2;
				float dst_y = i - projection_height / 2;

				float k = r / sqrt(r * r + dst_x * dst_x);
				float src_x = dst_x / k;
				float src_y = dst_y / k;

				if (src_x + projection_width / 2 >= 0 && src_x + projection_width / 2 < src.height()
					&& src_y + projection_height / 2 >= 0 && src_y + projection_height / 2 < src.width()) {
					for (int k = 0; k < res.spectrum(); k++) {
						res(i, j, k) = src.linear_atXY(src_x + projection_height / 2, src_y + projection_width / 2, k);
					}
				}
			}
		}

	}
	else {
		projection_width = src.width();
		projection_height = src.height();

		r = (projection_width / 2.0) / tan(angle * cimg::PI / 180.0);

		for (int i = 0; i < res.width(); i++) {
			for (int j = 0; j < res.height(); j++) {
				float dst_x = i - projection_width / 2;
				float dst_y = j - projection_height / 2;

				float k = r / sqrt(r * r + dst_x * dst_x);
				float src_x = dst_x / k;
				float src_y = dst_y / k;

				if (src_x + projection_width / 2 >= 0 && src_x + projection_width / 2 < src.width()
					&& src_y + projection_height / 2 >= 0 && src_y + projection_height / 2 < src.height()) {
					for (int k = 0; k < res.spectrum(); k++) {
						res(i, j, k) = src.linear_atXY(src_x + projection_width / 2, src_y + projection_height / 2, k);
						//res(i, j, k) = bilinear_interpolation(src, src_x + projection_width / 2, src_y + projection_height / 2, k);
					}
				}
			}
		}

	}
	
	return res;
}