#include "MorphingWithLocalWarping.h"

// 判断p点是否在三角形内部
bool MorphingWithLocalWarping::Triangle::isInTriangle(const Point & p)
{
	// 向量叉积和都为正或者都为负，那么点p就在三角形内部
	if (cross3(a, b, p) >= 0 && cross3(b, c, p) >= 0 && cross3(c, a, p) >= 0)
		return true;
	else if (cross3(a, b, p) <= 0 && cross3(b, c, p) <= 0 && cross3(c, a, p) <= 0)
		return true;
	else
		return false;
}

// 判断点是否在三角形外接圆外
bool MorphingWithLocalWarping::Triangle::isInOuterCircle(const Point & p)
{
	Point origin(0, 0);
	double a_x = this->a.x;
	double b_x = this->b.x;
	double c_x = this->c.x;
	double a_y = this->a.y;
	double b_y = this->b.y;
	double c_y = this->c.y;
	double D = 2 * (a_x*(b_y - c_y) + b_x * (c_y - a_y) + c_x * (a_y - b_y));
	double x = ((a_x*a_x + a_y * a_y)*(b_y - c_y) + (b_x*b_x + b_y * b_y)*(c_y - a_y) + (c_x*c_x + c_y * c_y)*(a_y - b_y)) / D;
	double y = ((a_x*a_x + a_y * a_y)*(c_x - b_x) + (b_x*b_x + b_y * b_y)*(a_x - c_x) + (c_x*c_x + c_y * c_y)*(b_x - a_x)) / D;
	Point center((int)x, (int)y);
	double radius = GetDistance(center, this->a);
	return (this->GetDistance(p, center) <= radius);
}


/*双线性插值*/
double MorphingWithLocalWarping::BilinearFilter(CImg<double> Image, double x, double y, int channel)
{
	int floor_x = floor(x);
	int floor_y = floor(y);
	int ceil_x = ceil(x);
	int ceil_y = ceil(y);
	/*
	+----tmp1----+
	|            | 
	|     *P     |
	|            |
	|            |
	+----tmp2----+
	*/
	double tmp1 = (ceil_x - x)*Image(ceil_x, floor_y, channel) + (x - floor_x) * Image(floor_x, floor_y, channel);
	double tmp2 = (ceil_x - x)*Image(ceil_x, ceil_y, channel) + (x - floor_x) * Image(floor_x, ceil_y, channel);
	double P = (ceil_y - y) * tmp1 + (y - floor_y) * tmp2;
	return P;
}

