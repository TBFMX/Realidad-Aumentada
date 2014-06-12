//============================================================================
// Name        : CamTest.cpp
// Author      : Abel Chavez
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/video/tracking.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<opencv2/legacy/legacy.hpp>
#include<iostream>
#include<iomanip>
#include<vector>

using namespace std;

void otherTest(){
	cv::VideoCapture cap(0);
	if(!cap.isOpened())
		return;
	cv::namedWindow("webcam");
	while(char(cv::waitKey(50)) != 'q'){
		cv::Mat frame;
		cap >> frame;
		cv::imshow("webcam",frame);
		if(frame.empty())
			break;
	}
}

int main() {
	otherTest();
	return 0;
}
