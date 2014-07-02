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
#include "ARDrawingContext.hpp"

////////////////////////////////////////////////////////////////////
// Standard includes:
#include <GLES/gl.h>
#include <GLES/glext.h>

void ARDrawingContextDrawCallback(void* param)
{
    ARDrawingContext * ctx = static_cast<ARDrawingContext*>(param);
    if (ctx)
    {
        ctx->draw();
    }
}

ARDrawingContext::ARDrawingContext(std::string windowName, cv::Size frameSize, const CameraCalibration& c)
  : m_isTextureInitialized(false)
  , m_isFurnishTextureInitialized(false)
  , m_calibration(c)
  , m_windowName(windowName)
{
    // Create window with OpenGL support
    //~ cv::namedWindow(windowName, cv::WINDOW_OPENGL);

    // Resize it exactly to video size
    //~ cv::resizeWindow(windowName, frameSize.width, frameSize.height);

    // Initialize OpenGL draw callback:
    //~ cv::setOpenGlContext(windowName);
    //~ cv::setOpenGlDrawCallback(windowName, ARDrawingContextDrawCallback, this);
}

ARDrawingContext::~ARDrawingContext()
{
    //~ cv::setOpenGlDrawCallback(m_windowName, 0, 0);
    glBindTexture(GL_TEXTURE_2D, 0); // Set the GL texture to NULL, standard cleanup
}

void ARDrawingContext::updateBackground(const cv::Mat& frame)
{
  frame.copyTo(m_backgroundImage);
}

void ARDrawingContext::updateWindow()
{
    //~ cv::updateWindow(m_windowName);
}

void ARDrawingContext::draw()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear entire screen:
  drawCameraFrame();                            // Render background
  drawAugmentedScene();                              // Draw AR
  glFlush();
}

// !!!!!!!!!! change BGR to RGB !!!!!!!!!!!!!!
void ARDrawingContext::drawCameraFrame()
{
}

void ARDrawingContext::drawAugmentedScene()
{
  // Init augmentation projection
  Matrix44 projectionMatrix;
  int w = m_backgroundImage.cols;
  int h = m_backgroundImage.rows;
  buildProjectionMatrix(m_calibration, w, h, projectionMatrix);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projectionMatrix.data);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (isPatternPresent)
  {
    // Set the pattern transformation
    Matrix44 glMatrix = patternPose.getMat44();
    glLoadMatrixf(reinterpret_cast<const GLfloat*>(&glMatrix.data[0]));

    // Render model
    drawCoordinateAxis();
    //~ drawCubeModel();
    //~ try{
		drawFurnish();
	//~ }
	//~ catch(int e){
		//~ std::cout<< "problem " << e << std::endl;
	//~ }
  }
}

void ARDrawingContext::buildProjectionMatrix(const CameraCalibration& calibration, int screen_width, int screen_height, Matrix44& projectionMatrix)
{
  float nearPlane = 0.01f;  // Near clipping distance
  float farPlane  = 100.0f;  // Far clipping distance

  // Camera parameters
  float f_x = calibration.fx(); // Focal length in x axis
  float f_y = calibration.fy(); // Focal length in y axis (usually the same?)
  float c_x = calibration.cx(); // Camera primary point x
  float c_y = calibration.cy(); // Camera primary point y

  projectionMatrix.data[0] = -2.0f * f_x / screen_width;
  projectionMatrix.data[1] = 0.0f;
  projectionMatrix.data[2] = 0.0f;
  projectionMatrix.data[3] = 0.0f;

  projectionMatrix.data[4] = 0.0f;
  projectionMatrix.data[5] = 2.0f * f_y / screen_height;
  projectionMatrix.data[6] = 0.0f;
  projectionMatrix.data[7] = 0.0f;

  projectionMatrix.data[8] = 2.0f * c_x / screen_width - 1.0f;
  projectionMatrix.data[9] = 2.0f * c_y / screen_height - 1.0f;
  projectionMatrix.data[10] = -( farPlane + nearPlane) / ( farPlane - nearPlane );
  projectionMatrix.data[11] = -1.0f;

  projectionMatrix.data[12] = 0.0f;
  projectionMatrix.data[13] = 0.0f;
  projectionMatrix.data[14] = -2.0f * farPlane * nearPlane / ( farPlane - nearPlane );        
  projectionMatrix.data[15] = 0.0f;
}


void ARDrawingContext::drawCoordinateAxis()
{
  static float lineX[] = {0,0,0,1,0,0};
  static float lineY[] = {0,0,0,0,1,0};
  static float lineZ[] = {0,0,0,0,0,1};

  glLineWidth(2);

  //~ glBegin(GL_LINES);
//~ 
  //~ glColor3f(1.0f, 0.0f, 0.0f);
  //~ glVertex3fv(lineX);
  //~ glVertex3fv(lineX + 3);
//~ 
  //~ glColor3f(0.0f, 1.0f, 0.0f);
  //~ glVertex3fv(lineY);
  //~ glVertex3fv(lineY + 3);
//~ 
  //~ glColor3f(0.0f, 0.0f, 1.0f);
  //~ glVertex3fv(lineZ);
  //~ glVertex3fv(lineZ + 3);
//~ 
  //~ glEnd();
}

// !!!!!!!!!!!!!!! change RGB to BGR
void ARDrawingContext::drawFurnish()
{

}
