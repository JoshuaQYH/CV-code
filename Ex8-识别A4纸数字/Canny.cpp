/*****************************************
参数说明：
	inputFileName:  待转换的彩色BMP图像
功能说明：
	将彩色BMP图像转换为灰度图
******************************************/

#include "Canny.h"
#include <queue>
#include <algorithm>

int Canny::readGrayImage(string inputFileName)
{
	try
	{
		SrcImage.load(inputFileName.c_str());
	}
	catch (const std::exception&)
	{
		return 0;
	}

	height= SrcImage._height;      // 矩阵行对应图像高度----------,多余去掉
	width = SrcImage._width;     // 列对应图像宽度
	CImg<short int> tmp(width, height, 1, 1, 0);
	grayImage = tmp;
	smoothImage = tmp;
	deltaXImage = tmp;
	deltaYImage = tmp;
	magnitudeImage = tmp;
	dirRadianImage = tmp;
	nonMaxSuppImage = tmp;
	resultEdgeImage = tmp;
	cutShortEdgeImage = tmp;

	// 灰度转换
	cimg_forXY(SrcImage, x, y) {
		// R G B 
		// Gray = R*0.299 + G*0.587 + B*0.114
		//grayImage(x, y) = SrcImage(x, y);
	     grayImage(x, y) = SrcImage(x, y, 0) * 0.299 + SrcImage(x, y, 1) * 0.587 + SrcImage(x, y, 2) * 0.114;

		// grayImage(x, y) = (SrcImage(x, y, 0) * 299 + SrcImage(x, y, 1) * 587 + SrcImage(x, y, 2) * 114 + 500) / 1000;
	}
	if (DEBUG) cout << "读入BMP并转化为灰度图成功" << endl;
	
	return 1;
}



/**************************************
参数：
	outputFileName: 检测出边缘后的图像名
	image: 检测出边缘后的图像对象
功能：
	保存检测后的灰度图
****************************************/
int Canny::writeGrayImage(string outputFileName, CImg<short int> image)
{
	image.save(outputFileName.c_str());
	if (DEBUG) cout << "输出图象已保存" << endl;
	return 0;
}



/***********************************************************************************
参数：
	image: 处理的图像对象
	sigma: 高斯分布的标准差，决定高斯滤波后图像被平滑的程度，高斯滤波属于低通filter
		   sigma越大，高斯滤波器的频带就越宽，平滑程度就越好
	smootheDim: The image after gaussian smoothing.
功能：
	对image对象进行高斯平滑，结果存在smootheDim
*************************************************************************************/
void Canny::gaussianSmooth(CImg<short int>* image, float sigma, CImg<short int>* smootheDim)
{
	int r, c, rr, cc,       /* Counter variables. */
		windowsize,         /* Dimension of the gaussian kernel. */
		center;             /* Half of the windowsize. */
	float *tempim,          /* Buffer for separable filter gaussian smoothing. */
		*kernel,            /* A one dimensional gaussian kernel. */
		dot,                /* Dot product summing variable. */
		sum;                /* Sum of the kernel weights variable. */
	int cols = (*image)._width;                        // 矩阵的行，图像的宽
	int rows = (*image)._height;                       // 矩阵的列，图像的长
	if (DEBUG) cout << "高斯模糊 " << endl;
	
	CImg<short int> tempImg(cols, rows, 1, 1, 0);  // 临时灰度图


	/**********************************
	* 1. 创建一维高斯平滑核
	***********************************/
	makeGaussianKernel(sigma, &kernel, &windowsize);
	center = windowsize / 2;
	

	/*   高斯滤波是可分离的，2D的高斯kernel拆成两个x，y方向，先对x方向滤波，滤波结果再y方向滤波  */
	
	/****************************
	* 2. 开始x方向的滤波模糊 ,卷积
	*****************************/
	if (DEBUG) cout << "Blur in the x-direction." << endl;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			dot = 0.0;
			sum = 0.0;
			for (cc = (-center); cc <= center; cc++) {
				if ((c + cc >= 0) && ((c + cc) < cols)) {
					dot += (float)(*image)(c+cc,r) * kernel[center + cc];
					sum += kernel[center + cc];
				}
			}
			tempImg(c, r) = dot / sum;
			//cout << r << "  " << c << endl;
		}
	}

	/*******************************
	* 3. 开始y方向上的滤波模糊 卷积
	********************************/
	if (DEBUG) cout << "Blur in the y-direction. " << endl;
	for (c = 0; c < cols; c++) {
		for (r = 0; r < rows; r++) {
			sum = 0.0;
			dot = 0.0;
			for (rr = (-center); rr <= center; rr++) {
				if ((r + rr >= 0) && (r + rr < rows)) {
					dot += tempImg(c, r + rr) * kernel[center + rr];
					sum += kernel[center + rr];
				}
			}
			(*smootheDim)(c, r) = (short int)(dot * BOOSTBLURFACTOR / sum + 0.5);
		}
	}
	free(kernel);
	
}

