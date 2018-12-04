#pragma once
/****************************************************************

Author: qiuyh

Creating Date: 18/11/27

Description: The main class of stitching pannorama images.

***************************************************************/
#include "Utils.h"
#include <set>
#include <vector>
#include <queue>


#include "vl/generic.h"
#include "vl/sift.h"
#include "vl/kdtree.h"

#define NUM_OF_PAIR 4
#define CONFIDENCE 0.99
#define INLIER_RATIO 0.5
#define RANSAC_THRESHOLD 4.0
#define KEYPOINTS_PAIRS_THRESHOLD 60

// 存储不同图片之间的特征点匹配对
struct KeyPointPair {
	VlSiftKeypoint vp1;
	VlSiftKeypoint vp2;
	KeyPointPair(VlSiftKeypoint _v1, VlSiftKeypoint _v2) :vp1(_v1), vp2(_v2) {}
};

// 传入特征点和图片，输出图片
void DisplayFeatureImage(map<vector<float>, VlSiftKeypoint> pointSet, CImg<float> img);

class PanoImageStitching
{
	vector<string> imageFileNameGroup;
	vector<CImg<float>> imageGroup;   /*待拼接的图像组*/
	int imageNum;                     /*待拼接的图像数目*/

	vector<map<vector<float>, VlSiftKeypoint>> siftFeaturePointsGroup; /*多个图像的特征点集合*/

	vector<vector<int>> adjacentImages;   /*根据KNN确定图片拼合顺序*/
	vector<vector<bool>> adjacentFlag;     /*拼合位置的标志*/

public:
	/*1. 该函数用于将所有的文件路径读取到一个图像组中*/
	void ReadAllImage(vector<string> imageFilePath);

	/*2. 将图像转换为球面坐标*/
	CImg<float> ImageToSphericalCoordinates(CImg<float> image);

	/*3. 将彩色图像转为RGB图像*/
	CImg<float> ToGrayImage(CImg<float> & image);

	/*4. 根据图像提取特征点,并将特征点返回存储*/
	map<vector<float>, VlSiftKeypoint> ExtractSIFTFeaturePoint(CImg<float> grayImage);

	/*5. 传入两张图片的特征点集合，利用kd树返回匹配的特征点对数*/
	vector<KeyPointPair> FindKNearestNeighbor(map<vector<float>, VlSiftKeypoint> &featurePoint1, 
		map<vector<float>, VlSiftKeypoint> &featurePoint2);

	/*6.  从给定的特征点集合中确定单应矩阵*/
	HomographyMatrix GetHomographyFromPointPairs(const vector<KeyPointPair> &pair);

	/*7. 计算可信点的数目*/
	vector<int> ComputeInliers(HomographyMatrix HMatrix, vector<KeyPointPair> pair, set<int> randomPairIndex);

	/*8. 重新计算所有可信点的最小二乘的单应矩阵*/
	HomographyMatrix ComputeLeastSquaresHomographyMatrix(vector<KeyPointPair> pairs, vector<int> inlier_Indexs);

	/*7. 对特征点对进行RANSAC，并得到单应矩阵*/
	HomographyMatrix RANSACForHomographyMatrix(vector<KeyPointPair> featurePointPair);

	/*8. 对RANSAC的结果建立概率模型，进行数学验证，去除一些无效的RANSAC结果*/
	bool MathVerification(HomographyMatrix HMatrix);

	/*9. 计算图像拼接后的长度宽度,变换后图像的最大最小宽度*/
	CImg<float> CalSizeOfStitchingImage(CImg<float> stitchedImage, CImg<float> adjacentImage, 
		HomographyMatrix transformMatrix);
	
	/*10. 两张图片漂移, 利用偏移量x， y，将拼接好的图片替换到下一张图片中*/
	void ShiftImageByOffset(CImg<float> &stitchedImg, CImg<float> &dstImg, int offsetX, int offsetY);

	/*11. 利用单应矩阵进行图像扭曲*/
	void WarpingImageWithHomography(CImg<float> &stitchedImg, CImg<float> &dstImage, 
		HomographyMatrix H, float offsetX, float offsetY);

	/*12. 对一对图片的特征点对集合进行漂移，图像拼接之后特征点坐标需要进行更新*/
	void ShifFeaturePointByOffset(map<vector<float>, VlSiftKeypoint> &featurePointSet,
		int offsetX, int offsetY);

	/*13. 利用单应矩阵对特征点的坐标进行扭曲*/
	void WarpingFeaturePointWithHomography(map<vector<float>, VlSiftKeypoint> &featurePointSet, 
		HomographyMatrix H, int offsetX, int offsetY);

	/*对每次连接进行束调整 ，不会。。*/
	void BundleAdjustment();

	/*矫正  */
	void Straightening();

	/*补偿*/
	void GainCompensation();

	/*.对两张相似图片进行多频段融合*/
	CImg<float> blendTwoImages(const CImg<float> &a, const CImg<float> &b);

	/*. 根据最大内接矩形裁剪*/
	void Crop(CImg<float> &stichedImg);

	/* 拼接总控函数*/
	void StartStitching();
};