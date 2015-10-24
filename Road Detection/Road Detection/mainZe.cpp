#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <stdio.h>

using namespace cv;
using namespace std;

int const max_BINARY_value = 255;
int threshold_type = 1;

Mat fillRoad(Mat& final_mask, Mat image)
{
	Mat filled, image_final, blue;

	blue = Mat::zeros(image.size(), CV_8UC3);
	blue.setTo(cv::Scalar(255, 50, 0));

	final_mask.copyTo(filled);
	int color = 100;
	int filling = cv::floodFill(filled, cv::Point((final_mask.cols - 1) * 3 / 8, (final_mask.rows - 1) * 3 / 4), color, (cv::Rect*)0, cv::Scalar(), 200);
	if (filling < 0.1*filled.cols*filled.rows)
	{
		final_mask.copyTo(filled);
		filling = cv::floodFill(filled, cv::Point((final_mask.cols - 1) / 2, (final_mask.rows - 1) * 3 / 4), color, (cv::Rect*)0, cv::Scalar(), 200);
	}

	filling = cv::floodFill(filled, cv::Point((final_mask.cols - 1) / 2, (final_mask.rows - 1) * 3 / 4), color, (cv::Rect*)0, cv::Scalar(), 200);
	circle(final_mask, cv::Point((filled.cols - 1) / 2, (final_mask.rows - 1) * 3 / 4), 1, cv::Scalar(255, 255, 0));
	// show the output  

	//imshow("filled", filled);
	final_mask = Mat::zeros(final_mask.size(), CV_8UC3);
	for (int i = 0;i < final_mask.cols;i++)
	{
		for (int j = 0;j < final_mask.rows;j++)
		{
			if (filled.at<uchar>(j, i) == color)
			{
				final_mask.at<cv::Vec3b>(j, i)[0] = 255;
				final_mask.at<cv::Vec3b>(j, i)[1] = 255;
				final_mask.at<cv::Vec3b>(j, i)[2] = 255;
			}
		}
	}
	//imshow("fill", final_mask);
	// wait for user 


	image.copyTo(image_final);
	blue.copyTo(image_final, final_mask);

	//namedWindow("finalMask", WINDOW_KEEPRATIO);
	//imshow("finalMask", final_mask);

	//namedWindow("test", WINDOW_KEEPRATIO);
	//imshow("test", image_final);


	return image;
}

Mat roadSegmentation(Mat& image_gray)
{
	Mat mask, divided, thresh;
	mask = Mat::zeros(image_gray.size(), CV_8UC3);
	rectangle(mask, Point(0, mask.rows / 2), Point(mask.cols, mask.rows), Scalar(255, 255, 255), CV_FILLED);
	cvtColor(mask, mask, CV_BGR2GRAY);

	image_gray.copyTo(divided, mask);

	GaussianBlur(divided, divided, Size(5, 5), 0, 0);

	adaptiveThreshold(divided, thresh, max_BINARY_value, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5, 1.5);
	dilate(thresh, thresh, getStructuringElement(MORPH_RECT, Size(2, 2)));

	//namedWindow("Ithreshi", WINDOW_KEEPRATIO);
	//imshow("Ithreshi", thresh);

	erode(thresh, thresh, getStructuringElement(MORPH_RECT, Size(2, 2)));

	//namedWindow("threshi", WINDOW_KEEPRATIO);
	//imshow("threshi", thresh);

	return thresh;
}

Mat CannyHough(Mat& image_gray, Mat& thresh)
{
	Scalar tempVal = mean(image_gray);
	float mean = tempVal.val[0];

	//Canny
	Canny(thresh, thresh, 0.66*mean, 1.33*mean, 3);


	vector<Vec4i> lines;
	HoughLinesP(thresh, lines, 1, CV_PI / 180, 15, 2, 10);


	// draw lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		double angle = atan2(l[3] - l[1], l[2] - l[0]) * 180.0 / CV_PI;
		angle = angle < 0 ? angle + 360 : angle;
		if ((angle > 20 && angle < 80) || (angle > 100 && angle < 160) || (angle > 200 && angle < 260) || (angle > 280 && angle < 340)) {
			line(thresh, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 0), 1, CV_AA);
		}
	}

	//namedWindow("hough", WINDOW_KEEPRATIO);
	//imshow("hough", thresh);

	return thresh;
}


