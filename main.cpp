#include <opencv2\opencv.hpp>
#include <iostream>
#include <cstdio>
#include <string>
#include "calcMaze.h"
#include "moveControl.h"

using namespace cv;
using namespace std;

void changeimg();
int m(int i, int j);
void cvThin(Mat* src, Mat* dst, int iterations = 1);
int getFirst(Point2f *pl, int hx, int hy, int tx, int ty);

VideoCapture cap(0);

Point2f win1Points[4];
Point2f testPoints[4];
Point2f originPoints[4];

bool beginTrack = false;
bool beginCalcMaze = false;
bool endCalcMaze = false;
bool test = false;
bool testend = false;

bool enableWin1 = true;
bool step1 = false;
bool enableWin2 = false;
bool step2 = false;
bool enableWin3 = false;
bool step3 = false;
int llen;
int trackObject = -1;

//图像背景二值化的阈值
int smin1 = 0, vmin1 = 0, vmax1 = 255;
int smin2 = 0, vmin2 = 20, vmax2 = 255;
int smin = 0, vmin = 0, vmax = 100;

Mat transImg;
Mat afterThin;
Mat maskbg;


int main()
{
	// 设定bin数目
	int histSize = 45;

	namedWindow("test", CV_WINDOW_AUTOSIZE);
	namedWindow("win4", CV_WINDOW_AUTOSIZE);
	namedWindow("win2", CV_WINDOW_AUTOSIZE);
	Mat frame;
	Mat hsvframe;
	Mat hsvsrc1;
	Mat hsvsrc2;
	Mat mask1;
	Mat mask2;
	Mat h_hist1, h_hist2;
	Mat backProject1, backProject2;
	Mat srcimg1;
	Mat srcimg2;

	Rect trackWindow1, trackWindow2;

	vector<Mat> splitsrcImg1;
	vector<Mat> splitsrcImg2;
	vector<Mat> splitImg;

	Point2f *pl;

	// 设定取值范围 ( H,S,V) 中的H
	float range[] = { 0, 180 };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;
	bool stop = false;
	bool changeIS = true;

	void on_mouse1(int event, int x, int y, int flags, void *ustc);
	void on_mouse2(int event, int x, int y, int flags, void *ustc);
	void on_mouse3(int event, int x, int y, int flags, void *ustc);
	setMouseCallback("test", on_mouse1, 0);
	setMouseCallback("win4", on_mouse2, 0);
	setMouseCallback("win2", on_mouse3, 0);

	if (!cap.isOpened()) return -1;

	int first = 0;
	int hx, hy, tx, ty, sox, soy, tax, tay, midx, midy;
	int k = 0;
	while (!stop)
	{
		//从摄像头获取帧
		cap >> frame;
		imshow("test", frame);

		if (step1)
		{
			Mat edge(frame.rows, frame.cols, frame.type());
			//创建一个3*3的单通道32位浮点矩阵保存变换数据
			Mat transMat(3, 3, CV_32FC3);
			//保存四个点的原始坐标
			//保存这四个点的新坐标
			Point2f newPoints[4];
			newPoints[0] = Point2f(0, 0);
			newPoints[1] = Point2f(0, frame.rows/2);
			newPoints[2] = Point2f(frame.cols/2 , 0);
			newPoints[3] = Point2f(frame.cols/2, frame.rows/2);

			transMat = getPerspectiveTransform(win1Points, newPoints);
			warpPerspective(frame, transImg, transMat, Size(newPoints[3].x, newPoints[3].y));
			imshow("win2", transImg);
			if (changeIS)
			{
				changeimg();
				enableWin2 = true;
				changeIS = false;
			}
		}

		int sx, sy, ex, ey;
		if (step2)
		{
			int m1x = testPoints[0].y;
			int m1y = testPoints[0].x;
			int m2x = testPoints[1].y;
			int m2y = testPoints[1].x;
			int i, j;
			for (i = m1x; i<m2x; i++)
			{
				for (j = m1y; j<m2y; j++)
					if (m(i, j) == 0) break;
				if (m(i, j) == 0) break;
			}
			sx = i;
			sy = j;
			m1x = testPoints[2].y;
			m1y = testPoints[2].x;
			m2x = testPoints[3].y;
			m2y = testPoints[3].x;
			for (i = m1x; i<m2x; i++)
			{
				for (j = m1y; j<m2y; j++)
					if (m(i, j) == 0) break;
				if (m(i, j) == 0) break;
			}
			int ex = i;
			int ey = j;
			pl = new Point2f[1000];
			calcMaze(sx, sy, ex, ey, afterThin.rows, afterThin.cols, pl, llen);
			step2 = false;
			enableWin3 = true;

		}




		if (step3)
		{
			//获取选定的初始目标区域
			if (trackObject < 0) {
				trackWindow1 = Rect(originPoints[0], originPoints[1]);
				trackWindow2 = Rect(originPoints[2], originPoints[3]);
				trackObject = 1;
				srcimg1 = transImg(trackWindow1);
				srcimg2 = transImg(trackWindow2);
				cvtColor(srcimg1, hsvsrc1, CV_RGB2HSV);
				cvtColor(srcimg2, hsvsrc2, CV_RGB2HSV);
				split(hsvsrc1, splitsrcImg1);
				split(hsvsrc2, splitsrcImg2);
				// 计算直方图:
				calcHist(&splitsrcImg1[0], 1, 0, Mat(), h_hist1, 1, &histSize, &histRange, uniform, accumulate);
				calcHist(&splitsrcImg2[0], 1, 0, Mat(), h_hist2, 1, &histSize, &histRange, uniform, accumulate);
				normalize(h_hist1, h_hist1, 0, 255, NORM_MINMAX);
				normalize(h_hist2, h_hist2, 0, 255, NORM_MINMAX);


			}

			//转换颜色空间

			cvtColor(transImg, hsvframe, CV_RGB2HSV);
			inRange(transImg, Scalar(vmin1, vmin1, vmin1), Scalar(vmax1, vmax1, vmax1), mask1);
			inRange(transImg, Scalar(vmin2, vmin2, vmin2), Scalar(vmax2, vmax2, vmax2), mask2);
			//其实可以添加两个mask来适配两个跟踪的物体

			//去除赛道背景
			//mask1 &= maskbg;
			//mask2 &= maskbg;
			//分离通道HSV
			split(hsvframe, splitImg);

			calcBackProject(&splitImg[0], 1, 0, h_hist1, backProject1, &histRange);
			calcBackProject(&splitImg[0], 1, 0, h_hist2, backProject2, &histRange);

			//去除背景的部分干扰:
			backProject1 &= mask1;
			backProject2 &= mask2;

			//对于两个目标进行跟踪
			RotatedRect trackBox1 = CamShift(backProject1, trackWindow1, TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 5, 1));
			ellipse(transImg, trackBox1, Scalar(0, 0, 255), 3, LINE_AA);

			RotatedRect trackBox2 = CamShift(backProject2, trackWindow2, TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 5, 1));
			ellipse(transImg, trackBox2, Scalar(0, 0, 255), 1, LINE_AA);

			if (trackWindow1.area() <= 1)
			{
				int cols = backProject1.cols, rows = backProject1.rows, r = (MIN(cols, rows) + 5) / 6;
				trackWindow1 = Rect(trackWindow1.x - r, trackWindow1.y - r, trackWindow1.x + r, trackWindow1.y + r) &Rect(0, 0, cols, rows);
			}

			if (trackWindow2.area() <= 1)
			{
				int cols = backProject2.cols, rows = backProject2.rows, r = (MIN(cols, rows) + 5) / 6;
				trackWindow2 = Rect(trackWindow2.x - r, trackWindow2.y - r, trackWindow2.x + r, trackWindow2.y + r) &Rect(0, 0, cols, rows);
			}



			//显示结果
			line(transImg, trackBox1.center, trackBox2.center, Scalar(255, 255, 255));
			imshow("test2", transImg);

			first++;

			hx = trackBox1.center.y;
			hy = trackBox1.center.x;
			tx = trackBox2.center.y;
			ty = trackBox2.center.x;
			midx = (hx + tx) / 2;
			midy = (hy + ty) / 2;
			if (first == 5)
			{
				sox = midx;
				soy = midy;
				k = getFirst(pl, hx, hy, tx, ty);
				tax = pl[k].x;
				tay = pl[k].y;
			}
			else {
				sox = midx;
				soy = midy;
				cout << "k: " << k << endl;
				cout << "distance:" << sqrt((tax - midx)*(tax - midx) + (tay - midy)*(tay - midy)) << endl;
				if (sqrt((tax - midx)*(tax - midx) + (tay - midy)*(tay - midy)) < reachtres)
				{
					k=(k+1)%llen;
					sox = midx;
					soy = midy;
					tax = pl[k].x;
					tay = pl[k].y;
				}
				cout << "tx ty: " << tx << ' ' << ty << endl;
				cout << "hx hy: " << hx << ' ' << hy << endl;
				cout << "sox soy: " << sox << ' ' << soy << endl;
				cout << "tax tay: " << tax << ' ' << tay << endl;
				changeDir(hx, hy, tx, ty, sox, soy, tax, tay);
			}
		}


		//退出程序
		if (waitKey(1) == 'q')
			stop = true;

	}

	return 0;
}


