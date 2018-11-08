#include "HoughForCircle.h"

/*按文件名保存图片*/
void HoughForCircle::saveImage(string fileName, CImg<short int> img)
{
	if (DEBUG) cout << "保存图片" << endl;
	img.save(fileName.c_str());
}


/*************************************************************
* 函数描述：将边缘检测图中的边缘点存储起来，
			同时存储梯度方向值 ，减少非边缘点的检索
*			（空间换时间）
* 参数：edgeImage ---- canny检测后的二值化边缘图
*       edgePoint ---- 所有边缘点的向量存储结构
**************************************************************/
void HoughForCircle::saveEdgePoint(CImg<short int> edgeImage, vector<Point>* edgePointSet)
{
	if (DEBUG) cout << "存储边缘点" << endl;
	Point p;
	cimg_forXY(edgeImage, x, y) {
		if (edgeImage(x, y) != 0) {
			p.x = x;
			p.y = y;
			p.foundedFlag = false;
			p.gradientDirectionValue = detector.angle_radians(deltaXImage(x, y), deltaYImage(x, y));
			(*edgePointSet).push_back(p);
		}
	}
	//cout << "边缘点数目" << (*edgePointSet).size() << endl;
}


/********************************************
* 函数描述：判断三点形成的三角形面积是否为0，
*			为0，则共线，避免浮点运算造成误差
* 参数说明：三点(x1,x2) (y1,y2) (z1,z2)
********************************************/
bool HoughForCircle::judgeOnLine(int x1, int x2, int y1, int y2, int z1, int z2)
{
	if (DEBUG) cout << "判断三点共线" << endl;
	if(DEBUG) cout << x1 << " " << x2 << " " << y1 << " " << y2 << " " << z1 << " " << z2 << endl;
	if ((x1 * y2 - x2 * y1) + (y1 * z2 - y2 * z1) + (z1 * x2 - z2 * x1) == 0) {
		if (DEBUG) cout << "共线" << endl;
		return true;
	}
	if (DEBUG) cout << "不共线" << endl;
	return false;
}



