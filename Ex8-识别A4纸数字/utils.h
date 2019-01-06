#pragma once

//#include <iostream>
//#include <vector>



struct Point {   /*点的结构*/
	int x;
	int y;
	int value;
	Point(int _x, int _y, int _value) :x(_x), y(_y), value(_value) {};
	Point() { x = 0; y = 0; value = 0; };
	Point(const Point& a) { x = a.x; y = a.y; value = a.value; };
};


struct Line {
	double k;
	double b;
	Line(double _k, double _b) :k(_k), b(_b) {};
};

struct Triangle {
	Point a;  /*三个顶点*/
	Point b;
	Point c;
	int index[3]; /*三个顶点在特征点集中的索引*/
	Triangle(Point _a, Point _b, Point _c, int a_index, int b_index, int c_index) :
		a(_a), b(_b), c(_c) {
		index[0] = a_index;
		index[1] = b_index;
		index[2] = c_index;  // 记录特征点在特征点集中的索引
	}
	/*求三点形成的向量叉积*/
	int cross3(const Point &a, const Point &b, const Point &p) {
		return (b.x - a.x)*(p.y - a.y) - (b.y - a.y)*(p.x - a.x);
	}
	/*求两点距离*/
	double GetDistance(Point A, Point B) {
		double dis = (A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y);
		return sqrt(dis);
	}
	/*判断点是否三角形的顶点*/
	bool isVertex(Point p) {
		if (p.x == a.x && p.y == a.y)
			return true;
		if (p.x == b.x && p.y == b.y)
			return true;
		if (p.x == c.x && p.y == c.y)
			return true;
		return false;
	}

	bool operator==(const Triangle t) {  //重载运算符==
		// ...不确定点的对应关系，所以有3！= 6个可能。。。
		if (this->a.x == t.a.x && this->a.y == t.a.y && this->b.x == t.b.x && this->b.y == t.b.y
			&& this->c.x == t.c.x && this->c.y == t.c.y) {
			return true;  // a b c
		}
		if (this->a.x == t.a.x && this->a.y == t.a.y && this->b.x == t.c.x && this->b.y == t.c.y
			&& this->c.x == t.b.x && this->c.y == t.b.y) {
			return true;  // a c b
		}
		if (this->a.x == t.b.x && this->a.y == t.b.y && this->b.x == t.a.x && this->b.y == t.a.y
			&& this->c.x == t.c.x && this->c.y == t.c.y) {
			return true;   // b a c 
		}
		if (this->a.x == t.b.x && this->a.y == t.b.y && this->b.x == t.c.x && this->b.y == t.c.y
			&& this->c.x == t.a.x && this->c.y == t.a.y) {
			return true;  // b c a 
		}
		if (this->a.x == t.c.x && this->a.y == t.c.y && this->b.x == t.a.x && this->b.y == t.a.y
			&& this->c.x == t.b.x && this->c.y == t.b.y) {
			return true;   // c a b
		}
		if (this->a.x == t.c.x && this->a.y == t.c.y && this->b.x == t.b.x && this->b.y == t.b.y
			&& this->c.x == t.a.x && this->c.y == t.a.y) {
			return true;   // c b a
		}
		return false;
	}
	bool isInTriangle(const Point& p) {
		/*判断点是否在三角形内*/
			// 向量叉积和都为正或者都为负，那么点p就在三角形内部
		if (cross3(a, b, p) >= 0 && cross3(b, c, p) >= 0 && cross3(c, a, p) >= 0)
			return true;
		else if (cross3(a, b, p) <= 0 && cross3(b, c, p) <= 0 && cross3(c, a, p) <= 0)
			return true;
		else
			return false;
	}
};
