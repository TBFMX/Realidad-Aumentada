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

#ifndef ARDrawingContext_HPP
#define ARDrawingContext_HPP

////////////////////////////////////////////////////////////////////
// File includes:
#include "GeometryTypes.hpp"
#include "CameraCalibration.hpp"

////////////////////////////////////////////////////////////////////
// Standard includes:
#include <opencv2/opencv.hpp>

class ARDrawingContext
{
public:
	ARDrawingContext();
	ARDrawingContext(cv::Size frameSize, const CameraCalibration& c);
	~ARDrawingContext();

	bool                isPatternPresent;
	Transformation      patternPose;


	//! Set the new frame for the background
	void updateBackground(cv::Mat& frame);
	void updateFurnishImage();
	void updateTigerImage();
	void updateWindow();
	//! Render entire scene in the OpenGl window
	void draw();
	void drawPersistance();
	void setHeight(unsigned);
	void setWidth(unsigned);
	void createTexture();
	void destroyTexture();
	bool isWindowUpdated();
	void validatePatternPresent();
	//! Draws the background with video
	void drawCameraFrame(); 
	//! Draws the coordinate axis 
	void drawCoordinateAxis();
	//! Draws the AR
	void drawAugmentedScene();
	
	void drawAugmentedPersistance();
	
	//! Draw furnish
	void drawFurnish(int chosenObject);
	
	//! Draw tiger
	void drawTiger(int chosenObject);
	
	bool isThereAPattern();
	
	void setObjectToDraw(int objectId);
	int objectToDraw();
	void setAngle(float angle);
	float getAngle();
	void setScale(float scale);
	float getScale();
	void setTranslate(float x,float y);
	//~ void getTranslate(float position[2]);
	
private:


  //! Builds the right projection matrix from the camera calibration for AR
  void buildProjectionMatrix(const CameraCalibration& calibration, int w, int h, Matrix44& result);

  //! Draw the cube model
  void drawCubeModel();



  bool               m_isBackgroundTextureInitialized;
  bool 				 m_isFurnishTextureInitialized;
  bool 				 m_isTigerTextureInitialized;
  CameraCalibration  m_calibration;
  cv::Mat            m_backgroundImage;
  cv::Mat			 m_furnishImage;
  cv::Mat 			 m_tigerImage;
  unsigned int 		 m_width;
  unsigned int 		 m_height;
  unsigned int 		 m_textureId[3];
  unsigned int 		 m_isWindowUpdated;
  bool 				 m_isPatternPresent;
  int 				 m_objectToDraw;
  Matrix44			 m_persistentPose;
  Matrix44			 m_persistentProjection;
  float 			 m_angle;
  float 			 m_scale;
  float				 m_translateX;
  float				 m_translateY;
  //~ std::string        m_windowName;
};

#endif