/*****************************************************
* 函数描述：随机产生三个边缘点，利用边缘点计算其形成的
*           圆的参数，包括圆心位置（a,b),半径r
* 参数说明：begingVerifiedCircle--临时保存三个参数
* 选点的标准： 三个点之间的距离不能小于已确定的参数dmin，
*             不共线，
****************************************************/
void HoughForCircle::calculate_a_b_r(Circle* beingVerifiedCircle, vector<Point> edgePointSet, vector<Circle> haveVerifiedCircleSet)
{
	int width = edgeImage._width;
	int height = edgeImage._height;
	
	int size = edgePointSet.size();
	int extendNum = 0;
	if (size > RAND_MAX) {
		extendNum = size / RAND_MAX;
	}

	//todo  引入梯度筛选
	Point p1 = {}, p2 = {}, p3 = {};
	int pIndex1 = 0;
	int pIndex2 = 0;
	int pIndex3 = 0;
	bool pFlag1 = false;
	bool pFlag2 = false;
	bool pFlag3 = false;
	int k1 = 0, b1 = 0, k2 = 0, b2 = 0, k3 = 0, b3 = 0, centerLine_k = 0, centerLine_b = 0;
	while (1) {
		if (DEBUG) cout << "计算圆的参数" << endl;

		Point centerCircleDot;

		// 随机挑选几个点
		// 第一个点和第二个点同在一个循环里定位
		while (1) {
			//cout << "1" << endl;
			pIndex1 = (rand() % size) + RAND_MAX * (rand() % (extendNum + 1));
			if (!(pIndex1 < size && p1.foundedFlag == false))
				continue;
			p1.x = edgePointSet[pIndex1].x;
			p1.y = edgePointSet[pIndex1].y;
			p1.gradientDirectionValue = edgePointSet[pIndex1].gradientDirectionValue;
			p1.foundedFlag = edgePointSet[pIndex1].foundedFlag;
			
			while (1) {
				//cout << "2" << endl;
				pIndex2 = (rand() % size) + RAND_MAX * (rand() % (extendNum + 1));
				if (!(pIndex2 < size && p2.foundedFlag == false))
					continue;
				p2.x = edgePointSet[pIndex2].x;
				p2.y = edgePointSet[pIndex2].y;
				p2.gradientDirectionValue = edgePointSet[pIndex2].gradientDirectionValue;
				p2.foundedFlag = edgePointSet[pIndex2].foundedFlag;
				if (p2.x == p1.x && p1.y == p2.y)
					continue;
				break;
			}
			
			// 在这里判断第一个点和第二个点的梯度方向上的直线是否能与圆的中垂线交于一点，不行，则重选两个点
			k1 = p1.gradientDirectionValue;
			b1 = p1.y - k1 * p1.x;

			k2 = p2.gradientDirectionValue;
			b2 = p2.y - k2 * p2.x;
			
			centerLine_k = (p2.x - p1.x) * 1.0 / (p2.y - p1.y);
			centerLine_b = (p1.y + p2.y) * 1.0 / 2 + (p1.x * p1.x - p2.x + p2.x) * 1.0 / (2 * (p2.y - p1.y));

			if (k1 == centerLine_k || k2 == centerLine_k) {
				// 平行。。。重选
				continue;
			}
			int x_1_k = (centerLine_b - b1) / (k1 - centerLine_k);
			int y_1_k = k1 * x_1_k + b1;

			int x_2_k = (centerLine_b - b2) / (k2 - centerLine_k);
			int y_2_k = k1 * x_2_k + b2;

			// 判断两个焦点距离是否满足阈值条件，满足则成功找到1，2点
			if ((x_1_k - x_2_k) * (x_1_k - x_2_k) + (y_2_k - y_1_k)*(y_2_k - y_1_k) <= circleCenterDistanceThreshold * circleCenterDistanceThreshold) {
				centerCircleDot.x = (x_1_k + x_2_k) / 2;
				centerCircleDot.y = (y_1_k + y_2_k) / 2;
				break;
			}
			
		}
		
		// 第3个点
		while (1) {
			//cout << "3" << endl;

			pIndex3 = (rand() % size) + RAND_MAX * (rand() % (extendNum + 1));
			if (!(pIndex3 < size && p3.foundedFlag == false))
				continue;

			
			// 判断第三点梯度方向所在直线是否与1，2点形成的圆形相交
			p3.gradientDirectionValue = edgePointSet[pIndex3].gradientDirectionValue;
			p3.foundedFlag = edgePointSet[pIndex3].foundedFlag;
			p3.x = edgePointSet[pIndex3].x;
			p3.y = edgePointSet[pIndex3].y; 
			
			// 三点共线，重选
			if (judgeOnLine(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y)) {
				continue;
			}


			k3 = p3.gradientDirectionValue;
			b3 = p3.y - k3 * p3.x;

			k2 = p2.gradientDirectionValue;
			b2 = p2.y - k2 * p2.x;

			centerLine_k = (p2.x - p3.x) * 3.0 / (p2.y - p3.y);
			centerLine_b = (p3.y + p2.y) * 3.0 / 2 + (p3.x * p3.x - p2.x + p2.x) * 3.0 / (2 * (p2.y - p3.y));

			if (k3 == centerLine_k || k2 == centerLine_k) {
				// 平行。。。重选
				continue;
			}
			int x_3_k = (centerLine_b - b3) / (k3 - centerLine_k);
			int y_3_k = k3 * x_3_k + b3;

			int x_2_k = (centerLine_b - b2) / (k2 - centerLine_k);
			int y_2_k = k3 * x_2_k + b2;

			// 判断两个焦点距离是否满足阈值条件，满足则成功找到3点
			if ((x_3_k - x_2_k) * (x_3_k - x_2_k) + (y_2_k - y_3_k)*(y_2_k - y_3_k) <= circleCenterDistanceThreshold * circleCenterDistanceThreshold) {
				centerCircleDot.x = (x_3_k + x_2_k) / 2;
				centerCircleDot.y = (y_3_k + y_2_k) / 2;
			//	cout << "3点  ok" << endl;
				break;
			}
		}

		// 挑选出不共线的三点,计算参数
		int t1 = p2.x * p2.x + p2.y * p2.y - (p1.x * p1.x + p1.y * p1.y);
		int t2 = 2 * (p2.y - p1.y);
		int t3 = p3.x * p3.x + p3.y * p3.y - (p1.x * p1.x + p1.y * p1.y);
		int t4 = 2 * (p3.y - p1.y);
		int t5 = (p2.x - p1.x)*(p3.y - p1.y) - (p3.x - p1.x)*(p2.y - p1.y);
		int t6 = 2 * (p2.x - p1.x);
		int t7 = 2 * (p3.y - p1.y);

		int a = (int)((t1 * t4 - t2 * t3) * 1.0 / (4 * t5)); 
		int b = (int)((t6 * t3 - t7 * t1) * 1.0 / (4 * t5));
	    int r = (int)(0.5 + sqrt((p1.x - beingVerifiedCircle->a)*(p1.x - beingVerifiedCircle->a) +
			(p1.y - beingVerifiedCircle->b)*(p1.y - beingVerifiedCircle->b)));
		

		if (a < 0 || a > width || b  < 0 || b > height) {
			if (DEBUG) {
				cout << "剔除圆心在图像外的情况  " << endl;
				cout << a << " " << b << " " << r << endl;
			}
			continue;

		}
		cout << a << " " << b << " " << r << endl;

		if (r < 10)
			continue;

		beingVerifiedCircle->a = a;
		beingVerifiedCircle->b = b;
		beingVerifiedCircle->r = r;
	//	cout << a << "---- " << b << " ----" << r << endl;
		if (!chooseCircle(*beingVerifiedCircle, haveVerifiedCircleSet)) {
			/*得到不相似的圆或者得到相似的圆次数超过上限upperLimitOfSimiliarCircle*/
			if (DEBUG) cout << "找到合适的随机圆，进入投票环节" << endl;
			break;
		}
	}
	
}

