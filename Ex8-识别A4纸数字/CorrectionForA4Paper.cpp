#include "CorrectionForA4Paper.h"

bool compare( Point a,Point b) {
	return a.value < b.value;
}
CImg<double> CorrectionForA4Paper::ToGrayImage(CImg<double> image)
{
	if (PRINT) cout << "转换为灰度图......" << endl;
	CImg<double> grayImage(image._width, image._height, 1, 1, 0);
	cimg_forXY(image, x, y) {
		grayImage(x, y, 0) = image(x, y, 0) * 0.299 + image(x, y, 1) * 0.587 + image(x, y, 2) * 0.114;
	}
	return grayImage;

}


vector<int> CorrectionForA4Paper::getInterpolationRGB(double scrX, double scrY) {
	vector<int> RGBvec;
	double scrX_head = floor(scrX);
	double scrY_head = floor(scrY);
	double scrX_tail = 0, scrY_tail = 0;
	
	int r00 = originImage((int)scrX_head, (int)scrY_head, 0, 0);
	int g00 = originImage((int)scrX_head, (int)scrY_head, 0, 1);
	int b00 = originImage((int)scrX_head, (int)scrY_head, 0, 2);
	int r01, r10, r11, g01, g10, g11, b01, b10, b11;
	r01 = r10 = r11 = g01 = g10 = g11 = b01 = b10 = b11 = 0;

	if ((int)scrY_head < originImage._height - 1) {
		scrY_tail = scrY - scrY_head;
		r01 = originImage((int)scrX_head, (int)scrY_head + 1, 0, 0);
		g01 = originImage((int)scrX_head, (int)scrY_head + 1, 0, 1);
		b01 = originImage((int)scrX_head, (int)scrY_head + 1, 0, 2);
	}
	if ((int)scrX_head < originImage._width - 1) {
		scrX_tail = scrX - scrX_head;
		r10 = originImage((int)scrX_head + 1, (int)scrY_head, 0, 0);
		g10 = originImage((int)scrX_head + 1, (int)scrY_head, 0, 1);
		b10 = originImage((int)scrX_head + 1, (int)scrY_head, 0, 2);
	}
	if (((int)scrY_head < originImage._height - 1) && ((int)scrX_head < originImage._width - 1)) {
		r11 = originImage((int)scrX_head + 1, (int)scrY_head + 1, 0, 0);
		g11 = originImage((int)scrX_head + 1, (int)scrY_head + 1, 0, 1);
		b11 = originImage((int)scrX_head + 1, (int)scrY_head + 1, 0, 2);
	}

	double scrX_tail_anti = (double)1 - scrX_tail;
	double scrY_tail_anti = (double)1 - scrY_tail;

	double temp_r = (double)r00 * scrX_tail_anti * scrY_tail_anti
		+ (double)r01 * scrX_tail_anti * scrY_tail
		+ (double)r10 * scrX_tail * scrY_tail_anti
		+ (double)r11 * scrX_tail * scrY_tail;
	double temp_g = (double)g00 * scrX_tail_anti * scrY_tail_anti
		+ (double)g01 * scrX_tail_anti * scrY_tail
		+ (double)g10 * scrX_tail * scrY_tail_anti
		+ (double)g11 * scrX_tail * scrY_tail;
	double temp_b = (double)b00 * scrX_tail_anti * scrY_tail_anti
		+ (double)b01 * scrX_tail_anti * scrY_tail
		+ (double)b10 * scrX_tail * scrY_tail_anti
		+ (double)b11 * scrX_tail * scrY_tail;
	int new_r = (int)floor(temp_r);
	int new_g = (int)floor(temp_g);
	int new_b = (int)floor(temp_b);

	RGBvec.push_back(new_r);
	RGBvec.push_back(new_g);
	RGBvec.push_back(new_b);

	return RGBvec;
}

// 
CImg<double> CorrectionForA4Paper::houghTransform(CImg<double> image)
{
	if (PRINT) cout << "霍夫变换..." << endl;
	int width = image._width;
	int height = image._height;
	int sqrt_xy = sqrt(width * width + height * height);

	
	CImg<double> tmp(360, sqrt_xy , 1, 1, 0);
	houghSpaceImage = tmp;  // 初始化霍夫空间

	/*
	double sin_value[180] = {};
	double cos_value[180] = {};

	int p = 0;
	int i = 0;
	for (int i = - 90; i < 90; i++) {
		sin_value[i + 90] = sin(i * pi / 180);
		cos_value[i + 90] = cos(i * pi / 180);
	}*/

	cimg_forXY(image, x, y) {
		if (image(x, y, 0) != 255)
			continue;
		for (int i = 0; i < 360; i++) { 
			double theta = (double)i * MyPI / 180.0;
			int p = (int)(x*cos(theta) + y * sin(theta));
			if (p >= 0 && p < sqrt_xy){
				houghSpaceImage(i, p, 0)++;

			}
				
		}
	}

	//houghSpaceImage.display();
	return houghSpaceImage;
}