int main(int argc, char* argv[]) {
	Mat image, image_gray;
	int option;

	cout << "Road detector 3000" << endl;
	cout << "1->image" << endl;
	cout << "2->video" << endl;

	cin >> option;

	if (option == 1)
	{
		image = cv::imread("road4.jpg", 1);
		if (!image.data)
			return 0;

		// Read input image
		Mat thresh, final_mask, image_final;
		final_mask = Mat::zeros(image.size(), CV_8UC3);

		//convert grey
		cvtColor(image, image_gray, CV_BGR2GRAY);
		thresh = roadSegmentation(image_gray);
		final_mask = CannyHough(image_gray, thresh);
		image_final = fillRoad(final_mask, image);

		waitKey(0);

		return 0;
	}
	else
	{
		VideoCapture video("RoadDetectionVideo1.mp4");
		Mat thresh, final_mask, image_final;

		if (!video.isOpened())
		{
			return 1;
		}


		while (video.isOpened())
		{
			bool success = video.read(image);

			final_mask = Mat::zeros(image.size(), CV_8UC3);

			//convert grey
			cvtColor(image, image_gray, CV_BGR2GRAY);
			thresh = roadSegmentation(image_gray);
			final_mask = CannyHough(image_gray, thresh);
			image_final = fillRoad(final_mask, image);
		}
	}

}

/*int main(int argc, char* argv[]) {
	// Read input image
	Mat image, image_gray, mask, divided, thresh, finalMask,image_final,blue;
	image = cv::imread("road5.jpg", 1);
	if (!image.data)
		return 0;

	blue= Mat::zeros(image.size(), CV_8UC3);
	blue.setTo(cv::Scalar(0, 0, 255));

	mask = Mat::zeros(image.size(), CV_8UC3);
	finalMask = Mat::zeros(image.size(), CV_8UC3);
	rectangle(mask, Point(0, mask.rows / 2), Point(mask.cols, mask.rows), Scalar(255, 255, 255), CV_FILLED);
	cvtColor(mask, mask, CV_BGR2GRAY);


	//convert grey
	cvtColor(image, image_gray, CV_BGR2GRAY);

	image_gray.copyTo(divided, mask);

	GaussianBlur(divided, divided, Size(3, 3), 0, 0);

	for (int i = 0;i < divided.cols;i++)
	{
		for (int j = 0;j < divided.rows;j++)
		{
			if (divided.at<uchar>(j, i) >= 200)
				divided.at<uchar>(j, i) = 255;
		}
	}

	threshold(divided, thresh, 150, max_BINARY_value, 0);

	//morphological closing (fill small holes in the foreground)
	dilate(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//morphological closing (fill small holes in the foreground)
	dilate(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	namedWindow("threshi", WINDOW_KEEPRATIO);
	imshow("threshi", thresh);

	//Canny
	Canny(thresh, thresh, 15, 45, 3);

	vector<Vec4i> lines;
	HoughLinesP(thresh, lines, 1, CV_PI / 180, 15, 2, 10);


	// draw lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		double angle = atan2(l[3] - l[1], l[2] - l[0]) * 180.0 / CV_PI;
		angle = angle < 0 ? angle + 360 : angle;
		if ((angle > 20 && angle < 80) || (angle > 100 && angle < 160) || (angle > 200 && angle < 260) || (angle > 280 && angle < 340)) {
			//line(finalMask, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 255, 255), 1, CV_AA);
			line(thresh, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 0), 1, CV_AA);
		}
	}

	namedWindow("threshi1", WINDOW_KEEPRATIO);
	imshow("threshi1", thresh);

	thresh.copyTo(finalMask);
	//cvtColor(finalMask, finalMask, CV_BGR2GRAY);

	//draw between lines
	bool foundRight = false, foundLeft = false;
	int init, end;

	for (int i = 0;i < finalMask.rows;i++)
	{
		//find first right point
		for (int j = finalMask.cols / 2;j < finalMask.cols;j++)
		{

			if (finalMask.at<uchar>(i, j) > 200)
			{
				end = j;
				foundRight = true;
				break;
			}

		}

		for (int j = 0;j < finalMask.cols / 2;j++)
		{
			if (finalMask.at<uchar>(i, j) > 200)
			{
				init = j;
				foundLeft = true;
				break;
			}

		}

		if (foundRight&&foundLeft)
		{
			for (int j = init;j <= end;j++)
			{
				finalMask.at<uchar>(i, j) = 255;
			}
		}
		foundRight=false;
		foundLeft = false;
	}

	image.copyTo(image_final);
	blue.copyTo(image_final,finalMask);

	namedWindow("thresh", WINDOW_KEEPRATIO);
	imshow("thresh", thresh);

	namedWindow("test", WINDOW_KEEPRATIO);
	imshow("test", image_final);
	waitKey(0);
}*/





///////////////////////////////////////////////////////////////////////////////////////////////////////////////