/***************************************************************
* 函数描述：判断计算得到的临时圆是否与已检测的圆相交，
*			相交返回真，阻止进一步投票，然后开始重新找下一个圆
*参数说明：
***************************************************************/
bool HoughForCircle::chooseCircle(Circle begingVerifiedCircle, vector<Circle> haveVerifiedCircleSet)
{
	if (DEBUG) cout << "判断随机生成的圆是否与已确定好的圆相似,或相交" << endl;
	return  false;
	int a = begingVerifiedCircle.a;
	int b = begingVerifiedCircle.b;
	int r = begingVerifiedCircle.r;
	for (int i = 0; i < haveVerifiedCircleSet.size(); i++) {
		int A = haveVerifiedCircleSet[i].a;
		int B = haveVerifiedCircleSet[i].b;
		int R = haveVerifiedCircleSet[i].r;

		if ((a-A) * (a - A) + (b - B)*(b - B) < (r + R) * (r + R)) {
			//cout << "两圆相交" << endl;
			return true;
		}
			
		
	}
	return false;
}



void HoughForCircle::vote(int voteNumThreshold, vector<Point> edgePointSet, Circle* beingVerifiedCircle, vector<Circle> haveVerifiedCircle)
{
	
	if (DEBUG) cout << "开始投票" << endl;
	int a = (*beingVerifiedCircle).a;
	int b = (*beingVerifiedCircle).b;
	int r = (*beingVerifiedCircle).r;
	int t = 2;
	int voteNum = 0;
	vector<int> voteFlag;
	for (int i = 0; i < edgePointSet.size(); i++) {
		int x = edgePointSet[i].x;
		int y = edgePointSet[i].y;
		if (edgePointSet[i].foundedFlag == true)
			continue;

		if (x > a + r + t || x < a - r - t || y > b + r + t || y < b - r - t) {
			// 点在圆的外接正方形外，跳过
			continue;
		}
		else {
			if ((r - edgeWidth) * (r - edgeWidth) < (x - a)*(x - a) + (y - b)*(y - b) &&
				(r + edgeWidth) * (r + edgeWidth) > (x - a)*(x - a) + (y - b)*(y - b)) {
				voteFlag.push_back(i);
				voteNum++;  //计票
			}
		}
	}
	voteNumThreshold = (int)( 2.0 * PI * r *voteNumThresholdRate);

	//cout << " a: " << a << "  b: " << b << " r: " << r << endl
		//<<" voteNumThreshold:" << voteNumThreshold << " voteNum: " << voteNum << endl;
	if (voteNum >= voteNumThreshold) {  //该圆符合条件
		if (DEBUG) cout << "该圆符合条件-------------------------------------------" << endl;
		haveVerifiedCircleSet.push_back(*beingVerifiedCircle);
		for (int i = 0; i < voteFlag.size(); i++) {
			edgePointSet[i].foundedFlag = true;
		}
		return;
	}

	if (DEBUG) cout << "该圆不符合条件" << endl;
}