// 读取文件中的特征点坐标，显示，若文件中没有特征点坐标
// 则打开窗口，通过鼠标点击捕获特征点坐标，写入文件中。
CImg<double> MorphingWithLocalWarping::GetFeaturePoints(CImg<double> Image,
	vector<Point*>* PointSet, string PointsSetFilePath)
{
	if (Debug) cout << "获取特征点" << endl;
	if (PointSet->size() != 0) {
		PointSet->clear();
	}
	string PointsFile = PointsSetFilePath;
	CImg<double> DetectedImage = Image;
	int BLUE[3] = { 0, 0, 255 };
	int width = Image._width;
	int height = Image._height;
	cout << width << " ad " << height << endl;
	ifstream InputPoints(PointsSetFilePath);
	ofstream OutputPoints;

	if (InputPoints.fail()) {  //不存在该文件，则创建文件并打开窗口获取特征点
		OutputPoints.open(PointsFile);  // output some points into file
		CImgDisplay showImageWindow(DetectedImage, PointsSetFilePath.c_str());  // 显示窗口
		while (!showImageWindow.is_closed()) {  // 当窗口关闭后，循环终止
			showImageWindow.wait();  //等待鼠标点击特征像素点
			if (showImageWindow.button() & 1 && showImageWindow.mouse_y() >= 0) {
				// 显示并存储鼠标点击的特征像素点
				Point* p = new Point(showImageWindow.mouse_x(), showImageWindow.mouse_y());
				PointSet->push_back(p);
				DetectedImage.draw_circle(p->x, p->y, 5, BLUE);
				DetectedImage.display(showImageWindow);
				// 像素数据写入文件
				OutputPoints << p->x << "," << p->y << endl;
				cout << "click" << endl;
			}
		}
		OutputPoints.close();
	}
	else {
		string Line;
		while (getline(InputPoints, Line)) {
			//每一行的格式都为   x,y
			int pos = Line.find(',');
			if (pos != string::npos) {
				string x_str = Line.substr(0, pos);
				string y_str = Line.substr(pos + 1, string::npos);
				Point* p = new Point(stoi(x_str), stoi(y_str));
				PointSet->push_back(p);
				cout << "point" << endl;
			//	DetectedImage.draw_circle(p->x, p->y, 5, BLUE);
			}
		}
	}
	
	PointSet->push_back(new Point(0, 0));
	PointSet->push_back(new Point(width/2, 0));
	PointSet->push_back(new Point(width, 0));
	PointSet->push_back(new Point(0,height/2));
	PointSet->push_back(new Point(width,height/2));
	PointSet->push_back(new Point(0,height));
	PointSet->push_back(new Point(width / 2,height));
	PointSet->push_back(new Point(width,height));

	InputPoints.close();
	if (Debug) cout << "获取特征点结束" << endl;
	return DetectedImage;
}


void MorphingWithLocalWarping::CalculateTriangles(
	vector<Point*>* SrcPoints, vector<Triangle*>* SrcTriangles,
	vector<Point*>* DstPoints, vector<Triangle*>* DstTriangles)
{
	if (Debug) cout << "三角剖分" << endl;
	if (SrcTriangles->size() != 0)
		SrcTriangles->clear();
	
	int size = SrcPoints->size();
	if (size < 3) {
		cout << "关键点小于三个，无法绘制三角形" << endl;
		return;
	}
	cout << size << " 关键点数目 " << endl;
	// 先生成源图像的三角形网格，然后根据对应关系直接生成目标图像的三角形网格
	for (int i = 0; i < size - 2; i++) {
		for (int j = i + 1; j < size - 1; j++) {
			for (int k = j + 1; k < size; k++) {
				//三重循环，穷举所有的三点组合
				Point* A = (*SrcPoints)[i];
				Point* B = (*SrcPoints)[j];
				Point* C = (*SrcPoints)[k];

				Triangle* tmpTri = new Triangle(*A, *B, *C, i, j, k);
				bool NoExit = true;
				// 找到一个三角形，判断有无点在其外接圆内
				for (int m = 0; m < size; m++) {
					Point* p = (*SrcPoints)[m];
					if (m == i || m == j || m == k) continue;
					double k1 = (double)(A->y - B->y) / (A->x - B->x);
					double k2 = (double)(A->y - C->y) / (A->x - C->x);
					if (tmpTri->isInOuterCircle(*p) || abs(k1 - k2) < 0.1) {
						// abs to avoid to closer points
						NoExit = false;
						break;
					}
				}
				if (NoExit) {
					SrcTriangles->push_back(tmpTri);
				}
			}
		}
	}
	for (int i = 0; i < SrcTriangles->size(); i++) {
		Triangle* DstTri = new Triangle(
			*DstPoints->at(SrcTriangles->at(i)->index[0]),
			*DstPoints->at(SrcTriangles->at(i)->index[1]),
			*DstPoints->at(SrcTriangles->at(i)->index[2]),
			SrcTriangles->at(i)->index[0],
			SrcTriangles->at(i)->index[1],
			SrcTriangles->at(i)->index[2]
		);
		DstTriangles->push_back(DstTri);
	}


	if (Debug) cout << "结束三角剖分" << endl;
}

