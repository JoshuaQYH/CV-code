#pragma once
#include "PanoImageStitching.h"
#include "Utils.h"


void DisplayFeatureImage(map<vector<float>, VlSiftKeypoint> pointSet, CImg<float> img)
{
	CImg<float> res = img;
	int w = img._width;
	int h = img._height;
	for (auto it = pointSet.begin(); it != pointSet.end(); it++) {
		int a = it->second.ix;
		int b = it->second.iy;
		int r = it->second.sigma;
		for (int i = a - r - 1; i < a + r + 1; i++) {
			if (i < 0 || i > w)
				continue;
			for (int j = b - r - 1; j < b + r + i; j++) {
				if (j < 0 || j > h)
					continue;
				if (abs((i - a) * (i - a) + (j - b)*(j - b) - r * r) <= 2) {
					res(i, j, 0) = 255;
					res(i, j, 1) = 0;
					res(i, j, 2) = 0;
				}
			}
		}
	}
	res.display("特征点提取示意图");
}


void PanoImageStitching::ReadAllImage(vector<string> imageFilePath)
{
	for (int i = 0; i < imageFilePath.size(); i++) {
		CImg<float> img(imageFilePath[i].c_str());
		imageGroup.push_back(img);
		imageFileNameGroup.push_back(imageFilePath[i]);
	}
	imageNum = imageGroup.size();
}


CImg<float> PanoImageStitching::ImageToSphericalCoordinates(CImg<float> image)
{
	CImg<float> res = cylinderProjection(image);
	return res;
}



CImg<float> PanoImageStitching::ToGrayImage(CImg<float>& image)
{
	CImg<float> gray_img(image._width, image._height, 1, 1, 0);
	cimg_forXY(image, x, y) {
		int r = image(x, y, 0);
		int g = image(x, y, 1);
		int b = image(x, y, 2);
		gray_img(x, y) = 0.299 * r + 0.587 * g + 0.114 * b;
	}
	return gray_img;
}

/*提取每一幅图像的特征点*/
map<vector<float>, VlSiftKeypoint> PanoImageStitching::ExtractSIFTFeaturePoint(CImg<float> grayImage)
{
	cout << "sift特征点提取" << endl;
	map<vector<float>, VlSiftKeypoint> res;

	int width = grayImage.width();
	int height = grayImage.height();

	vl_sift_pix *ImageData = new vl_sift_pix[width * height];
	cimg_forXY(grayImage, x, y) {
		ImageData[y * width + x] = grayImage(x, y, 0);
	}

	int noctaves = 6, nlevels = 3, o_min = 0;

	// 定义vlsiftfilt的结构体指针
	VlSiftFilt* siftFilt = NULL;
	siftFilt = vl_sift_new(width, height, noctaves, nlevels, o_min);

	//int KeyPoint = 0;
	//int idx = 0;
	//  开始处理分片
	if (vl_sift_process_first_octave(siftFilt, ImageData) != VL_ERR_EOF) {
		while (1) {
			// 计算每一组中的关键点
			vl_sift_detect(siftFilt);
			// 遍历并绘制每一个点
			//KeyPoint += siftFilt->nkeys;  // 检测到的关键点的数目
			VlSiftKeypoint* pKeypointList = siftFilt->keys; // 检测到的关键点列表指针

			for (int i = 0; i < siftFilt->nkeys; i++) {
				VlSiftKeypoint tmpKeyPoint = *pKeypointList;
				pKeypointList++;
				char red[3] = { 255,0,0 };
				grayImage.draw_circle(tmpKeyPoint.x, tmpKeyPoint.y, tmpKeyPoint.sigma / 2, red, 1);

				//idx++;
				double angles[4];
				// 计算特征点的方向
				int angleCount = vl_sift_calc_keypoint_orientations(siftFilt, angles, &tmpKeyPoint);
				for (int j = 0; j < angleCount; j++) {
					// 计算每一个方向的描述
					double tmpAngle = angles[j];
					float* Descriptors = new float[128];
					// 生成特征点的描述子
					vl_sift_calc_keypoint_descriptor(siftFilt, Descriptors, &tmpKeyPoint, tmpAngle);
					
					// 加入结果
					vector<float> tmp(128);
					for (int k = 0; k < 128; k++) {
						tmp[k] = Descriptors[k];
					}
					//cout << "tmp " << tmp.size() << endl;
					res.insert(pair<vector<float>, VlSiftKeypoint>(tmp, tmpKeyPoint));

					delete[]Descriptors;
					Descriptors = NULL;
				}
			}

			if (vl_sift_process_next_octave(siftFilt) == VL_ERR_EOF)
				break;
		}
	}
	vl_sift_delete(siftFilt);
	delete ImageData;
	ImageData = NULL;
	return res;
}

