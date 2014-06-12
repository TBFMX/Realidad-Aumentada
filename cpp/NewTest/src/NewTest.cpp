//============================================================================
// Name        : NewTest.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include<opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/video/tracking.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<opencv2/legacy/legacy.hpp>
#include<opencv2/flann/miniflann.hpp>
#include<opencv2/stitching/stitcher.hpp>
#include<opencv2/stitching/warpers.hpp>
#include<opencv2/calib3d/calib3d.hpp>
#include<iostream>
#include "HomographyFinder.h"

void doomyTest(){
	// VideoCapture object
	cv::VideoCapture cap(0);
	cv::Mat augmentedImg = cv::imread("images/dragon.jpg");

	//~ cv::Mat sourceImg = cv::imread("images/template.jpg");
	cv::Mat markerImg = cv::imread("images/hiro.jpg");
	//~ cv::Mat sourceImg = cv::imread("images/abelHand.jpg");
	//~ cv::Mat sourceImg = cv::imread("images/whiteBox.jpg");
	//~ if (sourceImg.empty())
		//~ std::cout << "problem reading source image" << std::endl;
	//~ else
		//~ return;
	//~ //unsigned int frame_count = 0;
	cv::Mat goodTest = markerImg;
	cv::Mat unityM;
	unityM =  cv::Mat::zeros(3,3,CV_64F);
	unityM.at<double>(0,0) = 1.0;
	unityM.at<double>(1,1) = 1.0;
	unityM.at<double>(2,2) = 1.0;
	unsigned clearImageAfterNFrames = 90;
	//unsigned lastFramePersistance;
	unsigned frameNumber = 30;
	int WAITINGTIME = 30;
	double timeForFrame = 1.0/frameNumber;
	cv::Mat remainingMatrix = unityM;
	double cumulativeTime = 0.0;
	while(char(cv::waitKey(WAITINGTIME)) != 'q' && cap.isOpened()){
		double t0 = (double)cv::getTickCount();
		cv::Mat test;
		cap >> test;
		if(test.empty()){
			continue;
		}else{
			unityM.at<double>(0,2)= test.rows;
			unityM.at<double>(1,2)= test.cols;
			HomographyFinder a(augmentedImg, markerImg,test);
			// if maintaining homography matrix
			a.estimatePerspective();
			cv::Mat homographyM = a.getHomographyMatrix();
			if(a.isGoodHomographyMatrix()){
				remainingMatrix = homographyM;
			}
			if (cumulativeTime >= timeForFrame*clearImageAfterNFrames){
				cumulativeTime = 0.0;
				std::cout << timeForFrame*clearImageAfterNFrames << std::endl;
				remainingMatrix = unityM;
			}else{
				a.setHomographyMatrix(remainingMatrix);
			}
			// if new homography matrix change bestHomography and find new homography

			// else mantain the same homograpy but find a new one

			// show frame according to the frame rate and homography
			a.showMatches();
			a.showEstimatedPerspective();
			a.showAugmented();
			//~ goodTest = test;
		}
		t0 = ((double)cv::getTickCount() - t0) / cv::getTickFrequency();
		//~ t0 = (double)cv::getTickCount() - t0;
		cumulativeTime += t0;
		std::cout << "Frame rate = " << t0 << std::endl;
		std::cout << "time = " << t0 << std::endl;
	}
}

int main() {
	doomyTest();
	return 0;
}