void MorphingWithLocalWarping::DrawPoints(CImg<double>* Image, vector<Point*> PointSet)
{
	if (Debug) cout << "画特征点" << endl;
	int Blue[] = { 0, 0, 255 };
	for (int i = 0; i < PointSet.size(); i++) {
		(*Image).draw_circle(PointSet[i]->x, PointSet[i]->y, 2, Blue, 1);
	}
	(*Image).display();
}

void MorphingWithLocalWarping::DrawTriangles(CImg<double>* Image, vector <Triangle*> TriangleSet)
{
	if (Debug) cout << "画三角形网格" << endl;
	int Blue[] = { 0, 0, 255 };
	for (int i = 0; i < TriangleSet.size(); i++) {
		(*Image).draw_line(TriangleSet[i]->a.x, TriangleSet[i]->a.y, TriangleSet[i]->b.x, TriangleSet[i]->b.y, Blue);
		(*Image).draw_line(TriangleSet[i]->a.x, TriangleSet[i]->a.y, TriangleSet[i]->c.x, TriangleSet[i]->c.y, Blue);
		(*Image).draw_line(TriangleSet[i]->c.x, TriangleSet[i]->c.y, TriangleSet[i]->b.x, TriangleSet[i]->b.y, Blue);
	}
	(*Image).display();
}

// 每一对三角形之间存在一个变换3*3矩阵B  B * src = dst
vector<vector<double>>* MorphingWithLocalWarping::GetTransformMatrixOfTriangle
(const Triangle * SrcTriangle, const Triangle * DstTriangle, vector<vector<double>>* matrix)
{

	// transform src to dst
		
		int u0 = SrcTriangle->a.x;
		int v0 = SrcTriangle->a.y;
		int u1 = SrcTriangle->b.x;
		int v1 = SrcTriangle->b.y;
		int u2 = SrcTriangle->c.x;
		int v2 = SrcTriangle->c.y;
		int x0 = DstTriangle->a.x;
		int y0 = DstTriangle->a.y;
		int x1 = DstTriangle->b.x;
		int y1 = DstTriangle->b.y;
		int x2 = DstTriangle->c.x;
		int y2 = DstTriangle->c.y;
		// |A|
		int detA;
		detA = u0 * v1 + u1 * v2 + u2 * v0 - u2 * v1 - u0 * v2 - u1 * v0;
		// inv(A) * detA
		int A11, A12, A13, A21, A22, A23, A31, A32, A33;
		A11 = v1 - v2;
		A21 = -(v0 - v2);
		A31 = v0 - v1;
		A12 = -(u1 - u2);
		A22 = u0 - u2;
		A32 = -(u0 - u1);
		A13 = u1 * v2 - u2 * v1;
		A23 = -(u0*v2 - u2 * v0);
		A33 = u0 * v1 - u1 * v0;
		vector<double> row1;
		vector<double> row2;
		vector<double> row3;
		row1.push_back((double)(x0*A11 + x1 * A21 + x2 * A31) / detA);
		row1.push_back((double)(x0*A12 + x1 * A22 + x2 * A32) / detA);
		row1.push_back((double)(x0*A13 + x1 * A23 + x2 * A33) / detA);
		row2.push_back((double)(y0*A11 + y1 * A21 + y2 * A31) / detA);
		row2.push_back((double)(y0*A12 + y1 * A22 + y2 * A32) / detA);
		row2.push_back((double)(y0*A13 + y1 * A23 + y2 * A33) / detA);
		row3.push_back((double)(A11 + A21 + A31) / detA);
		row3.push_back((double)(A12 + A22 + A32) / detA);
		row3.push_back((double)(A13 + A23 + A33) / detA);
		matrix->push_back(row1);
		matrix->push_back(row2);
		matrix->push_back(row3);
		return matrix;
		
}


