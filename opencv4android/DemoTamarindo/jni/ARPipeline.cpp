/*****************************************************************************
*   Markerless AR desktop application.
******************************************************************************
*   by Khvedchenia Ievgen, 5th Dec 2012
*   http://computer-vision-talks.com
******************************************************************************
*   Ch3 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

////////////////////////////////////////////////////////////////////
// File includes:
#include "ARPipeline.hpp"
ARPipeline::ARPipeline(){
	// tablet parameters
	float fx=628.6341119951087;
	float fy=628.7519411113429;
	float cx=325.3443919995285;
	float cy=236.0028199018263;
	CameraCalibration calibration(fx,fy,cx,cy);
	m_calibration = calibration;
};

ARPipeline::ARPipeline(const cv::Mat& patternImage, const CameraCalibration& calibration)
  : m_calibration(calibration)
{
  m_patternDetector.buildPatternFromImage(patternImage, m_pattern);
  m_patternDetector.train(m_pattern);
}

void ARPipeline::setPatternImage(const cv::Mat& patternImage){
	m_patternDetector.buildPatternFromImage(patternImage, m_pattern);
	m_patternDetector.train(m_pattern);
}

bool ARPipeline::processFrame(const cv::Mat& inputFrame)
{
  bool patternFound = m_patternDetector.findPattern(inputFrame, m_patternInfo);

  if (patternFound)
  {
    m_patternInfo.computePose(m_pattern, m_calibration);
  }

  return patternFound;
}

const Transformation& ARPipeline::getPatternLocation() const
{
  return m_patternInfo.pose3d;
}