/***************************************************
 函数功能：利用k-d树找出两张图片中的匹配的特征点
 输入：两个图像的特征点集合,1 用于搜索，2 用于查询
 输出：返回两个图像匹配的特征点对
******************************************************/
vector<KeyPointPair> PanoImageStitching::FindKNearestNeighbor(map<vector<float>, VlSiftKeypoint> &featurePoint1, map<vector<float>, VlSiftKeypoint> &featurePoint2)
{
	
	// 初始化一个森林，数据类型为float，维度128，树数目1，距离类型
	VlKDForest* forest = vl_kdforest_new(VL_TYPE_FLOAT, 128, 1, VlDistanceL1);

	float* forestData = new float[128 * featurePoint1.size()];
	int k = 0; 
	// 将查询点集特征向量存储到forestData
	for (auto it = featurePoint1.begin(); it != featurePoint1.end(); it++) {
		const vector<float> &descriptors = it->first;

		assert(descriptors.size() == 128);

		for (int i = 0; i < 128; i++) {
			forestData[i + 128 * k] = descriptors[i];
		}
		k++;
	}

	// 构建一个k-d树，传入森林，点集树，数据
	vl_kdforest_build(forest, featurePoint1.size(), forestData);

	vector<KeyPointPair> res; 

	// 初始化搜索指针
	VlKDForestSearcher* searcher = vl_kdforest_new_searcher(forest);
	// 声明最近邻数组
	VlKDForestNeighbor neighbors[2];

	// 遍历2的每一个点集，搜索 与 1 中的最近2个特征点
	for (auto it = featurePoint2.begin(); it != featurePoint2.end(); it++) {
		float* tmpData = new float[128];

		// 存储搜索特征点集的特征向量
		for (int i = 0; i < 128; i++) {
			tmpData[i] = (it->first)[i];
		}

		// 比较两个数据集之间相似点
		int nVisited = vl_kdforestsearcher_query(searcher, neighbors, 2, tmpData);

		float ratio = neighbors[0].distance / neighbors[1].distance;

		if (ratio < 0.5) {
			// 接收该特征点
			vector<float> des(128);
			for (int j = 0; j < 128; j++) {
				des[j] = forestData[j + neighbors[0].index * 128];
			}
			VlSiftKeypoint left = featurePoint1.find(des)->second;
			VlSiftKeypoint right = it->second;
			res.push_back(KeyPointPair(left, right));
		}
		
		delete[] tmpData;
		tmpData = NULL;
	}
	vl_kdforestsearcher_delete(searcher);
	vl_kdforest_delete(forest);

	delete[]forestData;
	forestData = NULL;
	return res;
}

