/*****************************************************************************
*   MarkerDetector.hpp
*   Example_MarkerBasedAR
******************************************************************************
*   by Khvedchenia Ievgen, 5th Dec 2012
*   http://computer-vision-talks.com
******************************************************************************
*   Ch2 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

#ifndef MarkerBasedAR_MarkerDetector_h
#define MarkerBasedAR_MarkerDetector_h

////////////////////////////////////////////////////////////////////
// Standard includes:
#include <vector>
#include <opencv2/opencv.hpp>

////////////////////////////////////////////////////////////////////
// File includes:
//~ #include "BGRAVideoFrame.h"
#include "CameraCalibration.hpp"
#include "GeometryTypes.hpp"
#include "Marker.hpp"


////////////////////////////////////////////////////////////////////
// Forward declaration:
//~ class Marker;

/**
 * A top-level class that encapsulate marker detector algorithm
 */
class MarkerDetector
{
public:
	MarkerDetector();
  /**
   * Initialize a new instance of marker detector object
   * @calibration[in] - Camera calibration (intrinsic and distortion components) necessary for pose estimation.
   */
  MarkerDetector(CameraCalibration calibration);
  
  typedef std::vector<cv::Point> PointsVector;
  typedef std::vector<PointsVector> ContoursVector;
  
  //! Searches for markes and fills the list of transformation for found markers
  //~ void processFrame(const BGRAVideoFrame& frame);
  bool processFrame(const cv::Mat& bgraMat);
  
  const std::vector<Transformation>& getTransformations() const;
  
protected:

  //! Main marker detection routine
  //~ bool findMarkers(const BGRAVideoFrame& frame, std::vector<Marker>& detectedMarkers);
  bool findMarkers(const cv::Mat& bgraMat, std::vector<Marker>& detectedMarkers);

  //! Converts image to grayscale
  void prepareImage(const cv::Mat& bgraMat, cv::Mat& grayscale) const;

  //! Performs binary threshold
  void performThreshold(const cv::Mat& grayscale, cv::Mat& thresholdImg) const;

  //! Detects appropriate contours
  void findContours(cv::Mat& thresholdImg, ContoursVector& contours, int minContourPointsAllowed) const;

  //! Finds marker candidates among all contours
  void findCandidates(const ContoursVector& contours, std::vector<Marker>& detectedMarkers);
  
  //! Tries to recognize markers by detecting marker code 
  void recognizeMarkers(const cv::Mat& grayscale, std::vector<Marker>& detectedMarkers);

  //! Calculates marker poses in 3D
  void estimatePosition(std::vector<Marker>& detectedMarkers);

private:
  float m_minContourLengthAllowed;
  
  cv::Size markerSize;
  cv::Mat camMatrix;
  cv::Mat distCoeff;
  std::vector<Transformation> m_transformations;
  
  cv::Mat m_grayscaleImage;
  cv::Mat m_thresholdImg;  
  cv::Mat canonicalMarkerImage;

  ContoursVector           m_contours;
  std::vector<cv::Point3f> m_markerCorners3d;
  std::vector<cv::Point2f> m_markerCorners2d;
  CameraCalibration   m_calibration;
};

#endif