//鼠标事件的回调函数
void on_mouse1(int event, int x, int y, int flags, void *ustc)
{
	static int count = 0;
	if (enableWin1)
	{
		if (event == CV_EVENT_LBUTTONDOWN&&count < 4 && !step1)
		{
			cout << '(' << x << ',' << y << ')' << endl;
			win1Points[count] = Point2f(x, y);
			count++;
		}
		if (count == 4)
		{
			step1 = true;
			enableWin1 = false;
		}
	}

}

void on_mouse2(int event, int x, int y, int flags, void *ustc)
{
	static int count = 0;
	if (enableWin2)
	{
		if (count < 4 && !step2&&event == CV_EVENT_LBUTTONDOWN)
		{
			cout << '(' << x << ',' << y << ')' << endl;
			testPoints[count] = Point2f(x, y);
			count++;
		}
		if (count == 4)
		{
			enableWin2 = false;
			step2 = true;
		}
	}
}

void on_mouse3(int event, int x, int y, int flags, void *ustc)
{
	static int count = 0;
	if (enableWin3)
	{
		if (count < 4 && !step3&&event == CV_EVENT_LBUTTONDOWN)
		{
			cout << '(' << x << ',' << y << ')' << endl;
			originPoints[count] = Point2f(x, y);
			count++;
		}
		if (count == 4)
		{
			enableWin3 = false;
			step3 = true;
		}
	}
}

