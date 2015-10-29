#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

Point RIGHT_LINE[2], LEFT_LINE[2], INTERSECT = Point(0,0);
int MENU_OPTION;
string FILENAME;

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

void detectLines(Mat original, Mat src, bool retry = false) {
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
	if (retry) {
		HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 50, 10);
	}
	else {
		HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 100, 1);
	}
	

	cvtColor(dst, dst, CV_GRAY2BGR);

	// draw lines
	int right_angle = 9999, left_angle = 0;

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

	if (INTERSECT == Point(0, 0) && MENU_OPTION == 1 && !retry) {
		detectLines(original, src, true);
		return;
	}

	if (INTERSECT != Point(0, 0)) {
		drawLine(&copyOriginal, LEFT_LINE, INTERSECT);
		drawLine(&copyOriginal, RIGHT_LINE, INTERSECT);
		circle(copyOriginal, INTERSECT, 10, Scalar(255, 255, 255), 3, 8);
	}

	imshow("Road Detection", copyOriginal);
}

void roadDetection(Mat src) {

	int iLowH = 0, iHighH = 179;
	int iLowS = 0, iHighS = 80;
	int iLowV = 0, iHighV = 255;

	Mat mask, imgOriginal, imgHSV, imgThresholded, whiteImg, road;

	mask = Mat::zeros(src.size(), CV_8UC3);

	rectangle(mask, Point(0, 1 * (mask.rows / 2)), Point(mask.cols, mask.rows), Scalar(255, 255, 255), CV_FILLED);
	cvtColor(mask, mask, CV_BGR2GRAY);

	src.copyTo(imgOriginal, mask);

	//Threshold the image
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); 

	//morphological opening (removes small objects from the foreground)
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//morphological closing (removes small holes from the foreground)
	dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	whiteImg = Mat::ones(imgThresholded.size(), imgThresholded.type()) * 255;

	imgOriginal.copyTo(road, imgThresholded);

	detectLines(src, road);
}

bool videoProcessing() {
	Mat src;
	Mat imgOriginal;

	VideoCapture video(FILENAME);
	Mat thresh, final_mask, image_final, mask;

	if (!video.isOpened())
	{
		cout << "Could not open or find the video" << endl;
		return false;
	}

	while (video.isOpened())
	{
		bool success = video.read(src);

		if (success) {
			roadDetection(src);
		}

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return true;
}

bool imageProcessing() {
	string imageName(FILENAME);

	Mat src;

	src = imread(imageName.c_str(), IMREAD_COLOR);

	if (src.empty())
	{
		cout << "Could not open or find the image" << endl;
		return false;
	}

	roadDetection(src);

	waitKey(0);

	return true;
}

void menu() {

	cout << "  ____                 _   ____       _            _   _             " << endl;
	cout << " |  _ \\ ___   __ _  __| | |  _ \\  ___| |_ ___  ___| |_(_) ___  _ __  " << endl;
	cout << " | |_) / _ \\ / _` |/ _` | | | | |/ _ \\ __/ _ \\/ __| __| |/ _ \\| '_ \\ " << endl;
	cout << " |  _ < (_) | (_| | (_| | | |_| |  __/ ||  __/ (__| |_| | (_) | | | |" << endl;
	cout << " |_| \\_\\___/ \\__,_|\\__,_| |____/ \\___|\\__\\___|\\___|\\__|_|\\___/|_| |_|" << endl;
	cout << endl;
	cout << "  _____  ___   ___   ___   ___   ___   ___  " << endl;
	cout << " |___ / / _ \\ / _ \\ / _ \\ / _ \\ / _ \\ / _ \\ " << endl;
	cout << "   |_ \\| | | | | | | | | | | | | | | | | | |" << endl;
	cout << "  ___) | |_| | |_| | |_| | |_| | |_| | |_| |" << endl;
	cout << " |____/ \\___/ \\___/ \\___/ \\___/ \\___/ \\___/ " << endl;
	cout << endl;

	string option;

	cout << "Choose an option:" << endl;
	cout << "1 - Image" << endl;
	cout << "2 - Video" << endl;
	cout << "0 - Exit" << endl;
	cin >> option;

	MENU_OPTION = atoi(option.c_str());

	if (MENU_OPTION == 0) {
		return;
	}

	cout << endl << "Filename:" << endl;
	cin >> FILENAME;

}

int main(int argc, char** argv)
{

	menu();

	switch (MENU_OPTION)
	{
	case 1:
		imageProcessing();
		break;
	case 2:
		videoProcessing();
		break;
	default:
		return 0;
		break;
	}

	return 0;
}