/**********************************************************
参数：
	sigma: 高斯分布标准差
	kernel：kernel代表高斯核的值，先带入高斯函数，然后归一化
	windowsize: 高斯核尺寸，滤波窗口值
功能：
	得到高斯核
*************************************************************/
void Canny::makeGaussianKernel(float sigma, float ** kernel, int * windowsize)
{
	int i, center;
	float x, fx, sum = 0.0;

	*windowsize = 1 + 2 * ceil(2.5 * sigma);  
	center = (*windowsize) / 2;

	//sigma越大，离散程度越厉害，因此加权的过程中，中心值的权重越弱，由此带来的图像越模糊；
	if (DEBUG) printf("      The kernel has %d elements.\n", *windowsize);
	if ((*kernel = (float *)calloc((*windowsize), sizeof(float))) == NULL) {
		fprintf(stderr, "Error callocing the gaussian kernel array.\n");
		exit(1);
	}

	for (i = 0; i<(*windowsize); i++) {
		x = (float)(i - center);
		fx = pow(2.71828, -0.5*x*x / (sigma*sigma)) / (sigma * sqrt(6.2831853));
		(*kernel)[i] = fx;  // 高斯核，直接点的位置导入，得到函数值
		sum += fx;
	}

	for (i = 0; i<(*windowsize); i++) (*kernel)[i] /= sum;  // 高斯核归一化

	if (DEBUG) {
		printf("The filter coefficients are:\n");
		for (i = 0; i<(*windowsize); i++)
			printf("kernel[%d] = %f\n", i, (*kernel)[i]);
	}
}

/**********************************************************
参数：
	smootheDim: 高斯平滑处理后的图像对象
	delta_x：x方向求偏导后的图像对象
	delta_y: y方向求偏导后的图像对象
函数功能：
	计算x y偏导后的图像
***********************************************************/
void Canny::derrivativeXY(CImg<short int> *smootheDim,
	CImg<short int> * delta_x, CImg<short int>  * delta_y)
{
	int cols = (*smootheDim)._width;                        // 矩阵的行，图像的宽
	int rows = (*smootheDim)._height;                       // 矩阵的列，图像的长


	if (DEBUG) cout << "计算 x 方向的偏导图像" << endl;
	
	/*
	for (int r = 1; r < rows - 1; r++) {
		for (int c = 1; c < cols - 1; c++) {
			(*delta_x)(c, r) = (*smootheDim)(c + 1, r) - (*smootheDim)(c, r);
		}
	}

	if (DEBUG) cout << "计算 y方向的偏导图像" << endl;
	for (int c = 1; c < cols - 1; c++) {
		for (int r = 1; r < rows - 1; r++) {
			(*delta_y)(c, r) = (*smootheDim) (c, r + 1) - (*smootheDim)(c, r);
		}
	}		
	*/

	
	/*sobel 算子*/
	
	int a00, a01, a02, a10, a11, a12, a20, a21, a22;
	for (int c = 1; c < cols - 1; c++) {
		for (int r = 1; r < rows - 1; r++) {
			a00 = (*smootheDim)(c - 1, r - 1);
			a01 = (*smootheDim)(c , r - 1);
			a02 = (*smootheDim)(c + 1, r - 1);
			a10 = (*smootheDim)(c - 1, r );
			a11 = (*smootheDim)(c, r);
			a12 = (*smootheDim)(c + 1, r );
			a20 = (*smootheDim)(c - 1, r + 1);
			a21 = (*smootheDim)(c , r + 1);
			a22 = (*smootheDim)(c + 1, r + 1);
			(*delta_x)(c, r) = a20 * 1 + a10 * 2 + a00 + ((-1) * a02 + (-2) * a12 + (-1)*a22);
			(*delta_y)(c, r) = a02 * (1) + a01 * (2) + a00 * (1) + a20 * (-1) + a21 * (-2) + a22 * (-1);
		}
	}
		
	

	

}