/*根据特征点对计算单应矩阵*/
HomographyMatrix PanoImageStitching::GetHomographyFromPointPairs(const vector<KeyPointPair>& pair)
{
	assert(pair.size() == 4);

	// 要恢复H中的8个参数，需要有4对特征点
	float u0 = pair[0].vp1.x, v0 = pair[0].vp1.y;
	float u1 = pair[1].vp1.x, v1 = pair[1].vp1.y;
	float u2 = pair[2].vp1.x, v2 = pair[2].vp1.y;
	float u3 = pair[3].vp1.x, v3 = pair[3].vp1.y;

	float x0 = pair[0].vp2.x, y0 = pair[0].vp2.y;
	float x1 = pair[1].vp2.x, y1 = pair[1].vp2.y;
	float x2 = pair[2].vp2.x, y2 = pair[2].vp2.y;
	float x3 = pair[3].vp2.x, y3 = pair[3].vp2.y;

	float c1, c2, c3, c4, c5, c6, c7, c8;

	c1 = -(u0*v0*v1*x2 - u0 * v0*v2*x1 - u0 * v0*v1*x3 + u0 * v0*v3*x1 - u1 * v0*v1*x2 + u1 * v1*v2*x0 + u0 * v0*v2*x3 - u0 * v0*v3*x2 + u1 * v0*v1*x3 - u1 * v1*v3*x0 + u2 * v0*v2*x1 - u2 * v1*v2*x0
		- u1 * v1*v2*x3 + u1 * v1*v3*x2 - u2 * v0*v2*x3 + u2 * v2*v3*x0 - u3 * v0*v3*x1 + u3 * v1*v3*x0 + u2 * v1*v2*x3 - u2 * v2*v3*x1 + u3 * v0*v3*x2 - u3 * v2*v3*x0 - u3 * v1*v3*x2 + u3 * v2*v3*x1)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);

	c2 = (u0*u1*v0*x2 - u0 * u2*v0*x1 - u0 * u1*v0*x3 - u0 * u1*v1*x2 + u0 * u3*v0*x1 + u1 * u2*v1*x0 + u0 * u1*v1*x3 + u0 * u2*v0*x3 + u0 * u2*v2*x1 - u0 * u3*v0*x2 - u1 * u2*v2*x0 - u1 * u3*v1*x0
		- u0 * u2*v2*x3 - u0 * u3*v3*x1 - u1 * u2*v1*x3 + u1 * u3*v1*x2 + u1 * u3*v3*x0 + u2 * u3*v2*x0 + u0 * u3*v3*x2 + u1 * u2*v2*x3 - u2 * u3*v2*x1 - u2 * u3*v3*x0 - u1 * u3*v3*x2 + u2 * u3*v3*x1)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);

	c3 = (u0*v1*x2 - u0 * v2*x1 - u1 * v0*x2 + u1 * v2*x0 + u2 * v0*x1 - u2 * v1*x0 - u0 * v1*x3 + u0 * v3*x1 + u1 * v0*x3 - u1 * v3*x0 - u3 * v0*x1 + u3 * v1*x0
		+ u0 * v2*x3 - u0 * v3*x2 - u2 * v0*x3 + u2 * v3*x0 + u3 * v0*x2 - u3 * v2*x0 - u1 * v2*x3 + u1 * v3*x2 + u2 * v1*x3 - u2 * v3*x1 - u3 * v1*x2 + u3 * v2*x1)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);

	c4 = (u0*u1*v0*v2*x3 - u0 * u1*v0*v3*x2 - u0 * u2*v0*v1*x3 + u0 * u2*v0*v3*x1 + u0 * u3*v0*v1*x2 - u0 * u3*v0*v2*x1 - u0 * u1*v1*v2*x3 + u0 * u1*v1*v3*x2 + u1 * u2*v0*v1*x3 - u1 * u2*v1*v3*x0 - u1 * u3*v0*v1*x2 + u1 * u3*v1*v2*x0
		+ u0 * u2*v1*v2*x3 - u0 * u2*v2*v3*x1 - u1 * u2*v0*v2*x3 + u1 * u2*v2*v3*x0 + u2 * u3*v0*v2*x1 - u2 * u3*v1*v2*x0 - u0 * u3*v1*v3*x2 + u0 * u3*v2*v3*x1 + u1 * u3*v0*v3*x2 - u1 * u3*v2*v3*x0 - u2 * u3*v0*v3*x1 + u2 * u3*v1*v3*x0)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);

	c5 = -(u0*v0*v1*y2 - u0 * v0*v2*y1 - u0 * v0*v1*y3 + u0 * v0*v3*y1 - u1 * v0*v1*y2 + u1 * v1*v2*y0 + u0 * v0*v2*y3 - u0 * v0*v3*y2 + u1 * v0*v1*y3 - u1 * v1*v3*y0 + u2 * v0*v2*y1 - u2 * v1*v2*y0
		- u1 * v1*v2*y3 + u1 * v1*v3*y2 - u2 * v0*v2*y3 + u2 * v2*v3*y0 - u3 * v0*v3*y1 + u3 * v1*v3*y0 + u2 * v1*v2*y3 - u2 * v2*v3*y1 + u3 * v0*v3*y2 - u3 * v2*v3*y0 - u3 * v1*v3*y2 + u3 * v2*v3*y1)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);

	c6 = (u0*u1*v0*y2 - u0 * u2*v0*y1 - u0 * u1*v0*y3 - u0 * u1*v1*y2 + u0 * u3*v0*y1 + u1 * u2*v1*y0 + u0 * u1*v1*y3 + u0 * u2*v0*y3 + u0 * u2*v2*y1 - u0 * u3*v0*y2 - u1 * u2*v2*y0 - u1 * u3*v1*y0
		- u0 * u2*v2*y3 - u0 * u3*v3*y1 - u1 * u2*v1*y3 + u1 * u3*v1*y2 + u1 * u3*v3*y0 + u2 * u3*v2*y0 + u0 * u3*v3*y2 + u1 * u2*v2*y3 - u2 * u3*v2*y1 - u2 * u3*v3*y0 - u1 * u3*v3*y2 + u2 * u3*v3*y1)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);

	c7 = (u0*v1*y2 - u0 * v2*y1 - u1 * v0*y2 + u1 * v2*y0 + u2 * v0*y1 - u2 * v1*y0 - u0 * v1*y3 + u0 * v3*y1 + u1 * v0*y3 - u1 * v3*y0 - u3 * v0*y1 + u3 * v1*y0
		+ u0 * v2*y3 - u0 * v3*y2 - u2 * v0*y3 + u2 * v3*y0 + u3 * v0*y2 - u3 * v2*y0 - u1 * v2*y3 + u1 * v3*y2 + u2 * v1*y3 - u2 * v3*y1 - u3 * v1*y2 + u3 * v2*y1)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);

	c8 = (u0*u1*v0*v2*y3 - u0 * u1*v0*v3*y2 - u0 * u2*v0*v1*y3 + u0 * u2*v0*v3*y1 + u0 * u3*v0*v1*y2 - u0 * u3*v0*v2*y1 - u0 * u1*v1*v2*y3 + u0 * u1*v1*v3*y2 + u1 * u2*v0*v1*y3 - u1 * u2*v1*v3*y0 - u1 * u3*v0*v1*y2 + u1 * u3*v1*v2*y0
		+ u0 * u2*v1*v2*y3 - u0 * u2*v2*v3*y1 - u1 * u2*v0*v2*y3 + u1 * u2*v2*v3*y0 + u2 * u3*v0*v2*y1 - u2 * u3*v1*v2*y0 - u0 * u3*v1*v3*y2 + u0 * u3*v2*v3*y1 + u1 * u3*v0*v3*y2 - u1 * u3*v2*v3*y0 - u2 * u3*v0*v3*y1 + u2 * u3*v1*v3*y0)
		/ (u0*u1*v0*v2 - u0 * u2*v0*v1 - u0 * u1*v0*v3 - u0 * u1*v1*v2 + u0 * u3*v0*v1 + u1 * u2*v0*v1 + u0 * u1*v1*v3 + u0 * u2*v0*v3 + u0 * u2*v1*v2 - u0 * u3*v0*v2 - u1 * u2*v0*v2 - u1 * u3*v0*v1
			- u0 * u2*v2*v3 - u0 * u3*v1*v3 - u1 * u2*v1*v3 + u1 * u3*v0*v3 + u1 * u3*v1*v2 + u2 * u3*v0*v2 + u0 * u3*v2*v3 + u1 * u2*v2*v3 - u2 * u3*v0*v3 - u2 * u3*v1*v2 - u1 * u3*v2*v3 + u2 * u3*v1*v3);


	return HomographyMatrix(c1, c2, c3, c4, c5, c6, c7, c8);
}