// 根据比例得到中间图像的三角形坐标
MorphingWithLocalWarping::Triangle* MorphingWithLocalWarping::GetTransitionOnTriangle(Triangle* SrcTriangle, 
	Triangle* DstTriangle, float ratio, Triangle* MorphTriangle)
{
	int a_x = (int)((1 - ratio) * SrcTriangle->a.x + ratio * DstTriangle->a.x);
	int a_y = (int)((1 - ratio) * SrcTriangle->a.y + ratio * DstTriangle->a.y);
	int b_x = (int)((1 - ratio) * SrcTriangle->b.x + ratio * DstTriangle->b.x);
	int b_y = (int)((1 - ratio) * SrcTriangle->b.y + ratio * DstTriangle->b.y);
	int c_x = (int)((1 - ratio) * SrcTriangle->c.x + ratio * DstTriangle->c.x);
	int c_y = (int)((1 - ratio) * SrcTriangle->c.y + ratio * DstTriangle->c.y);
	MorphTriangle->a = Point(a_x, a_y);
	MorphTriangle->b = Point(b_x, b_y);
	MorphTriangle->c = Point(c_x, c_y);
	return MorphTriangle;
}


//对两张图片进行加权 result = (1 - ratio) * src + ratio * dst
CImg<double>* MorphingWithLocalWarping::MorphingTriangleSets(const vector<Triangle*> SrcTriangleSet, const vector<Triangle*> DstTriangleSet,
	CImg<double>* SrcImage, CImg<double>* DstImage, CImg<double>* MorphingImage, double ratio)
{	
	//int width = (1 - ratio) * SrcImage->_width + ratio * DstImage->_width;
	//int height = (1 - ratio) * SrcImage->_height + ratio * DstImage->_height;
	//CImg<double> result(width, height, 1, 3);
	//(*MorphingImage) = result;
	if (Debug) cout << "逐帧morphing" << endl;

	double blue[3] = { 0,0,255 };
	double red[3] = { 255, 0,0 };
	for (int i = 0; i < SrcTriangleSet.size(); i++) {
		// 循环遍历三角形对
		//得到中间过渡变换的三角形
		cout << i << ".." << endl;
		Triangle* middleTriangle = new Triangle(Point(0,0), Point(0,0), Point(0,0), 0,0,0);
		GetTransitionOnTriangle(SrcTriangleSet[i], DstTriangleSet[i], ratio, middleTriangle);
		
		//求源三角形到过渡三角形的一个 变换矩阵
		vector<vector<double>> Src2MiddleMatrix;
		Src2MiddleMatrix = *GetTransformMatrixOfTriangle(middleTriangle, SrcTriangleSet[i], &Src2MiddleMatrix);
		// 求目标三角形到过渡三角形的一个变换矩阵
		vector<vector<double>> Dst2MiddleMatrix;
		Dst2MiddleMatrix = *GetTransformMatrixOfTriangle(middleTriangle, DstTriangleSet[i], &Dst2MiddleMatrix);
		// 遍历中间的图像morphing image
		//CImg<float> Src2MiddleMatrix = GetTransformMatrixOfTriangle(SrcTriangleSet[i], middleTriangle);
		//CImg<float> Dst2MiddleMatrix = GetTransformMatrixOfTriangle(DstTriangleSet[i], middleTriangle);
		CImg<double> tmpSrc = (*MorphingImage);
		CImg<double> tmpDst = (*MorphingImage);
		cimg_forXY(*MorphingImage, x, y) {
			// 找到过渡矩阵中的像素点
			if (middleTriangle->isInTriangle(Point(x, y))) {
		
				//从中间三角形反向warphing，应用插值求出该点的像素值
				//double TmpX_a = double(x* Src2MiddleMatrix(0,0)+ y * Src2MiddleMatrix(1,0) + Src2MiddleMatrix(2,0));
				//double TmpY_a = double(x*Src2MiddleMatrix(1,0) + y * Src2MiddleMatrix(1,1) + Src2MiddleMatrix(2,1));
				double TmpX_a = double(x* (Src2MiddleMatrix)[0][0] + y * (Src2MiddleMatrix)[0][1] + (Src2MiddleMatrix)[0][2]);
				double TmpY_a = double(x*(Src2MiddleMatrix)[1][0] + y * (Src2MiddleMatrix)[1][1] + (Src2MiddleMatrix)[1][2]);
				double Pixel_a[3] = { 0 };
				cimg_forC(*SrcImage, c) {
					// 遍历通道进行独立插值
					Pixel_a[c] = (*SrcImage).linear_atXY(TmpX_a, TmpY_a, c);
					//tmpSrc(TmpX_a, TmpY_a, c) = (*SrcImage).linear_atXY(TmpX_a, TmpY_a, c);
				//	Pixel_a[c] = BilinearFilter((*SrcImage), TmpX_a, TmpY_a, c);
				}
				//double TmpX_b = double(x* Dst2MiddleMatrix(0, 0) + y * Dst2MiddleMatrix(1, 0) + Dst2MiddleMatrix(2, 0));
				//double TmpY_b = double(x* Dst2MiddleMatrix(1, 0) + y * Dst2MiddleMatrix(1, 1) + Dst2MiddleMatrix(2, 1));
				double TmpX_b = double(x* (Dst2MiddleMatrix)[0][0] + y * (Dst2MiddleMatrix)[0][1] + (Dst2MiddleMatrix)[0][2]);
				double TmpY_b = double(x*(Dst2MiddleMatrix)[1][0] + y * (Dst2MiddleMatrix)[1][1] + (Dst2MiddleMatrix)[1][2]);

				double Pixel_b[3] = { 0 };
				cimg_forC(*DstImage, c) {
					//双线性插值
					//Pixel_b[c] = BilinearFilter((*DstImage), TmpX_b, TmpY_b, c);
					Pixel_b[c] = (*DstImage).linear_atXY(TmpX_b, TmpY_b, c);
					//tmpDst(TmpX_b, TmpY_b, c) = (*DstImage).linear_atXY(TmpX_b, TmpY_b, c);
				}
				//morphing
				
				cimg_forC(*MorphingImage, c) {
					(*MorphingImage)(x, y, 0, c) = (1 - ratio) * Pixel_a[c] + ratio * Pixel_b[c];
				}
				
			}
		}
		
	
	}
	
	//(*MorphingImage).display();
	return MorphingImage;
}

