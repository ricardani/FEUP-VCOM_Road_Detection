#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

Point RIGHT_LINE[2], LEFT_LINE[2], INTERSECT;

double cross(Point v1, Point v2) {
	return v1.x*v2.y - v1.y*v2.x;
}

bool getIntersectionPoint(Point a1, Point a2, Point b1, Point b2, Point & intPnt) {
	Point p = a1;
	Point q = b1;
	Point r(a2 - a1);
	Point s(b2 - b1);

	if (cross(r, s) == 0) { return false; }

	double t = cross(q - p, s) / cross(r, s);

	intPnt = p + t*r;

	return true;
}

double Slope(int x0, int y0, int x1, int y1) {
	return (double)(y1 - y0) / (x1 - x0);
}

void fullLine(Mat *img, Point a, Point b, Point new_point[]) {
	double slope = Slope(a.x, a.y, b.x, b.y);
	int line_thickness = 2;

	new_point[0] = Point(0, 0);
	new_point[1] = Point(img->cols, img->rows);

	new_point[0].y = -(a.x - new_point[0].x) * slope + a.y;
	new_point[1].y = -(b.x - new_point[1].x) * slope + b.y;
}

void drawLine(Mat *img, Point line[], Point intersection) {

	if (line[0].y > line[1].y) {
		cv::line(*img, line[0], intersection, Scalar(0, 0, 255), 2, CV_AA);
	}
	else {
		cv::line(*img, line[1], intersection, Scalar(0, 0, 255), 2, CV_AA);
	}


}

void detectLines(Mat original, Mat src) {
	int edgeThresh = 1;
	int lowThreshold = 50;
	int const maxThreshold = 400;
	int kernel_size = 3;

	Mat src_gray, dst, color_dst, copyOriginal;

	original.copyTo(copyOriginal);

	//Gaussian Blur
	cvtColor(src, src_gray, CV_RGB2GRAY);

	GaussianBlur(src_gray, dst, Size(kernel_size, kernel_size), 0, 0);

	//Canny
	Canny(dst, dst, lowThreshold, maxThreshold, kernel_size);

	src.copyTo(color_dst);

	vector<Vec4i> lines;
	// detect lines
	HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 100, 1);

	cvtColor(dst, dst, CV_GRAY2BGR);

	// draw lines
	int right_angle = 99999, left_angle = 0;

	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		double angle = atan2(l[3] - l[1], l[2] - l[0]) * 180.0 / CV_PI;
		angle = angle < 0 ? angle + 360 : angle;
		angle = angle > 180 ? angle - 180 : angle;

		if (angle > 10 && angle <= 45) {
			if (angle > left_angle) {
				fullLine(&copyOriginal, Point(l[0], l[1]), Point(l[2], l[3]), LEFT_LINE);
				left_angle = angle;
			}  
		}
		else if (angle < 170 && angle >= 135) {
			if (angle < right_angle) {
				fullLine(&copyOriginal, Point(l[0], l[1]), Point(l[2], l[3]), RIGHT_LINE);
				right_angle = angle;
			}
		}
	}

	getIntersectionPoint(LEFT_LINE[0], LEFT_LINE[1], RIGHT_LINE[0], RIGHT_LINE[1], INTERSECT);
	drawLine(&copyOriginal, LEFT_LINE, INTERSECT);
	drawLine(&copyOriginal, RIGHT_LINE, INTERSECT);


	circle(copyOriginal, INTERSECT, 10, Scalar(255, 255, 255), 3, 8);

	imshow("Probabilistic Hough", copyOriginal);
}

int main(int argc, char** argv)
{
	/*string imageName("road1.jpg");
	if (argc > 1)
	{
		imageName = argv[1];
	}

	Mat src;

	src = imread(imageName.c_str(), IMREAD_COLOR);

	int iLowH = 0, iHighH = 179;
	int iLowS = 0, iHighS = 80;
	int iLowV = 0, iHighV = 255;

	Mat imgOriginal = src;

	Mat imgHSV;

	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	Mat imgThresholded;

	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

	//morphological opening (removes small objects from the foreground)
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//morphological closing (removes small holes from the foreground)
	dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	Mat whiteImg = Mat::ones(imgThresholded.size(), imgThresholded.type()) * 255;

	//imshow("Thresholded Image", imgThresholded); //show the thresholded image

	Mat road;

	imgOriginal.copyTo(road, imgThresholded);

	//imshow("Original", road); //show the original image

	detectLines(src, road);

	waitKey(0);

	return 0;*/


	Mat src;
	Mat imgOriginal;

	int iLowH = 0, iHighH = 179;
	int iLowS = 0, iHighS = 80;
	int iLowV = 0, iHighV = 255;

	VideoCapture video("RoadDetectionVideo2.mp4");
	Mat thresh, final_mask, image_final, mask;

	if (!video.isOpened())
	{
		return 1;
	}

	while (video.isOpened())
	{
		bool success = video.read(src);

		mask = Mat::zeros(src.size(), CV_8UC3);

		rectangle(mask, Point(0, 2 * (mask.rows / 3)), Point(mask.cols, mask.rows), Scalar(255, 255, 255), CV_FILLED);
		cvtColor(mask, mask, CV_BGR2GRAY);

		src.copyTo(imgOriginal, mask);

		if (success) {
			Mat imgHSV;

			cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

			Mat imgThresholded;

			inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

			//morphological opening (removes small objects from the foreground)
			erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
			dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

			//morphological closing (removes small holes from the foreground)
			dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
			erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

			Mat whiteImg = Mat::ones(imgThresholded.size(), imgThresholded.type()) * 255;

			//imshow("Thresholded Image", imgThresholded); //show the thresholded image

			Mat road;

			imgOriginal.copyTo(road, imgThresholded);

			//imshow("Original", road); //show the original image

			detectLines(src, road);
		}

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
}