/*根据圆参数集合输出方程*/
void HoughForCircle::inputCircleEquation(vector<Circle> haveVerifiedCircleSet)
{
	if (DEBUG) cout << "输出圆方程" << endl;
	for (int i = 0; i < haveVerifiedCircleSet.size(); i++) {
		cout << "(x - " << haveVerifiedCircleSet[i].a << ")^2 + (y - " << haveVerifiedCircleSet[i].b << " )^2 = "
			<< haveVerifiedCircleSet[i].r * haveVerifiedCircleSet[i].r << endl;
	}
}

void HoughForCircle::drawCircleByEquation(vector<Circle> haveVerifiedCircleSet, CImg<short int>* houghImage)
{
	if (DEBUG) cout << "根据方程绘制图像" << endl;
	unsigned char blue[] = { 0,0,255 };
	for (int i = 0; i < haveVerifiedCircleSet.size(); i++) {
		int a = haveVerifiedCircleSet[i].a;
		int b = haveVerifiedCircleSet[i].b;
		int r = haveVerifiedCircleSet[i].r;
		cimg_forXY(*houghImage, x, y) {
			if ((x - a)*(x - a) + (y - b)*(y - b) < (r + edgeWidth)*(r + edgeWidth)
				&& (x - a)*(x - a) + (y - b)*(y - b) > (r - edgeWidth)*(r - edgeWidth)) {
				(*houghImage)(x, y, 0) = 0;
				(*houghImage)(x, y, 1) = 0;
				(*houghImage)(x, y, 2) = 255;
			}
		}
	}
	

}

void HoughForCircle::drawCircleEdge(vector<Point> edgePointSet, vector<Circle> haveVerifiedCircleSet, CImg<short int>* houghImage)
{
	if (DEBUG) cout << "画出圆的边缘点" << endl;
	for (int i = 0; i < edgePointSet.size(); i++) {
		int x = edgePointSet[i].x;
		int y = edgePointSet[i].y;
		if ((*houghImage)(x, y, 2) == 255) {
			(*houghImage)(x, y, 0) = 255;
			(*houghImage)(x, y, 2) = 0;
		}
	}
}


void HoughForCircle::hough(string fileName)
{
	clock_t startTime = clock();
	if (DEBUG) cout << "开始霍夫变换" << endl;

	/*加载图像*/
	detector.canny(fileName);
	//edgeImage = *detector.getNonMaxSuppImage();
	edgeImage = *detector.getCutShortEdgeImage();
	string cannyFileName = "canny_" + fileName;
	saveImage(cannyFileName.c_str(), edgeImage);
	houghImage.load_bmp(fileName.c_str());
	deltaXImage = *detector.getDeltaXImage();
	deltaYImage = *detector.getDeltaYImage();

	/*保存边缘点*/
	saveEdgePoint(edgeImage, &edgePointSet);
	stopFlag = 0;
	
	height = edgeImage._height;
	width = edgeImage._width;
	maxTimes = maxTimesRate * (height / 3 + width / 3 + edgePointSet.size()/100);

	//cout << "边缘点数目 " << edgePointSet.size() << " h " << height << " w" << width << endl;

	for(int i = 0; i < maxTimes; i++){ // 终止条件是，连续产生许多个已有的圆的参数
		/*计算得到合适的圆的参数 a，b，r*/
		calculate_a_b_r(&beingVerifiedCircle, edgePointSet, haveVerifiedCircleSet);
		/*执行投票*/
		vote(this->voteNumThreshold, edgePointSet, &beingVerifiedCircle, haveVerifiedCircleSet);
	}
	
	/*输出圆方程*/
	inputCircleEquation(this->haveVerifiedCircleSet);

	/*绘制圆*/
	drawCircleByEquation(this->haveVerifiedCircleSet, &houghImage);

	/*绘制圆的边缘点*/
	drawCircleEdge(edgePointSet, haveVerifiedCircleSet, &houghImage);

	string houghFileName = "hough_" + fileName;
	saveImage(houghFileName.c_str(), houghImage);
	houghImage.display();
	cout << "据检测，图中硬币个数为 " << haveVerifiedCircleSet.size() << endl;
	clock_t endTime = clock();
	cout << "总共花费时间为 " << (double)(endTime - startTime)/CLOCKS_PER_SEC << " s"<< endl;
}
