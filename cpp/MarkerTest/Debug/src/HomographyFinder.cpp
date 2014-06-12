/*
 * HomographyFinder.cpp
 *
 *  Created on: 23/04/2014
 *      Author: abel
 */

#include "HomographyFinder.h"

HomographyFinder::HomographyFinder(cv::Mat augmentedImg,cv::Mat markerImg, cv::Mat frameImg){
	im = markerImg;
	gb_im_transformed = frameImg;
	gb_im_augmented = augmentedImg;
	unityM =  cv::Mat::zeros(3,3,CV_64F);
	unityM.at<double>(0,0) = 1.0;
	unityM.at<double>(1,1) = 1.0;
	unityM.at<double>(2,2) = 1.0;
	unityM.at<double>(0,2)= frameImg.rows;
	unityM.at<double>(1,2)= frameImg.cols;

	gbFrameImgRows = frameImg.rows;
	gbFrameImgCols = frameImg.cols;
	//~ std::cout << unityM << std::endl;
}

void HomographyFinder::estimatePerspective(){
	// Match ORB features to point correspondences between the images
	// clearing kp and t_kp keypoints
	//~ kp.clear();
	//~ t_kp.clear();
	//~ frameNumber = 1;
	//~ good_matches.clear();

	cv::Mat desc, t_desc, im_g, t_im_g;

	cv::cvtColor(im, im_g, CV_BGR2GRAY);
	cv::cvtColor(gb_im_transformed, t_im_g, CV_BGR2GRAY);

	cv::OrbFeatureDetector featureDetector;
	cv::OrbDescriptorExtractor featureExtractor;

	featureDetector.detect(im_g, kp);
	featureDetector.detect(t_im_g, t_kp);

	featureExtractor.compute(im_g, kp, desc);
	featureExtractor.compute(t_im_g, t_kp, t_desc);

	cv::flann::Index flannIndex(desc, cv::flann::LshIndexParams(12,20,1), cvflann::FLANN_DIST_EUCLIDEAN);
	//~ cv::flann::Index flannIndex(desc, cv::flann::LshIndexParams(12,20,1), cvflann::FLANN_DIST_HAMMING);
	cv::Mat match_idx(t_desc.rows, 2, CV_32SC1), match_dist(t_desc.rows, 2, CV_32FC1);
	flannIndex.knnSearch(t_desc, match_idx, match_dist, 2, cv::flann::SearchParams());

	//~ std::vector<cv::DMatch> good_matches;
	//~ points.clear();
	//~ points_transformed.clear();
	//~ good_matches.clear();
	for(int i = 0; i < match_dist.rows; i++){
		if(match_dist.at<float>(i,0) < 2.0 * match_dist.at<float>(i, 1)){
			cv::DMatch dm(i, match_idx.at<int>(i,0), match_dist.at<float>(i,0));
			good_matches.push_back(dm);
			points.push_back((kp[dm.trainIdx]).pt);
			points_transformed.push_back((t_kp[dm.queryIdx]).pt);
		}
	}
	// this line was added
	//~ if (good_matches.size() >= 20 ){
	std::cout << good_matches.size() << std::endl;
	if (good_matches.size() >= 10){
		M = cv::findHomography(points, points_transformed, CV_RANSAC, 2);
	}
	std::cout << "Estimated Perspective transform = " << M << std::endl;
}

bool HomographyFinder::isGoodHomographyMatrix(){
	return (M.cols > 0 && (abs(M.at<double>(0,2)) <= gbFrameImgRows) && (abs(M.at<double>(1,2)) <= gbFrameImgCols)
						 &&	(M.at<double>(0,1) * M.at<double>(1,0) <= 0.0)
						&& ((M.at<double>(0,0)*M.at<double>(0,0) + M.at<double>(1,0) * M.at<double>(1,0)) <= 1.0)
					   && ((M.at<double>(0,1)*M.at<double>(0,1) + M.at<double>(1,1) * M.at<double>(1,1)) <= 1.0)
					    );
};

cv::Mat HomographyFinder::getHomographyMatrix(){
	return M;
}

void HomographyFinder::setHomographyMatrix(cv::Mat newM){
	M = newM;
}

void HomographyFinder::showMatchesController(cv::Mat *im_show){
	cv::drawMatches(gb_im_transformed, t_kp, im, kp, good_matches, *im_show);
	std::cout << "controller im_show "<< (*im_show).rows << "," << (*im_show).cols << std::endl;
}

void HomographyFinder::showEstimatedPerspectiveController(cv::Mat *im_perspective_transformed){

	if(isGoodHomographyMatrix())
		cv::warpPerspective(im, *im_perspective_transformed, M, gb_im_transformed.size());
	else{
		cv::warpPerspective(im, *im_perspective_transformed, unityM, gb_im_transformed.size());
	}
}

void HomographyFinder::showAugmentedController(cv::Mat *im_perspective_transformed){
	//~ cv::Mat im_perspective_transformed;
	showEstimatedPerspectiveController(im_perspective_transformed);
	if(isGoodHomographyMatrix()){
		// this image is shown in perspective
		cv::warpPerspective(gb_im_augmented, gb_im_perspective_augmented, M, gb_im_transformed.size());

		cv::warpPerspective(im, *im_perspective_transformed, M, gb_im_transformed.size());
	}else{
		cv::warpPerspective(im, *im_perspective_transformed, unityM, gb_im_transformed.size());
	}
}

void HomographyFinder::showMatches(){
	cv::Mat im_show;
	showMatchesController(&im_show);
	std::cout << "im_show "<< im_show.rows << "," << im_show.cols << std::endl;
	imshow("ORB matches", im_show);
}

void HomographyFinder::showEstimatedPerspective(){
	cv::Mat im_perspective_transformed;
	showEstimatedPerspectiveController(&im_perspective_transformed);
	cv::imshow("Estimated Perspective transform", im_perspective_transformed);
}

void HomographyFinder::showAugmented(){
	cv::Mat im_perspective_augmented;
	showAugmentedController(&im_perspective_augmented);
	//~ cv::imshow("Augmented Perspective transform", gb_im_transformed + im_perspective_augmented);
	cv::imshow("Augmented Perspective transform", gb_im_transformed + gb_im_perspective_augmented);
}


