#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

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
	HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 50, 10);

	cvtColor(dst, dst, CV_GRAY2BGR);

	// draw lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		double angle = atan2(l[3] - l[1], l[2] - l[0]) * 180.0 / CV_PI;
		angle = angle < 0 ? angle + 360 : angle;
		if ((angle > 10 && angle < 170) || (angle > 190 && angle < 350)) {
			line(copyOriginal, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, CV_AA);
			line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, CV_AA);
		}
	}

	imshow("Probabilistic Hough", copyOriginal);
	imshow("Canny", dst);
}

int main(int argc, char** argv)
{

	string imageName("road1.jpg");
	if (argc > 1)
	{
		imageName = argv[1];
	}

	Mat src;

	src = imread(imageName.c_str(), IMREAD_COLOR);

	namedWindow("Control", CV_WINDOW_KEEPRATIO); //create a window called "Control"

	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 80;

	int iLowV = 0;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);


	while (true)
	{
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

		imshow("Thresholded Image", imgThresholded); //show the thresholded image

		Mat road;

		imgOriginal.copyTo(road, imgThresholded);

		imshow("Original", road); //show the original image

		detectLines(src, road);

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;
}