// Include OpenCV's C++ Interface
#include<opencv2/opencv.hpp>
#include "cartoon.h"
void cartoonifyImage(cv::Mat *srcColor,	cv::Mat *dst){
	cv::Mat gray;
	cv::cvtColor(*srcColor, gray, CV_BGR2GRAY);
	const int MEDIAN_BLUR_FILTER_SIZE = 7;
	medianBlur(gray,gray, MEDIAN_BLUR_FILTER_SIZE);
	cv::Mat edges;
	const int LAPLACIAN_FILTER_SIZE = 5;
	Laplacian(gray,edges, CV_8U, LAPLACIAN_FILTER_SIZE);
	
	cv::Mat mask;
	const int EDGES_THRESHOLD = 80;
	cv::threshold(edges, mask, EDGES_THRESHOLD, 255, cv::THRESH_BINARY_INV);
	
	cv::Size size = (*srcColor).size();
	cv::Size smallSize;
	smallSize.width = size.width/2;
	smallSize.height = size.height / 2;
	cv::Mat smallImg = cv::Mat(smallSize, CV_8UC4);
	cv::resize(*srcColor, smallImg, smallSize, 0,0,cv::INTER_LINEAR);
	
	cv::Mat tmp = cv::Mat(smallSize, CV_8UC4);
//	int repetitions = 7; // Repetitions for strong cartoon effect.
//	for(int i = 0; i < repetitions; i++){
//		int ksize = 9; // Filter size. Has a large effect on speed.
//		double sigmaColor = 9; // Filter color strength.
//		double sigmaSpace = 7; // Spatial strength. Affects speed.
//		bilateralFilter(smallImg, tmp, ksize, sigmaColor, sigmaSpace);
//		bilateralFilter(tmp, smallImg, ksize, sigmaColor, sigmaSpace);
//	}
	
	cv::Mat bigImg;
	cv::resize(smallImg, bigImg, size, 0,0,cv::INTER_LINEAR);
	(*dst).setTo(0);
	bigImg.copyTo(*dst,mask);
}
