//! [includes]
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	string imageName("road1.jpg");
	if (argc > 1)
	{
		imageName = argv[1];
	}

	Mat src, src_gray;
	Mat detected_edges, dst;

	src = imread(imageName.c_str(), IMREAD_COLOR);

	if (src.empty())
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	int edgeThresh = 1;
	int lowThreshold = 300;
	int const maxThreshold = 500;
	int kernel_size = 3;

	cvtColor(src, src_gray, CV_RGB2GRAY);

	GaussianBlur(src_gray, detected_edges, Size(kernel_size, kernel_size), 0, 0);

	Canny(detected_edges, detected_edges, lowThreshold, maxThreshold, kernel_size);

	dst = Scalar::all(0);

	src_gray.copyTo(dst, detected_edges);
	imshow("Canny", dst);

	waitKey(0);

	return 0;
}