//图像投影变换
void changeimg()
{
	Mat mask;
	inRange(transImg, Scalar(0, smin, vmin), Scalar(180, 256, vmax), mask);
	for (int i = 0; i < mask.rows; i++) {
		for (int j = 0; j < mask.cols; j++) {
			mask.ptr<uchar>(i)[j] = (uchar)(255 - (int)mask.ptr<uchar>(i)[j]);
		}
	}
	maskbg = mask.clone();
	cvThin(&mask, &afterThin, 100);
	imshow("win4", afterThin);



}

void cvThin(Mat* src, Mat* dst, int iterations)
{
	src->copyTo(*dst);
	Mat t_image(src->clone());
	for (int n = 0; n < iterations; n++)
		for (int s = 0; s <= 1; s++) {
			dst->copyTo(t_image);
			for (int i = 0; i < src->rows; i++)
			{
				for (int j = 0; j < src->cols; j++)
				{
					if ((int)t_image.ptr<uchar>(i)[j]) {
						int a = 0, b = 0;
						int d[8][2] = { { -1, 0 },{ -1, 1 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 1, -1 },{ 0, -1 },{ -1, -1 } };
						int p[8];
						p[0] = (i == 0) ? 0 : (int)t_image.ptr<uchar>(i - 1)[j];
						for (int k = 1; k <= 8; k++) {
							if (i + d[k % 8][0] < 0 || i + d[k % 8][0] >= src->rows || j + d[k % 8][1] < 0 || j + d[k % 8][1] >= src->cols) p[k % 8] = 0;
							else p[k % 8] = (int)t_image.ptr<uchar>(i + d[k % 8][0])[j + d[k % 8][1]];
							if (p[k % 8]) {
								b++;
								if (!p[k - 1]) a++;
							}
						}
						if (b >= 2 && b <= 6 && a == 1) if (!s && !(p[2] && p[4] && (p[0] || p[6]))) dst->ptr<uchar>(i)[j] = 0;
						else if (s && !(p[0] && p[6] && (p[2] || p[4]))) dst->ptr<uchar>(i)[j] = 0;
					}
				}
			}

		}
}

int m(int i, int j)
{
	return 1 - ((int)afterThin.ptr<uchar>(i)[j]) / 255;
}

int getFirst(Point2f *pl, int hx, int hy, int tx, int ty)
{
	int l = 0;
	int vecx1 = hx - tx, vecy1 = hy - ty, vecx2 = pl[l].x - hx, vecy2 = pl[l].y - hy;
	while (vecx1*vecx2 + vecy1*vecy2 <= 0)
	{
		l++;
		vecx2 = pl[l].x - hx;
		vecy2 = pl[l].y - hy;
	}
	return l;
}