vector< Point> CorrectionForA4Paper::getLinesFromHoughSpace(CImg<double> houghSpaceImage, CImg<double> _orignImage, int threshold)
{
	CImg<double> drawImage = _orignImage;
	
	vector< Point>PointSet;   // 存放从霍夫空间中提取的峰值点

	// 从霍夫空间中提取峰值点，得到直线的极坐标方程
	const int ymin = 0;
	const int ymax = originImage._height - 1;
	const int xmin = 0;
	const int xmax = originImage._width - 1;
	cimg_forXY(houghSpaceImage, x, y) {
		
		if (houghSpaceImage(x, y, 0) > threshold) {
			bool flag = false;
		
			for (int i = 0; i <PointSet.size(); i++) {
				if (pow( PointSet[i].x - x, 2) + pow( PointSet[i].y - y, 2) < houghPointDistance) {
					flag = true;
					if ( PointSet[i].value < houghSpaceImage(x, y, 0)) {
						 PointSet[i] = *(new Point(x, y, houghSpaceImage(x, y, 0)));
					//	cout << " Point; " << x << " " << y << " " << houghSpaceImage(x, y, 0) << endl;
					}
				}
			}
			if (flag == false) {
				 PointSet.push_back(*(new Point(x, y, houghSpaceImage(x, y, 0))));
				//cout << " Point; " << x << " " << y << " " << houghSpaceImage(x, y, 0) << endl;
			}

			// 求出极坐标转为笛卡尔坐标的直线，与x ，y轴的交点坐标
			/*
			const int x0 = CrossY(x, y, ymin);  // 直线与 y = ymin直线的交点横坐标
			const int x1 = CrossY(x, y, ymax); // 
			const int y0 = CrossX(x, y, xmin); // 
			const int y1 = CrossX(x, y, xmax);


			if (x0 >= xmin && x0 <= xmax ||              // 表示的直线是否在图像内  
				x1 >= xmin && x1 <= xmax ||
				y0 >= ymin && y0 <= ymax ||
				y1 >= ymin && y1 <= ymax) {
				for (int i = 0; i <My PointSet.size(); i++) {
					if (pow( PointSet[i].x - x, 2) + pow( PointSet[i].y-y, 2) < hough PointDistance ) {
						flag = true;
						if ( PointSet[i].value < houghSpaceImage(x, y, 0)) {
							 PointSet[i] = *(newMy Point(x, y, houghSpaceImage(x, y, 0)));
							cout << " Point replace; " << x << " " << y << " " << houghSpaceImage(x, y, 0) << endl;
						}
					}
				}
				if (flag == false) {
					 PointSet.push_back(*(newMy Point(x, y, houghSpaceImage(x, y, 0))));
					cout << " Point; " << x << " " << y << " " << houghSpaceImage(x, y, 0) << endl;
				}
			}*/
		}
	}
	
	vector<Line> lineSet;  // 笛卡尔坐标系下直线方程集合
	// 极坐标转为笛卡尔坐标系
	for (int i = 0; i < PointSet.size(); i++) {
		double angle = (double) PointSet[i].x * MyPI / 180.0;
		double k = -cos(angle) / sin(angle);
		double b = (double) PointSet[i].y / sin(angle);
		lineSet.push_back(*(new Line(k, b)));
		cout << "直线方程为 y = (" << k << ") x + ( " << b << ")" << endl;
	}

	//画线
	const double line_color[] = { 255, 0, 0 };
	for (int i = 0; i < lineSet.size(); i++) {
		const int x0 = double(ymin - lineSet[i].b) / lineSet[i].k;
		const int y0 = xmin * lineSet[i].k + lineSet[i].b;

		const int x1 = double(ymax - lineSet[i].b) / lineSet[i].k;
		const int y1 = xmax * lineSet[i].k + lineSet[i].b;
		
		if (abs(lineSet[i].k) > 1) {
			drawImage.draw_line(x0, ymin, x1, ymax, line_color);

		}
		else {
			drawImage.draw_line(xmin, y0, xmax, y1, line_color);
		}
	}


	// 计算直线交点坐标
	//cout << "交点坐标: " << endl;
	vector< Point> intersectionSet;  // 直线交点集合

	for (int i = 0; i < lineSet.size(); i++) {
		for (int j = 0; j < lineSet.size(); j++) {
			double k_i = lineSet[i].k;
			double b_i = lineSet[i].b;
			double k_j = lineSet[j].k;
			double b_j = lineSet[j].b;

			double x = (b_j - b_i) / (k_i - k_j);   // 交点坐标
			double y = (k_i * b_j - k_j * b_i) / (k_i - k_j);

			if (x >= 0 && x < _orignImage._width && y >= 0 && y < _orignImage._height) {
				bool flag = false;
				if (!flag) {
					intersectionSet.push_back(*(new Point(x, y, 0)));
				//	cout << "( " << x << " ," << y << " )" << endl;
				}
			}
		}
	}
	// 画点
	double Point_color[] = { 0, 255, 0 };
	for (int i = 0; i < intersectionSet.size(); i++) {
		drawImage.draw_circle(intersectionSet[i].x, intersectionSet[i].y, 15,Point_color);
	}



	//根据距离筛选出4个角点
	int length = intersectionSet.size();
	vector<bool> dotFlag;
	for (int i = 0; i < length;  i++) {
		dotFlag.push_back(false);
	}
	vector< Point>Pointset;
	vector<int> voteNumSet;  // 记录角点票数
	for (int i = 0; i < length; i++) {
		if (dotFlag[i] && voteNumSet.size() >= 4) {
			// 不查找与已有角点相邻的点
			continue;
		}
		// 根据距离投票选出角点
		int voteNum = 0;
		vector<int> index;
		for (int j = 0; j < length; j++) {
			if (dotFlag[j])  // 跳过 标志为true的点
				continue;
			if (j == i) continue;
			int x1 = intersectionSet[i].x;
			int y1 = intersectionSet[i].y;
			int x2 = intersectionSet[j].x;
			int y2 = intersectionSet[j].y;
			int distance = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
			if (distance < this->_distance) { // 统计相邻点，然后记录其下标
				voteNum++;
				index.push_back(j);
			}
		}
		if (voteNum >= this->_voteNum) {  // 找到目标角点，添加，然后将相邻点视为同一点，做标记
			
			int midX = drawImage._width / 2;
			int midY = drawImage._height / 2;
			int minDistance = 100000;
			int resX = 0;
			int resY = 0;
			for (int i = 0; i < index.size(); i++) {
				int x0 =  intersectionSet[index[i]].x;
				int y0 = intersectionSet[index[i]].y;
				int distance0 = sqrt((x0 - midX) * (x0 - midX) + (y0 - midY) * (y0 - midY));
				if (distance0 < minDistance) {
					resX = x0;
					resY = y0;
					minDistance = distance0;
				}
			}
		
			Pointset.push_back(Point(resX, resY, 0));
			voteNumSet.push_back(voteNum);
			dotFlag[i] = true;
			for (int j = 0; j < index.size(); j++)
				dotFlag[index[j]] = true;
		}

	}
	vector< Point> top4Point;
	int maxVote = 0;
	int maxIndex = 0;
	while (top4Point.size() < 4) {
		maxVote = 0;
		maxIndex = 0;
		for (int i = 0; i < voteNumSet.size(); i++) {
			if (voteNumSet[i] > maxVote) {
				maxVote = voteNumSet[i];
				maxIndex = i;
			}
		}
		top4Point.push_back( Pointset[maxIndex]);
		voteNumSet[maxIndex] = -1;
	}

	// 防止点不等于4个
	if (top4Point.size() < 4) {
		for (int i = 0; i < 4 - top4Point.size(); i++) {
			int px = rand() % originImage._width;
			int py = rand() % originImage._height;
			Point tmpPoint(px, py, 0);
			top4Point.push_back(tmpPoint);
		}
	}
	// 画点
	CImg<double> image = originImage;
	for (int i = 0; i < top4Point.size(); i++) {
		image.draw_circle(top4Point[i].x, top4Point[i].y, 15,Point_color);
	}
	image.display();
	drawImage.display();
	//drawEdgeOnA4Paper(drawImage, PointSet);

	return drawEdgeOnA4Paper(drawImage, top4Point);
}