/*******************************************************************************
* FUNCTION: angle_radians
* PURPOSE: This procedure computes the angle of a vector with components x and
* y. It returns this angle in radians with the answer being in the range
* 0 <= angle <2*PI.
*******************************************************************************/
double Canny::angle_radians(double x, double y)
{
	///if (DEBUG) cout << "angel radians  偏导夹角" << endl;

	double xu, yu, ang;

	xu = fabs(x);
	yu = fabs(y);

	if ((xu == 0) && (yu == 0)) return(0);

	ang = atan(yu / xu);

	if (x >= 0) {
		if (y >= 0) return(ang);
		else return(2 * PI - ang);
	}
	else {
		if (y >= 0) return(PI - ang);
		else return(PI + ang);
	}
}

/*******************************************************************************
* Procedure: radian_direction
* Purpose: To compute a direction of the gradient image from component dx and
* dy images. Because not all derriviatives are computed in the same way, this
* code allows for dx or dy to have been calculated in different ways.
*
* FOR X:  xdirtag = -1  for  [-1 0  1]  x 轴的方向标志
*         xdirtag =  1  for  [ 1 0 -1]
*
* FOR Y:  ydirtag = -1  for  [-1 0  1]' y轴的方向标志
*         ydirtag =  1  for  [ 1 0 -1]'
*        传入 -1  -1 符合图像坐标系，即x轴朝右，y轴朝下
* The resulting angle is in radians measured counterclockwise from the
* xdirection. The angleMy Points "up the gradient".

参数：
	delta_x, delta_y 源图像的x，y方向的偏导图像结果
	dir_radian 梯度方向图
	xdirtag, ydirtag 确定 dx， dy的方向 
*******************************************************************************/
void Canny::radianDirection(CImg<short int>* delta_x, CImg<short int>* delta_y, 
	CImg<short int>* dir_radians, int xdirtag, int ydirtag)
{
	if (DEBUG) cout << "计算梯度方向图" << endl;

	int cols = (*delta_x)._width;
	int rows = (*delta_x)._height;
	
	double dx, dy;

	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			dx = (double)(*delta_x)(c, r);
			dy = (double)(*delta_y)(c, r);
			if (xdirtag == 1) dx = -dx;
			if (ydirtag == -1) dy = -dy;
			(*dir_radians)(c, r) =(float)angle_radians(dx, dy);
		}
	}
}

/*
参数：x方向偏导图，y方向偏导图，梯度幅值图
功能：计算梯度幅值 = sqrt（x^2 + y^2)
*/
void Canny::magnitudeXY(CImg<short int> * delta_x, CImg<short int> * delta_y, 
	 CImg<short int>* magnitude)
{
	if (DEBUG) cout << "计算梯度幅值图" << endl;
	int cols = (*delta_x)._width;
	int rows = (*delta_x)._height;

	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			int sq1 =(*delta_x)(col, row) * (*delta_x)(col, row);
			int sq2 = (*delta_y)(col, row)* (*delta_y)(col, row);
			(*magnitude)(col, row) = (short)(0.5 + sqrt( (float) sq1 + (float) sq2));
		}
	}
}

