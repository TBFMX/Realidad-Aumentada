/*
 * HomographyFinder.h
 *
 *  Created on: 23/04/2014
 *      Author: abel
 */

#ifndef HOMOGRAPHYFINDER_H_
#define HOMOGRAPHYFINDER_H_

namespace Homography {

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

class HomographyFinder{
private:
	cv::Mat im, gb_im_transformed, gb_im_augmented, gb_im_perspective_augmented;
	std::vector<cv::Point2f> points, points_transformed;
	cv::Mat M;
	unsigned gbFrameImgRows, gbFrameImgCols;

	//unsigned frameNumber;
	std::vector<cv::DMatch> good_matches;
	std::vector<cv::KeyPoint> kp, t_kp;
	void showMatchesController(cv::Mat *im_show);
	cv::Mat unityM;
	void showEstimatedPerspectiveController(cv::Mat *im_perspective_transformed);
	void showAugmentedController(cv::Mat *im_augmented);
public:
	HomographyFinder(cv::Mat,cv::Mat, cv::Mat);
	void showMatches();
	void showEstimatedPerspective();
	void estimatePerspective();
	bool isGoodHomographyMatrix();
	cv::Mat getHomographyMatrix();
	void setHomographyMatrix(cv::Mat newM);
	void showAugmented();
};

} /* namespace Homography */

#endif /* HOMOGRAPHYFINDER_H_ */