/************************************************************
传入单应矩阵，所有特征点对，以及随机挑选的特征点对
计算该随机挑选的特征点对的内点数目
***************************************************************/
vector<int> PanoImageStitching::ComputeInliers(HomographyMatrix HMatrix, vector<KeyPointPair> pair, set<int> randomPairIndex)
{
	vector<int> inlierIndex;
	for (int i = 0; i < pair.size(); i++) {
		if (randomPairIndex.find(i) != randomPairIndex.end())
			continue;

		//在某一特征点对上进行验证测试
		//先获取变换匹配后的点坐标
		int realX = pair[i].vp2.x;
		int realY = pair[i].vp2.y;

		// 利用单应矩阵计算变换后的点的坐标。
		int x = pair[i].vp1.x;
		int y = pair[i].vp2.y;

		int homographyX = HMatrix.a11 * x + y * HMatrix.a12 + HMatrix.a13 * x * y + HMatrix.a21;
		int homographyY = HMatrix.a22 * x + y * HMatrix.a23 + HMatrix.a31 * x * y + HMatrix.a32;

		int distance = sqrt((realX - homographyX)* (realX - homographyX) + (realY - homographyY)*(realY - homographyY));
		
		if (distance < RANSAC_THRESHOLD) {
			inlierIndex.push_back(i);
		}
	}
	return inlierIndex;
}


/*利用最大的可信点集以及最小二乘法估计最终的单应矩阵*/
HomographyMatrix PanoImageStitching::ComputeLeastSquaresHomographyMatrix(vector<KeyPointPair> pairs, vector<int> inlier_Indexs)
{
	int calc_size = inlier_Indexs.size();

	CImg<double> A(4, calc_size, 1, 1, 0);
	CImg<double> b(1, calc_size, 1, 1, 0);

	for (int i = 0; i < calc_size; i++) {
		int cur_index = inlier_Indexs[i];

		A(0, i) = pairs[cur_index].vp1.x;
		A(1, i) = pairs[cur_index].vp1.y;
		A(2, i) = pairs[cur_index].vp1.x * pairs[cur_index].vp1.y;
		A(3, i) = 1;

		b(0, i) = pairs[cur_index].vp2.x;
	}
	

	CImg<double> x1 = b.get_solve(A);

	for (int i = 0; i < calc_size; i++) {
		int cur_index = inlier_Indexs[i];
		b(0, i) = pairs[cur_index].vp2.y;
	}

	CImg<double> x2 = b.get_solve(A);
	//cout << " a32 " << x2(0, 3) << endl;
	return HomographyMatrix(x1(0, 0), x1(0, 1), x1(0, 2), x1(0, 3), x2(0, 0), x2(0, 1), x2(0, 2), x2(0, 3));

}


/********************************
经过ransac得到特征点对集合的单应矩阵
**********************************/
HomographyMatrix PanoImageStitching::RANSACForHomographyMatrix(vector<KeyPointPair> featurePointPair)
{
	if (DEBUG) cout << "RANSAC for matrix" << endl;
	// 过滤较少的特征对
	assert(featurePointPair.size() >= NUM_OF_PAIR);

	float P = CONFIDENCE;  // 置信度
	float p = NUM_OF_PAIR;  
	float inlierRatio = INLIER_RATIO;
	int iterNum = ceil(log(1 - P) / log(1 - pow(inlierRatio, p))); // 计算迭代次数
	
	vector<int> maxInliersIndex;  //保存每次迭代数量最多的可信点
	int tmpIterNum = iterNum;
	cout << "迭代次数" << iterNum << endl;
	while (--iterNum) {
		// 随机存储四对特征点
		vector<KeyPointPair> randomPairs;
		// 存储随机特征点对的下标
		set<int> indexSet;

		for (int i = 0; i < NUM_OF_PAIR; i++) {
			int randomIndex = rand()%featurePointPair.size();
			while (indexSet.find(randomIndex) != indexSet.end()) {
				randomIndex = rand()%featurePointPair.size();
			}
			randomPairs.push_back(featurePointPair[randomIndex]);
			
			indexSet.insert(randomIndex);
		}
		// 从挑选得到的四对特征点对，计算得到单应矩阵
		HomographyMatrix HMatrix = this->GetHomographyFromPointPairs(randomPairs);
	//	cout << "---------------------- ransac 的矩阵" << endl;
		//HMatrix.print();
		
		// 利用单应矩阵，选择的特征点对下标，所有特征点对，计算可信点的下标
		vector<int> inliersIndex = ComputeInliers(HMatrix, featurePointPair, indexSet);
	//	cout << "内点数目为 "<<inliersIndex.size() << endl;
	///	cout << "最多内点数为" << maxInliersIndex.size() << endl;
	//	cout << "----------------------" << endl;
		// 保存拥有最多可信点的特征点对
		if (inliersIndex.size() > maxInliersIndex.size()) {
			maxInliersIndex.clear();
			for (int i = 0; i < inliersIndex.size(); i++) {
				maxInliersIndex.push_back(inliersIndex[i]);
			}
		}
		if (iterNum == 1 && maxInliersIndex.size() < featurePointPair.size() / 4)
			iterNum += tmpIterNum;
	}
	//cout << "maxInliersIndex " << maxInliersIndex.size() << endl;
	// 根据选出的的可信点的下标，使用最小二乘法重新估计单应矩阵
	HomographyMatrix H = ComputeLeastSquaresHomographyMatrix(featurePointPair, maxInliersIndex);
	//cout << "--------------" << endl;
	//H.print();
	//cout << "最终单应矩阵,内点数为" << maxInliersIndex.size()<< endl;
	// 检查单应矩阵的优劣

	return H;
}