/**********************************************************************************
参数：梯度幅值图，x，y方向偏导图，梯度方向图， 非极大值抑制图
功能：非极大值抑制含义是，对于梯度幅值图上的每一个点G，比较边缘法线上的相邻两个点，
	 若两个点都比G大，那么G置为0，如果都小于或等于G，那么G = POSSIBLEEDGE，保留
***********************************************************************************/
void Canny::nonMaxSuppression(CImg<short int>* magnitude, CImg<short int>* delta_x,
	CImg<short int>* delta_y, CImg<short int>* nonMaxSupp)
{
	if (DEBUG) cout << "non max suppression " << endl;
	int ncols = (*delta_x)._width;
	int nrows = (*delta_x)._height;

	/*预处理，将结果图像的边缘置为0*/
	cimg_forXY(*nonMaxSupp, x, y) {
		(*nonMaxSupp)(x, y) = NOTEDGE;
	}

	for (int i = 1; i < ncols; i++) {
		for (int j = 1; j < nrows; j++) {
			/*************************************************************************
			* 扫描梯度幅值图，确定（xMag, yMag)是否为该梯度方向上的极大值点,
			*   比相邻两点G1, G2大的话，该位置填充灰度POSSIBLEEDGE（边缘），否则置为0（不是边缘）
			**************************************************************************/
			float deltaGx = (float)(*delta_x)(i, j);  // 该点的x方向偏导
			float deltaGy = (float)(*delta_y)(i, j);  // 该点的y方向偏导
			float weight = abs(deltaGy / deltaGx);
			if ((*magnitude)(i, j) < 30) continue;
			if (deltaGx == 0) {
				if ((*magnitude)(i, j) >= (*magnitude)(i, j - 1) && (*magnitude)(i, j) >= (*magnitude)(i, j + 1))
					(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
			}
			else if (deltaGy == 0) {
				if ((*magnitude)(i, j) >= (*magnitude)(i - 1, j) && (*magnitude)(i, j) >= (*magnitude)(i + 1, j))
					(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
			}
			else if (deltaGx == deltaGy) {
				if ((*magnitude)(i, j) >= (*magnitude)(i - 1, j - 1) && (*magnitude)(i, j) >= (*magnitude)(i + 1, j + 1))
					(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
			}
			else if (deltaGx == (-1)*deltaGy) {
				if ((*magnitude)(i, j) >= (*magnitude)(i - 1, j + 1) && (*magnitude)(i, j) >= (*magnitude)(i + 1, j - 1))
					(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
			}
			else {
				/*  估计邻域法
				float angle = atan(abs(deltaGy) / deltaGx);
				if (angle <= PI / 8 || angle >= PI * 7.0 / 8) {
					if ((*magnitude)(i, j) > (*magnitude)(i - 1, j) && (*magnitude)(i, j) > (*magnitude)(i + 1, j))
						(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
				}
				else if (angle > PI / 8 && angle < PI * 3 / 8) {
					if ((*magnitude)(i, j) >(*magnitude)(i - 1, j - 1) && (*magnitude)(i, j) > (*magnitude)(i + 1, j + 1))
						(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
				}
				else if (angle >= PI * 3 / 8 && angle <= PI * 5 / 8) {
					if ((*magnitude)(i, j) > (*magnitude)(i, j - 1) && (*magnitude)(i, j) > (*magnitude)(i, j + 1))
						(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
				}
				else if (angle > PI * 5 / 8 && angle < PI * 7 / 8) {
					if ((*magnitude)(i, j) >(*magnitude)(i - 1, j + 1) && (*magnitude)(i, j) > (*magnitude)(i + 1, j - 1))
						(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
				}
				*/
	
				
				// 线性插值法 
				if (deltaGx * deltaGy < 0) {
					if (abs(deltaGx) > abs(deltaGy)) {
						float G1 = weight * (*magnitude)(i + 1, j - 1) + (1 - weight) * (*magnitude)(i + 1, j);
						float G2 = weight * (*magnitude)(i - 1, j + 1) + (1 - weight) * (*magnitude)(i - 1, j);
						if ((*magnitude)(i, j) >= G1 && (*magnitude)(i, j) >= G2)
							(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
					}
					else if(abs(deltaGx) < abs(deltaGy)) {
						float G1 = weight * (*magnitude)(i + 1, j - 1) + (1 - weight) * (*magnitude)(i, j - 1);
						float G2 = weight * (*magnitude)(i - 1, j + 1) + (1 - weight) * (*magnitude)(i, j + 1);
						if ((*magnitude)(i, j) >= G1 && (*magnitude)(i, j) >= G2)
							(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
					}
				}
				else if(deltaGx * deltaGy > 0) {
					if (abs(deltaGx) > abs(deltaGy)) {
						float G1 = weight * (*magnitude)(i - 1, j - 1) + (1 - weight) * (*magnitude)(i - 1, j);
						float G2 = weight * (*magnitude)(i + 1, j + 1) + (1 - weight) * (*magnitude)(i + 1, j);
						if ((*magnitude)(i, j) >= G1 && (*magnitude)(i, j) >= G2)
							(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
					}
					else if(abs(deltaGx) < abs(deltaGy)){
						float G1 = weight * (*magnitude)(i - 1, j - 1) + (1 - weight) * (*magnitude)(i, j - 1);
						float G2 = weight * (*magnitude)(i + 1, j + 1) + (1 - weight) * (*magnitude)(i, j + 1);
						if ((*magnitude)(i, j) >= G1 && (*magnitude)(i, j) >= G2)
							(*nonMaxSupp)(i, j) = POSSIBLEEDGE;
					}
				}
				
				
			}
		}

	}
}

/****************************************************************************
参数： 梯度图magnitude，非最大化抑制图nonMaxSupp, 边缘图edge
		低阈值点tlow
        高阈值点thigh
功能：边缘检测，减少假边缘的数目，具体做法是采用双阈值法，选择两个高低阈值，
	  整个过程描述如下：
		用非最大抑制的结果初始化edge，扫描edge：
		如果该像素的梯度值小于tlow，则该像素为非边缘像素；
		如果该像素的梯度值大于thigh，则该像素为边缘像素；
		如果该像素的梯度值介于tlow与thigh之间，需要进一步检测该像素的邻域内的8个点，
		如果这8个点内有一个或以上的点梯度超过了thigh，则该像素为边缘像素，否则不是边缘像素。。
**************************************************************************************/
void Canny::applyHysteresis(CImg<short int>* magnitude, CImg<short int>* nonMaxSupp, 
	   float tlow, float thigh, CImg<short int>* edge)
{
	if (DEBUG) cout << "apply_hystersis ing " << endl;
	int cols = (*magnitude)._width;
	int rows = (*magnitude)._height;

	/*1. 非最大值抑制结果来初始化边缘图,图像四周置为0 */
	cimg_forXY(*nonMaxSupp, x, y) {
		(*edge)(x, y) = (*nonMaxSupp)(x, y);
		if (x == 0 || x == cols || y == 0 || y == rows)
			(*edge)(x, y) = NOTEDGE;
	}


	cimg_forXY(*edge, x, y) {
		// 如果该像素的梯度值小于tlow，则该像素为非边缘像素；
		if ((*magnitude)(x, y) < tlow) {
			(*edge)(x, y) = 0;
		}
		//如果该像素的梯度值大于thigh，则该像素为边缘像素；
		else if ((*magnitude)(x, y) > thigh) {
			(*edge)(x, y) = POSSIBLEEDGE;
		}
		//如果该像素的梯度值介于tlow与thigh之间，需要进一步检测该像素的邻域内的8个点，
		//如果这8个点内有一个或以上的点梯度超过了thigh，则该像素为边缘像素，否则不是边缘像素。。
		else {
			//考虑有领域有8个点的情况
			if (x > 0 && y > 0 && x < cols && y < rows) {
				if ((*magnitude)(x - 1, y - 1) >  thigh|| (*magnitude)(x, y - 1) >   thigh || (*magnitude)(x + 1, y - 1) >  thigh
					|| (*magnitude)(x - 1, y) >   thigh || (*magnitude)(x, y + 1) >   thigh ||
					(*magnitude)(x - 1, y + 1) >  thigh || (*magnitude)(x, y + 1) >  thigh || (*magnitude)(x + 1, y + 1) >   thigh)
					(*edge)(x, y) = POSSIBLEEDGE;
				else
					(*edge)(x, y) = NOTEDGE;
			}
		}
	}

}



/****************************************************************
参数： edge 阈值处理后的图像对象
功能： 删除图像中较短长度小于20的边缘，边缘长度定义为像素点的个数
	   过程是，对某一边缘进行广度搜索，统计长度，若小于20就将这些边缘点置为NOTEDGE.
	   达到剔除短边缘的目的。
******************************************************************/
void Canny::cutShortEdge(CImg<double>* edge, CImg<double>* resultEdgeImage, int cutLen)
{
	if (DEBUG) cout << "  剔除短边缘" << endl;
	int cols = (*resultEdgeImage)._width;
	int rows = (*resultEdgeImage)._height;
	cimg_forXY((*resultEdgeImage), x, y) {
		(*edge)(x, y) = (*resultEdgeImage)(x, y);
	}
	

	// 标记图像, 记录搜索过的点，设置灰度值为128
	CImg<double> tmp(cols, rows, 1, 1, 0);
	queue<XY> edgeQueue;   //定义一个队列进行广度搜索
	vector<XY>PointSet;   //一个边缘的点的集合
	
	for (int i = 0; i < cols; i++) {
		for (int j = 0; j < rows; j++) {
			//cout << "1" << endl;
			if ((*edge)(i, j) == POSSIBLEEDGE && tmp(i, j) != 128) {
				// 从该点出发，进行边缘点搜索，统计边缘点长度
			//	cout << "0" << endl;
				while(!edgeQueue.empty())
					edgeQueue.pop();   //清空队列
				 PointSet.clear();      //  清空向量

				 XY Point;              //点结构体
				 Point.col = i;         // 当前点 坐标
				 Point.row = j;

				edgeQueue.push( Point);  // 将当前节点加入队列
				 PointSet.push_back( Point);  //将当前节点记入记录的向量
				tmp(i, j) = 128;      //  当前位置已被搜索

				int ii = i;   // 搜索过程中变化的坐标
				int jj = j;

				while (edgeQueue.size() != 0) {  //  搜索边缘完毕
					ii = edgeQueue.front().col;  // 更新坐标
					jj = edgeQueue.front().row;
					edgeQueue.pop();   //弹出队头
					bool jumpFlag = false;  // 优化？
					// 搜索点（ii，jj）八 邻域
					for (int iii = -1; iii <= 1; iii++) {
						for (int jjj = -1; jjj <= 1; jjj++) {
							if (iii == 0 && jjj == 0)
								continue;  // 跳过初始点
							 Point.col = ii + iii;
							 Point.row = jj + jjj;

							bool flag = false;
							for (int t = 0; t <PointSet.size(); t++) {
								if ( PointSet[t].col ==Point.col &&PointSet[t].row ==Point.row) {
									flag = true;
									break;
								}
							}
							if (flag) continue;  // 跳过已搜索的点 

							// 找到未搜索过并且为边缘点的店
							if ((*edge)(ii + iii, jj + jjj) == POSSIBLEEDGE){
								tmp(ii + iii, jj + jjj) = 128;
								 PointSet.push_back( Point);
								edgeQueue.push( Point);
							//	cout << "1" << endl;
							 }
							if ( PointSet.size() > 20) {
								jumpFlag = true;
								break;
							}
					
						}

					}
					if (jumpFlag)
						break;
				}
				if ( PointSet.size() <= cutLen) { // 删除短边缘
					//cout << " 2 " << endl;
					for (int k = 0; k <PointSet.size(); k++) {
						int r = PointSet[k].row;
						int c = PointSet[k].col;
						tmp(c, r) = 128;
						(*edge)(c, r) = NOTEDGE;  
					}
				}
				 PointSet.clear();

			}
		}
	}
	cout << "finish" << endl;
}


/*
对输入文件进行边缘检测，然后得到输出图像
*/
CImg<short int> Canny::canny(string inputFileName) {
	cout << "正在检测图像边缘..." << endl;
	/*1. 读取图像，并转化为灰度图*/
	readGrayImage(inputFileName);
	/*2. 高斯平滑*/
	gaussianSmooth(&grayImage, this->sigma, &smoothImage);
	/*3. 求xy方向的偏导图*/
	derrivativeXY(&smoothImage, &deltaXImage, &deltaYImage);
	/*4. 求梯度方向图*/
	radianDirection(&deltaXImage, &deltaYImage, &dirRadianImage, -1, -1);  //x轴朝右 -1 ,y轴朝下 -1 
	/*5. 求梯度幅值图*/
	magnitudeXY(&deltaXImage, &deltaYImage, &magnitudeImage);
	/*6. 非最大化抑制*/
	nonMaxSuppression(&magnitudeImage, &deltaXImage, &deltaYImage, &nonMaxSuppImage);
	/*7. 滞后阈值处理*/
	applyHysteresis(&magnitudeImage, &nonMaxSuppImage, tLow, tHigh, &resultEdgeImage);
	/*8.  剔除短边缘*/
	//cutShortEdge(&cutShortEdgeImage,&resultEdgeImage, 30);

	/*9.输出最终图像*/
	string name;  // 规范命名

	cout << "-------------" << endl;
	/*将图像保存*/
	/*1.  灰度图 */
	
	sprintf_s(output, "%d_Gray_sigma=%.2f_low=%.2f_high=%.2f_bf=%.4f_%s", number, this->sigma, this->tLow, this->tHigh, BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.assign(output);
	name += inputFileName;
//	writeGrayImage(name, grayImage);
	
	/*2.高斯模糊图*/
	sprintf_s(output, "%d_Smoothe_sigma=%.2f_low=%.2f_high=%.2f_bf=%.4f_%s", number, this->sigma, this->tLow, this->tHigh, BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName;
	//writeGrayImage(name, smoothImage);

	/*3.x y方向偏导图 */
	sprintf_s(output, "%d_DeltaX_sigma=%.2f_low=%.2f_high=%.2f_bf=%.4f_%s",number, this->sigma, this->tLow, this->tHigh, BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName;
	//writeGrayImage(name,deltaXImage);
	sprintf_s(output, "%d_DeltaY_sigma=%.2f_low=%.2f_high=%.2f_bf=%.4f_%s",number, this->sigma, this->tLow, this->tHigh, BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName;
	//writeGrayImage(name, deltaYImage);

	/*4. 梯度方向图*/
	sprintf_s(output, "%d_DirRadian_sigma=%.2f_low=%.2f_high=%.2f_bf=%.4f_%s", number, this->sigma, this->tLow, this->tHigh, this->BOOSTBLURFACTOR,this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName;
	//writeGrayImage(name, dirRadianImage);

	/*5. 梯度幅值图*/
	sprintf_s(output, "%d_Magnitude_sigma=%.2f_low=%.2f_high=%.2f_bf=%.4f_%s", number, this->sigma, this->tLow, this->tHigh,this->BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName; 
	//writeGrayImage(name,  magnitudeImage);

	/*6. 非最大化抑制图*/
	sprintf_s(output, "%d_NonMaxSupp_sigma=%.2f_low=%.2f_high=%.2f_bf=%.4f_%s", number, this->sigma, this->tLow, this->tHigh, BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName;
	//writeGrayImage(name, nonMaxSuppImage);

	/*7.滞后阈值处理得到最终图像*/
	sprintf_s(output, "%d_resultEdge_sigma=%.3f_low=%.2f_high=%.2f_bf=%.4f_%s", number, this->sigma, this->tLow, this->tHigh, BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName;
//	writeGrayImage(name, resultEdgeImage);
	//cout << name.c_str() << endl;

	/*8. 短边缘结果*/
	sprintf_s(output, "%d_cutShortEdge_sigma=%.3f_low=%.2f_high=%.2f_bf=%.4f_%s", number, this->sigma, this->tLow, this->tHigh, BOOSTBLURFACTOR, this->inputFileName.c_str());
	name.clear();
	name.assign(output);
	name += inputFileName;
	//writeGrayImage(name, cutShortEdgeImage);

	// 打印图像
	if (DEBUG) {
		//grayImage.display("Gray source image");
		//smoothImage.display("Smooth image");
		//deltaXImage.display("delta x image");
		//deltaYImage.display("delta y image");
		//dirRadianImage.display("direction radian image.");
		magnitudeImage.display("magnitude image.");
		nonMaxSuppImage.display("non max suppression image");
		resultEdgeImage.display("the result of edge image");
		cutShortEdgeImage.display("cut the short edge of the image");
	}
	
	number++; // 文件名序号递增
	//return cutShortEdgeImage;
	return resultEdgeImage;
}
