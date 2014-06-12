#include<opencv2/opencv.hpp>

#ifndef _ORBANDROID_H
#define _ORBANDROID_H

class OrbAndroid {
private:
	cv::Mat im, im_transformed, im_perspective_transformed, im_augmented, im_perspective_augmented;
	std::vector<cv::Point2f> points, points_transformed;
	cv::Mat M;
	unsigned gbFrameImgRows, gbFrameImgCols;
	std::vector<cv::DMatch> good_matches;
	std::vector<cv::KeyPoint> kp, t_kp;
public:
	OrbAndroid();
	OrbAndroid(cv::Mat *markerImg);
	OrbAndroid(cv::Mat *markerImg, cv::Mat *frameImg);
	~OrbAndroid(){}
	void estimate_perspective();
	void getAugmented(cv::Mat *);
	void updateFrame(cv::Mat *);
	void updateMarker(cv::Mat *);
	bool letAugmentedBeShown();
};

#endif