/*检查单应矩阵是否是好的
条件1： H（3，1） H（3，2）大于最大透视度 2e - 3返回true 
条件2： 计算左上角2*2矩阵的行列式，如果大于0则是保持方向的，否则翻转返回false*/
bool PanoImageStitching::MathVerification(HomographyMatrix HMatrix)
{
	float HOMO_MAX_PERSPECTIVE = 0.003;
	if (abs(HMatrix.a31) > HOMO_MAX_PERSPECTIVE || abs(HMatrix.a32) > 12)
		return false;

	
	return true;
}

/*传入仿射变换的矩阵计算拼接后图像的长度和宽度*/
CImg<float> PanoImageStitching::CalSizeOfStitchingImage(CImg<float> stitchedImage, CImg<float> adjacentImage, HomographyMatrix transformMatrix)
{
	CImg<float> res(2, 3);

	int adj_w = adjacentImage._width;
	int adj_h = adjacentImage._height;
	
	int stitched_w = stitchedImage._width;
	int stitched_h = stitchedImage._height;

	// 计算变换后x的最小值
	float minX = 0;
	// 从四个角点开始变换
	minX = cimg::min(get_warped_x(0, 0, transformMatrix), 
		get_warped_x(adj_w - 1, 0, transformMatrix),
		get_warped_x(0, adj_h - 1, transformMatrix), 
		get_warped_x(adj_w - 1, adj_h - 1, transformMatrix));
	if (minX > 0) minX = 0;

	// 计算变换后y的最小值
	float minY = 0;
	minY = cimg::min(get_warped_y(0, 0, transformMatrix),
		get_warped_y(adj_w - 1, 0, transformMatrix),
		get_warped_y(0, adj_h - 1, transformMatrix),
		get_warped_y(adj_w - 1, adj_h - 1, transformMatrix));
	if (minY > 0) minY = 0;
	
	// 计算变换后 x的最大值
	float maxX = 0;
	maxX = cimg::max(get_warped_x(0, 0, transformMatrix),
		get_warped_x(adj_w - 1, 0, transformMatrix),
		get_warped_x(0, adj_h - 1, transformMatrix),
		get_warped_x(adj_w - 1, adj_h - 1, transformMatrix));
	if (maxX < stitched_w) maxX = stitched_w;

	// 计算变换后y的最大值
	float maxY = 0;
	maxY = cimg::max(get_warped_y(0, 0, transformMatrix),
		get_warped_y(adj_w - 1, 0, transformMatrix),
		get_warped_y(0, adj_h - 1, transformMatrix),
		get_warped_y(adj_w - 1, adj_h - 1, transformMatrix));
	if (maxY < stitched_h) maxY = stitched_h;

	int outputX = ceil(maxX - minX);
	int outputY = ceil(maxY - minY);

	res(0, 0) = outputX;
	res(0, 1) = outputY;

	res(1, 0) = minX;
	res(1, 1) = minY;

	res(2, 0) = maxX;
	res(2, 1) = maxY;
	//res.resize(outputX, outputY);
//	if (DEBUG) cout << "outputX " << outputX << " outputY " << outputY << endl;
	//if (DEBUG) cout << "minX " << minX << " minY " << minY << endl;
//	if (DEBUG) cout << "maxX " << maxX << " maxY " << maxY << endl;
//	cout << "=============" << endl;
	return res;
}


// 利用x， y偏移量，将拼接完成的图片，替换到下一张要拼接的图片中。
void PanoImageStitching::ShiftImageByOffset(CImg<float>& stitchedImg, CImg<float>& dstImg, int offsetX, int offsetY)
{
	cimg_forXY(dstImg, x, y) {
		int x0 = x + offsetX;
		int y0 = y + offsetY;

		if (x0 >= 0 && x0 < stitchedImg.width() && y0 >= 0 && y0 < stitchedImg._height) {
			cimg_forC(dstImg, c) {
				dstImg(x, y, c) = stitchedImg(x0, y0, c);
			}
		}
	}

//	dstImg.display("图片偏移");
}

/*利用单应矩阵对图像进行扭曲*/
void PanoImageStitching::WarpingImageWithHomography(CImg<float>& stitchedImg, CImg<float>& dstImage, HomographyMatrix H, float offsetX, float offsetY)
{//
	//cout << stitchedImg._width << " " << stitchedImg._height << "  " << dstImage._width << " " << dstImage._height << endl;
	cout << offsetX << " " << offsetY << endl;
	cimg_forXY(dstImage, dstX, dstY){
		int srcX = get_warped_x(dstX + offsetX, dstY + offsetY, H);
		int srcY = get_warped_y(dstX + offsetY, dstY + offsetY, H);
		if (srcX >= 0 && srcX < stitchedImg._width && srcY >= 0 && srcY < stitchedImg._height) {
			cimg_forC(stitchedImg, c) {
				dstImage(dstX, dstY, c) = stitchedImg(srcX, srcY, c);
			}
		}
	}
//	dstImage.display("图片扭曲");
}