//总控函数，执行整个过程
void MorphingWithLocalWarping::StartMorphing()
{
	// 获得src特征点
	GetFeaturePoints(SrcImage, &SrcPointSet, SrcPointFilePath);
	// 画出src特征点,显示
	DrawPoints(&SrcMeshImage, SrcPointSet);
	// 获得dst特征点
	GetFeaturePoints(DstImage, &DstPointSet, DstPointFilePath);
	// 画出dst特征点，显示
	DrawPoints(&DstMeshImage, DstPointSet);
	// 求出dst, src三角形网格
	CalculateTriangles(&SrcPointSet, &SrcTriangleSet, &DstPointSet, &DstTriangleSet);
	// 画出src网格
	DrawTriangles(&SrcMeshImage, SrcTriangleSet);
	// 画出dst网格
	DrawTriangles(&DstMeshImage, DstTriangleSet);
	// 对每一对src，dst三角形进行morphing
	// 默认11帧，控制ratio从 0 ~ 1,每次递增0.1
	// 根据帧数来控制ratio,逐次保存生成结果 
	for (int i = 0; i < frame; i++) {
		double ratio = (double)1.0 * i / (frame - 1);
		cout << ratio << " . " << endl;
		MorphingTriangleSets(SrcTriangleSet, DstTriangleSet, &SrcImage,
			&DstImage, &MorphingImage, ratio);
		char FileName[100] = {0};
		sprintf_s(FileName, "%s_%d.bmp", "morphing\\morphing", i);
		MorphingImage.save(FileName);
		//MorphingImage.display(FileName);
	}
}