vector< Point> CorrectionForA4Paper::drawEdgeOnA4Paper(CImg<double> &drawImage, vector< Point>PointSet)
{
	if ( PointSet.size() != 4){
		cout << "A4纸 必须是4个角点, 检测结果无效" << endl;
		Sleep(4000);
		exit(0);
	}
	
	orderPointSet.push_back( PointSet[0]);
	bool crossFlag[4] = { true, false, false, false };  // 与 flag0 对角 结果为true
	int  Point_Index = 0;
	for (int i = 1; i < 4; i++) {
		float k = 1.0 * (orderPointSet[0].y -PointSet[i].y) / (orderPointSet[0].x - PointSet[i].x);
		float b = orderPointSet[0].y - k * orderPointSet[0].x;
		int flag = 0;  // 标志是否为对角点
		for (int j = 1; j < 4; j++) {
			if (j == i) continue;
			float y = k *PointSet[j].x + b;
			if ( PointSet[j].y - y > 0) {
				flag++;
			}
			else {
				flag--;
			}
		}
		if (flag == 0) {
			// 找到00的对角点 
			Point_Index = i;
			crossFlag[Point_Index] = true;
			break;
		}
	}
	int next = 0;  // 确定好对角点，确定顺序
	for (int i = 1; i < 4; i++) {
		/*
		if (crossFlag[i] == false) {
			orderPointSet.push_back( PointSet[i]);
			if(stop == 0) orderPointSet.push_back( PointSet[Point_Index]);
			crossFlag[i] = true;
			if(stop == 0)i = 1;
			stop = 1;
		}*/
		if (crossFlag[i] == false && next == 0) {
			orderPointSet.push_back(PointSet[i]);
			orderPointSet.push_back(PointSet[Point_Index]);
			next = 1;
		}
		else if (crossFlag[i] == false && next == 1) {
			orderPointSet.push_back(PointSet[i]);
			next = 2;
		}
			
		
	}
	
	for (int i = 0; i < 4; i++) {
		int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
		if (i == 3) {
			x1 = orderPointSet[3].x;
			y1 = orderPointSet[3].y;
			x2 = orderPointSet[0].x;
			y2 = orderPointSet[0].y;
		}
		else {
			x1 = orderPointSet[i].x;
			y1 = orderPointSet[i].y;
			x2 = orderPointSet[i + 1].x;
			y2 = orderPointSet[i + 1].y;
		}
		double line_color[] = { 0, 0, 255 };
		drawImage.draw_line(x1, y1, x2, y2, line_color);
	}

	
	double color1[] = { 255,255,255 };
	double color2[] = { 255,0,0 };
	double color3[] = { 0,255,0 };
	double color4[] = { 0,0,255 };
	// 白 红 绿 蓝
	CImg<double> img = originImage;
	img.draw_circle(orderPointSet[0].x, orderPointSet[0].y, 50, color1);
	img.draw_circle(orderPointSet[1].x, orderPointSet[1].y, 50, color2);
	img.draw_circle(orderPointSet[2].x, orderPointSet[2].y, 50, color3);
	img.draw_circle(orderPointSet[3].x, orderPointSet[3].y, 50, color4);

	//img.display();
	return orderPointSet;
}