/*利用xy偏移量计算特征点的偏移后的位子*/
void PanoImageStitching::ShifFeaturePointByOffset(map<vector<float>, VlSiftKeypoint>& featurePointSet, int offsetX, int offsetY)
{
	for (auto it = featurePointSet.begin(); it != featurePointSet.end(); it++) {
		it->second.x -= offsetX;
		it->second.y -= offsetY;
		it->second.ix = int(it->second.x);
		it->second.iy = int(it->second.y);
	}
	
}

/* 利用单应矩阵重新更新特征点的坐标*/
void PanoImageStitching::WarpingFeaturePointWithHomography(map<vector<float>, VlSiftKeypoint>& featurePointSet, HomographyMatrix H, int offsetX, int offsetY)
{
	for (auto it = featurePointSet.begin(); it != featurePointSet.end(); it++) {
		float tmpX = it->second.x;
		float tmpY = it->second.y;

		it->second.x = get_warped_x(tmpX, tmpY, H) - offsetX;
		it->second.y = get_warped_y(tmpX, tmpY, H) - offsetY;

		it->second.ix = int(it->second.x);
		it->second.iy = int(it->second.y);
	}
}


/*多波段融合函数接口
1. 


*/
bool isEmpty(const CImg<unsigned char> &img, int x, int y) {
	assert(img.spectrum() == 3);
	return (img(x, y, 0) == 255 && img(x, y, 1) == 255 && img(x, y, 2) == 255);
}

CImg<float>  PanoImageStitching::blendTwoImages(const CImg<float> &a, const CImg<float> &b) {
	assert(a.width() == b.width() && a.height() == b.height() && a.spectrum() == b.spectrum());

	// Find the center point of a and overlapping part.
	double sum_a_x = 0;
	double sum_a_y = 0;
	int a_n = 0;
	//double sum_b_x = 0;
	//double sum_b_y = 0;
	//int b_n = 0;
	double sum_overlap_x = 0;
	double sum_overlap_y = 0;
	int overlap_n = 0;
	if (a.width() > a.height()) {
		for (int x = 0; x < a.width(); x++) {
			if (!isEmpty(a, x, a.height() / 2)) {
				sum_a_x += x;
				a_n++;
			}

			//if (!isEmpty(b, x, b.height() / 2)) {
			//	sum_b_x += x;
			//	b_n++;
			//}

			if (!isEmpty(a, x, a.height() / 2) && !isEmpty(b, x, a.height() / 2)) {
				sum_overlap_x += x;
				overlap_n++;
				
			}
		}
	}
	else {
		for (int y = 0; y < a.height(); y++) {
			if (!isEmpty(a, a.width() / 2, y)) {
				sum_a_y += y;
				a_n++;
			}

			if (!isEmpty(a, a.width() / 2, y) && !isEmpty(b, b.width() / 2, y)) {
				sum_overlap_y += y;
				overlap_n++;
			
			}
		}
	}

	int min_len = (a.width() < a.height()) ? a.width() : a.height();

	int n_level = floor(log2(min_len));

	vector<CImg<float> > a_pyramid(n_level);
	vector<CImg<float> > b_pyramid(n_level);
	vector<CImg<float> > mask(n_level);

	// Initialize the base.
	a_pyramid[0] = a;
	b_pyramid[0] = b;
	mask[0] = CImg<float>(a.width(), a.height(), 1, 1, 0);

	if (a.width() > a.height()) {
		if (sum_a_x / a_n < sum_overlap_x / overlap_n) {
			for (int x = 0; x < sum_overlap_x / overlap_n; x++) {
				for (int y = 0; y < a.height(); y++) {
					mask[0](x, y) = 1;
				}
			}
		}
		else {
			for (int x = sum_overlap_x / overlap_n + 1; x < a.width(); x++) {
				for (int y = 0; y < a.height(); y++) {
					mask[0](x, y) = 1;
				}
			}
		}
	}
	else {
		if (sum_a_y / a_n < sum_overlap_y / overlap_n) {
			for (int x = 0; x < a.width(); x++) {
				for (int y = 0; y < sum_overlap_y / overlap_n; y++) {
					mask[0](x, y) = 1;
				}
			}
		}
		else {
			for (int x = 0; x < a.width(); x++) {
				for (int y = sum_overlap_y / overlap_n; y < a.height(); y++) {
					mask[0](x, y) = 1;
				}
			}
		}
	}

	// Down sampling a and b, building Gaussian pyramids.
	for (int i = 1; i < n_level; i++) {
		a_pyramid[i] = a_pyramid[i - 1].get_blur(2).get_resize(a_pyramid[i - 1].width() / 2, a_pyramid[i - 1].height() / 2, 1, a_pyramid[i - 1].spectrum(), 3);
		b_pyramid[i] = b_pyramid[i - 1].get_blur(2).get_resize(b_pyramid[i - 1].width() / 2, b_pyramid[i - 1].height() / 2, 1, b_pyramid[i - 1].spectrum(), 3);

		mask[i] = mask[i - 1].get_blur(2).get_resize(mask[i - 1].width() / 2, mask[i - 1].height() / 2, 1, mask[i - 1].spectrum(), 3);
	}

	// Building Laplacian pyramids.
	for (int i = 0; i < n_level - 1; i++) {
		a_pyramid[i] = a_pyramid[i] - a_pyramid[i + 1].get_resize(a_pyramid[i].width(), a_pyramid[i].height(), 1, a_pyramid[i].spectrum(), 3);
		b_pyramid[i] = b_pyramid[i] - b_pyramid[i + 1].get_resize(b_pyramid[i].width(), b_pyramid[i].height(), 1, b_pyramid[i].spectrum(), 3);
	}

	vector<CImg<float>> blend_pyramid(n_level);

	for (int i = 0; i < n_level; i++) {
		blend_pyramid[i] = CImg<float>(a_pyramid[i].width(), a_pyramid[i].height(), 1, a_pyramid[i].spectrum(), 0);

		for (int x = 0; x < blend_pyramid[i].width(); x++) {
			for (int y = 0; y < blend_pyramid[i].height(); y++) {
				for (int k = 0; k < blend_pyramid[i].spectrum(); k++) {
					blend_pyramid[i](x, y, k) = a_pyramid[i](x, y, k) * mask[i](x, y) + b_pyramid[i](x, y, k) * (1.0 - mask[i](x, y));

				}
			}
		}
	}

	CImg<float> res = blend_pyramid[n_level - 1];
	for (int i = n_level - 2; i >= 0; i--) {
		res.resize(blend_pyramid[i].width(), blend_pyramid[i].height(), 1, blend_pyramid[i].spectrum(), 3);

		for (int x = 0; x < blend_pyramid[i].width(); x++) {
			for (int y = 0; y < blend_pyramid[i].height(); y++) {
				for (int k = 0; k < blend_pyramid[i].spectrum(); k++) {
					float t = res(x, y, k) + blend_pyramid[i](x, y, k);

					if (t > 255) {
						t = 255;
					}
					else if (t < 0) {
						t = 0;
					}

					res(x, y, k) = t;
				}
			}
		}
	}

	return res;
}