CImg<double> CorrectionForA4Paper::correctA4Paper(CImg<double> originImage, vector< Point> PointSet)
{
	/////// 确定A4纸的左上角的点
	int minIndex = 0; //
	int minDistance = 1000000;
	for (int i = 0; i < PointSet.size(); i++) {  // 按照离原点最近的原则选取A4纸的左上角的点
		int distance = sqrt( PointSet[i].x * PointSet[i].x +PointSet[i].y *PointSet[i].y);
		if (distance < minDistance) {
			minIndex = i;
			minDistance = distance;
		}
	}
	vector< Point> leftUpPointFirstSet(4);  // 存储从左上角开始的点集合，顺时针或者逆时针
	int tmpIndex = minIndex;
	for (int i = 0; i < 4; i++) {
		leftUpPointFirstSet[i] =PointSet[tmpIndex++];
		tmpIndex %= 4;
	}

	CImg<double> originA4Paper = originImage;

	////// 确定A4纸长宽 
	int dis1 = calDistance(leftUpPointFirstSet[0], leftUpPointFirstSet[1]);
	int dis2 = calDistance(leftUpPointFirstSet[1], leftUpPointFirstSet[2]);
	int dis3 = calDistance(leftUpPointFirstSet[2], leftUpPointFirstSet[3]);
	int dis4 = calDistance(leftUpPointFirstSet[3], leftUpPointFirstSet[0]);
	
	int rankDis[4] = { dis1, dis2, dis3, dis4 };

	sort(rankDis, rankDis + 4);


	/// 确定变换点的对应关系
	bool directionFlag = false;  // 为正表示角点从左上角顺时针分布
	if (dis1 < dis2) {
		directionFlag = true;    // 第一边小于第二边，则为顺时针
	}

	vector< Point> a4EdgePointSet; // 该数组存储与leftUp PointFirSet位置相对应的A4纸角点
	a4EdgePointSet.push_back( Point(0, 0, 0));

	int a4width = 0;
	int a4height = 0;
	int a4_tmp_width = 0;
	int a4_tmp_height = 0;

	if (originImage._width > originImage._height) {
		a4width = A4height * MyScalar;
		a4height = A4width * MyScalar;
	}
	else{
		a4width = A4width * MyScalar;
		a4height = A4height * MyScalar;
	}

	CImg<double> A4Paper(a4width, a4height, 1, 3, 0);

	if (directionFlag) { // 顺时针
		a4EdgePointSet.push_back(Point(a4width, 0, 0));
		a4EdgePointSet.push_back(Point(a4width, a4height, 0));
		a4EdgePointSet.push_back(Point(0, a4height, 0));
	}
	else {
		a4EdgePointSet.push_back( Point(0, a4height, 0));
		a4EdgePointSet.push_back( Point(a4width, a4height, 0));
		a4EdgePointSet.push_back( Point(a4width, 0, 0));
	}


	CImg<double> img = originImage;
	double color1[] = { 255,255,255 };
	double color2[] = { 255,0,0 };
	double color3[] = { 0,255,0 };
	double color4[] = { 0,0,255 };
	img.draw_circle(a4EdgePointSet[0].x, a4EdgePointSet[0].y, 50, color1);
	img.draw_circle(a4EdgePointSet[1].x, a4EdgePointSet[1].y, 50, color2);
	img.draw_circle(a4EdgePointSet[2].x, a4EdgePointSet[2].y, 50, color3);
	img.draw_circle(a4EdgePointSet[3].x, a4EdgePointSet[3].y, 50, color4);
	//img.display("调整1");
	CImg<double> img1 = originImage;
	img1.draw_circle(leftUpPointFirstSet[0].x, leftUpPointFirstSet[0].y, 5, color1);
	img1.draw_circle(leftUpPointFirstSet[1].x, leftUpPointFirstSet[1].y, 5, color2);
	img1.draw_circle(leftUpPointFirstSet[2].x, leftUpPointFirstSet[2].y, 5, color3);
	img1.draw_circle(leftUpPointFirstSet[3].x, leftUpPointFirstSet[3].y, 5, color4);
	////img1.display("调整2");


	double x0, y0, x1, y1, x2, y2, x3, y3;
	if (directionFlag == false) {
		x0 = leftUpPointFirstSet[0].x;
		y0 = leftUpPointFirstSet[0].y;
		x1 = leftUpPointFirstSet[3].x;
		y1 = leftUpPointFirstSet[3].y;
		x2 = leftUpPointFirstSet[2].x;
		y2 = leftUpPointFirstSet[2].y;
		x3 = leftUpPointFirstSet[1].x;
		y3 = leftUpPointFirstSet[1].y;
	}
	else {
		x0 = leftUpPointFirstSet[0].x;
		y0 = leftUpPointFirstSet[0].y;
		x1 = leftUpPointFirstSet[1].x;
		y1 = leftUpPointFirstSet[1].y;
		x2 = leftUpPointFirstSet[2].x;
		y2 = leftUpPointFirstSet[2].y;
		x3 = leftUpPointFirstSet[3].x;
		y3 = leftUpPointFirstSet[3].y;
	}
	

	double dx32 = x3 - x2;
	double dx21 = x2 - x1;
	double dy32 = y3 - y2;
	double dy21 = y2 - y1;

	double a, b, c, d, e, f, g, h;

	c = x0;
	f = y0;
	
	if (originImage._width > originImage._height) {
		h = ((dy32 + y1 - f) * dx21 - (dx32 + x1 - c) * dy21) / ((A4width * 2.0) * (dx32 * dy21 - dy32 * dx21));
		g = ((A4width * 2.0) * dx32 * h + (dx32 + x1 - c)) / ((A4height * 2.0) * dx21);
		a = (((A4height* 2.0) * g + 1) * x1 - c) / (A4height * 2.0);
		d = (((A4height * 2.0) * g + 1) * y1 - f) / (A4height* 2.0);
		b = (((A4width * 2.0) * h + 1) * x3 - c) / (A4width * 2.0);
		e = (((A4width * 2.0) * h + 1) * y3 - f) / (A4width * 2.0);
	}
	else {
		h = ((dy32 + y1 - f) * dx21 - (dx32 + x1 - c) * dy21) / ((A4height * 2.0) * (dx32 * dy21 - dy32 * dx21));
		g = ((A4height * 2.0) * dx32 * h + (dx32 + x1 - c)) / ((A4width * 2.0) * dx21);
		a = (((A4width * 2.0) * g + 1) * x1 - c) / (A4width * 2.0);
		d = (((A4width * 2.0) * g + 1) * y1 - f) / (A4width * 2.0);
		b = (((A4height * 2.0) * h + 1) * x3 - c) / (A4height * 2.0);
		e = (((A4height * 2.0) * h + 1) * y3 - f) / (A4height * 2.0);
	}
	
	cout << "a, b, c, d, e, f, g, h" << endl;
	cout << a << " " << b << " " << c << " " << d << " " << e
		<< " " << f << " " << g << " " << h << " " << endl;

	int width = originA4Paper._width;
	int height = originA4Paper._height;
	cimg_forXY(A4Paper, x, y) {
		if (width > height) {
			double srcY = (a * (double)x + b * (double)y + c) / (g * (double)x + h * (double)y + 1);
			double srcX = (d * (double)x + e * (double)y + f) / (g * (double)x + h * (double)y + 1);
			if (srcY < 0) srcY = 0;
			if (srcY >= height) srcY = height - 1;
			if (srcX < 0) srcX = 0;
			if (srcX >= width) srcX = width - 1;
			vector<int> RGBvec;
			RGBvec = getInterpolationRGB(srcX, srcY);    //插值
			A4Paper(x, y, 0, 0) = RGBvec[0];
			A4Paper(x, y, 0, 1) = RGBvec[1];
			A4Paper(x, y, 0, 2) = RGBvec[2];
		}
		else {
			double srcX = (a * (double)x + b * (double)y + c) / (g * (double)x + h * (double)y + 1);
			double srcY = (d * (double)x + e * (double)y + f) / (g * (double)x + h * (double)y + 1);
			if (srcX < 0) srcX = 0;
			if (srcX >= width) srcX = width - 1;
			if (srcY < 0) srcY = 0;
			if (srcY >= height) srcY = height - 1;
			vector<int> RGBvec;
			RGBvec = getInterpolationRGB(srcX, srcY);    //插值
			A4Paper(x, y, 0, 0) = RGBvec[0];
			A4Paper(x, y, 0, 1) = RGBvec[1];
			A4Paper(x, y, 0, 2) = RGBvec[2];
		}

		/*
		double srcX = (a * (double)x + b * (double)y + c) / (g * (double)x + h * (double)y + 1);
		double srcY = (d * (double)x + e * (double)y + f) / (g * (double)x + h * (double)y + 1);
		if (srcX < 0) srcX = 0;
		if (srcX >= width) srcX = width - 1;
		if (srcY < 0) srcY = 0;
		if (srcY >= height) srcY = height - 1;

		vector<int> RGBvec;
		if (width > height) {
			RGBvec = getInterpolationRGB(srcY, srcX);    //插值
		} 
		else{
			RGBvec = getInterpolationRGB(srcX, srcY);    //插值
		}
		
		A4Paper(x, y, 0, 0) = RGBvec[0];
		A4Paper(x, y, 0, 1) = RGBvec[1];
		A4Paper(x, y, 0, 2) = RGBvec[2];
		*/
	}
	return A4Paper;
}

CImg<double> CorrectionForA4Paper::correct(string inputImageName, CImg<short int> edgeImage)
{
	originImage.load(inputImageName.c_str());
	houghSpaceImage = houghTransform(edgeImage);
	vector< Point> PointSet = getLinesFromHoughSpace(houghSpaceImage, originImage, houghThreshold);
	CImg<double> result = correctA4Paper(originImage, PointSet);
	return result;
}
  