void PanoImageStitching::Crop(CImg<float>& stichedImg)
{
	int w = stichedImg._width;
	int h = stichedImg._height;

	int minX = 0;
	// 从上到小遍历，沿图像边缘查找
	for (int x = 1; x < w; x++) {
		minX++;
		bool flag = false;
		for (int y = 1; y < h; y++) {
			if (stichedImg(x, y, 0) == 255 && stichedImg(x, y, 1) == 255 && stichedImg(x, y, 2) == 255) {
				while (y != h) {
					if (stichedImg(x - 1, y + 1, 0) != 255 || stichedImg(x - 1, y + 1, 1) != 255 ||  stichedImg(x - 1, y + 1, 2) != 255) {
						x--;
						y++;
					}
					else if (stichedImg(x , y + 1, 0) != 255 || stichedImg(x , y + 1, 1) != 255 || stichedImg(x, y + 1, 2) != 255) {
						y++;
						continue;
					}
					else if (stichedImg(x + 1, y + 1, 0) != 255 || stichedImg(x + 1, y + 1, 1) != 255 || stichedImg(x + 1, y + 1, 2) != 255) {
						x++;
						y++;
					}
					else {

					}
				}
			}
		}
		if (flag) break;
	}

}


void PanoImageStitching::StartStitching()
{
	CImg<float> outputImage;
	srand(time(0));
	
	for (int i = 0; i < imageNum; i++) {
		imageGroup[i] = ImageToSphericalCoordinates(imageGroup[i]);
		if (DEBUG) cout << i << "图片转为球面坐标完成！" << endl;
		CImg<float> grayImage = ToGrayImage(imageGroup[i]);
		if (DEBUG) cout << i << "彩色图片生成黑色图片" << endl;
		//grayImage.display();
		siftFeaturePointsGroup.push_back(ExtractSIFTFeaturePoint(grayImage));
		if (DEBUG) cout << i << "提取图片特征点完成！" << endl;
	}
	cout << "------------" << endl;
	imageGroup[0].display("球面坐标");
	DisplayFeatureImage(siftFeaturePointsGroup[0], imageGroup[0]);

	for (int i = 0; i < imageNum; i++) {
		vector<bool> v;
		for (int j = 0; j < imageNum; j++) {
			v.push_back(false);
		}
		adjacentFlag.push_back(v);
	}

	vector<vector<int>> adjImg(imageNum);
	adjacentImages = adjImg;
	

	for (int i = 0; i < imageNum; i++) {
		for (int j = 0; j < imageNum; j++) {
			if (i == j) continue;
			if (DEBUG) cout << "knn搜索中两个特征点集的大小：" << siftFeaturePointsGroup[i].size() << " " << siftFeaturePointsGroup[j].size() << endl;
			vector<KeyPointPair> featurePointsPair = FindKNearestNeighbor(
				siftFeaturePointsGroup[i], siftFeaturePointsGroup[j]);
			if (DEBUG) cout << i << " " << j << " KNN算法进行特征匹配完成！" << endl;
			
			if (featurePointsPair.size() >= KEYPOINTS_PAIRS_THRESHOLD) {
				adjacentImages[i].push_back(j);
				//adjacentImages[j].push_back(i);
				adjacentFlag[i][j] = true;
				if (DEBUG) cout << imageFileNameGroup[i] << "  " << imageFileNameGroup[j] << " 存在邻接的可能。" << featurePointsPair.size() << " 个匹配对" << endl;
			}
		}
	}

	
	/*使用广度搜索，找到一个合适的邻接序列，然后拼接*/
	int startIndex = 0;
	queue<int> ToStitchImageIndex;  // 使用队列来管理未拼接的图像
	ToStitchImageIndex.push(startIndex);
	CImg<float> stitchedImage = imageGroup[startIndex];
	int prevIndex = startIndex;
	cout << "开始拼接" << endl;
	while (!ToStitchImageIndex.empty()) {
		int frontIndex = ToStitchImageIndex.front();
		ToStitchImageIndex.pop();

		// 遍历可能的邻接点
		for (int i = 0; i < adjacentImages[frontIndex].size(); i++) {
			int adjcentImageIndex = adjacentImages[frontIndex][i];
			// 先加入当前结点的子结点
			if (!adjacentFlag[frontIndex][adjcentImageIndex]) {
				continue;  // 拼接好了，跳过
			}
			else {
				adjacentFlag[frontIndex][adjcentImageIndex] = false;
				adjacentFlag[adjcentImageIndex][frontIndex] = false;
				// 加入队列
				ToStitchImageIndex.push(adjcentImageIndex);
			}

			// 计算前向变换和反向变换的特征点对
			vector<KeyPointPair>  src_to_dst_key_points = FindKNearestNeighbor(siftFeaturePointsGroup[frontIndex], siftFeaturePointsGroup[adjcentImageIndex]);
			vector<KeyPointPair>  dst_to_src_key_points = FindKNearestNeighbor(siftFeaturePointsGroup[adjcentImageIndex], siftFeaturePointsGroup[frontIndex]);
			
			/*
			vector<KeyPointPair> dst_to_src_key_points;
			for (auto it = src_to_dst_key_points.begin(); it != src_to_dst_key_points.end(); it++) {
				dst_to_src_key_points.push_back(KeyPointPair(src_to_dst_key_points[j].vp2, src_to_dst_key_points[j].vp1));
			}*/
			//cout << "前向变换特征点对大小" << src_to_dst_key_points.size() << endl;
			//cout << "反向变换特征点对大小" << dst_to_src_key_points.size() << endl;

			// 平衡前后变换的特征点对
			
			if (src_to_dst_key_points.size() > dst_to_src_key_points.size()) {
				dst_to_src_key_points.clear();
				for (int j = 0; j < src_to_dst_key_points.size(); j++) {
					KeyPointPair pairs(src_to_dst_key_points[j].vp2, src_to_dst_key_points[j].vp1);
					dst_to_src_key_points.push_back(pairs);
				}
			}
			else {
				src_to_dst_key_points.clear();
				for (int j = 0; j < dst_to_src_key_points.size(); j++) {
					KeyPointPair pairs(dst_to_src_key_points[j].vp2, dst_to_src_key_points[j].vp1);
					src_to_dst_key_points.push_back(pairs);
				}
			}
			//cout << "平衡后" << endl;
			//cout << "前向变换特征点对大小" << src_to_dst_key_points.size() << endl;
			//cout << "反向变换特征点对大小" << dst_to_src_key_points.size() << endl;


			// RANSAC算法计算前后两个变换的单应矩阵
			HomographyMatrix src_to_dst_matrix = RANSACForHomographyMatrix(src_to_dst_key_points);
			HomographyMatrix dst_to_src_matrix = RANSACForHomographyMatrix(dst_to_src_key_points);


			src_to_dst_matrix.print();
			dst_to_src_matrix.print();
		
			// 粗略计算拼接后的图像宽高信息
			CImg<float> lengthInfo = CalSizeOfStitchingImage(stitchedImage, imageGroup[adjcentImageIndex], dst_to_src_matrix);
			int outputX = lengthInfo(0, 0);
			int outputY = lengthInfo(0, 1);
			float minX = lengthInfo(1, 0);
			float minY = lengthInfo(1, 1);
			float maxX = lengthInfo(2, 0);
			float maxY = lengthInfo(2, 1);

			/*
			if (DEBUG) cout << "outputX " << outputX << " outputY " << outputY << endl;
			if (DEBUG) cout << "minX " << minX << " minY " << minY << endl;
			if (DEBUG) cout << "maxX " << maxX << " maxY " << maxY << endl;
			*/
			CImg<float> lastStitch(maxX, maxY, 1, 3, 255);
			CImg<float> nextStitch(maxX, maxY, 1, 3, 255);

			CImg<float> _adjcentImage = imageGroup[adjcentImageIndex];

			// 对要拼接的图片进行漂移，然后单应矩阵变换得到图片 nextstich
			WarpingImageWithHomography(_adjcentImage, nextStitch, src_to_dst_matrix, minX, minY);
			// 对拼接后的图片进行漂移 laststich
			ShiftImageByOffset(stitchedImage, lastStitch, minX, minY);
			// 更新特征点，修改刚拼接的图片特征点坐标
			WarpingFeaturePointWithHomography(siftFeaturePointsGroup[adjcentImageIndex], dst_to_src_matrix, minX, minY);
			// 更新特征点，修改当前拼接好的图片的特征点坐标
			ShifFeaturePointByOffset(siftFeaturePointsGroup[prevIndex], minX, minY);

			// 对两张图片进行多波段融合，完成拼接。
			lastStitch.display("被拼接图片");
			nextStitch.display("下一步要拼接的图片");

			cout << "Image " << imageFileNameGroup[adjcentImageIndex] << " has stitched to image " << imageFileNameGroup[prevIndex] << endl;
			stitchedImage = blendTwoImages(lastStitch, nextStitch);
			stitchedImage.display("拼接融合结果");
			// 修改当前拼接图的最后一块图片的索引
			prevIndex = adjcentImageIndex;
		}
	}
	stitchedImage.display("拼接后的图像");

	//Crop(stitchedImage);
}
