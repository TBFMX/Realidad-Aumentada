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
#include "geometryStructs.hpp"
#include "couch2.h"
#include "rossiniColors2.h"

////////////////////////////////////////////////////////////////////
// Standard includes:
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/egl.h> // requires ndk r5 or newer

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include "logger.h"
#define LOG_TAG    "DRAWING_RENDERER"
#define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

//~ void getImageFromPath(cv::Mat *rgbImage){
	//~ cv::Mat m_furnishImage = cv::imread("sdcard/Models/Couch/couch.jpg");
	//~ cvtColor(m_furnishImage,m_furnishImage,CV_BGR2RGB);
	//~ LOG_INFO("furnish copied");
	//~ m_furnishImage.copyTo(*rgbImage);
//~ }
#define maxObjSize2 1596

float outNormals2[maxObjSize2*3];
float outTexCoords2[maxObjSize2*2];
float outVertexes2[maxObjSize2*3];
float outColors2[maxObjSize2][4];
float _angle2 = 0.0;
point3 cameraOrigin2;
unsigned finalVertexSize2;

//~ float gW = 1024;
//~ float gH = 768;
//~ const GLfloat bgTextureVertices[] = { 0, 0, 0, gW, 0, 0, 0, gH, 0, gW, gH, 0, 
		//~ 0,0,0,1,0,0, // x
		//~ 0,0,0,0,1,0, // y
		//~ 0,0,0,0,0,1};;

ARDrawingContext::ARDrawingContext(){
	m_isBackgroundTextureInitialized = false;
	m_isFurnishTextureInitialized = false;
	m_isWindowUpdated = false;
	m_isPatternPresent = false;
	
	//~ m_backgroundImage;
	//~ m_furnishImage;
	m_width = 1024;
	m_height = 768;
	//~ m_textureId[2];
	m_isWindowUpdated = false;
	// tablet parameters
	float fx=628.6341119951087;
	float fy=628.7519411113429;
	float cx=325.3443919995285;
	float cy=236.0028199018263;
	CameraCalibration calibration(fx,fy,cx,cy);
	m_calibration = calibration;
	//~ cv::Mat furnishImage = cv::imread("sdcard/Models/Couch/couch.jpg");
	//~ cvtColor(furnishImage,furnishImage,CV_BGR2RGBA);
};

ARDrawingContext::ARDrawingContext(cv::Size frameSize, const CameraCalibration& c)
  : m_isBackgroundTextureInitialized(false)
  , m_isFurnishTextureInitialized(false)
  , m_calibration(c)
  ,m_isWindowUpdated(false)
  ,m_isPatternPresent(false)
  //~ , m_windowName(windowName)
{
    // Create window with OpenGL support
    //~ cv::namedWindow(windowName, cv::WINDOW_OPENGL);

    // Resize it exactly to video size
    //~ cv::resizeWindow(windowName, frameSize.width, frameSize.height);

    // Initialize OpenGL draw callback:
    //~ cv::setOpenGlContext(windowName);
    //~ cv::setOpenGlDrawCallback(windowName, ARDrawingContextDrawCallback, this);
    m_width = frameSize.width;
    m_height = frameSize.height;
    //~ if(m_furnishImage.empty()){
		//~ cv::Mat furnishImage = cv::imread("sdcard/Models/Couch/couch.jpg");
		//~ cvtColor(furnishImage,furnishImage,CV_BGR2RGBA);
	//~ }
}

ARDrawingContext::~ARDrawingContext()
{
    //~ cv::setOpenGlDrawCallback(m_windowName, 0, 0);
    glBindTexture(GL_TEXTURE_2D, 0); // Set the GL texture to NULL, standard cleanup
}

void ARDrawingContext::updateBackground(const cv::Mat& frame)
{
	//~ LOG_INFO("intenting to update background Image w,h = %d, %d", frame.cols, frame.rows);
  frame.copyTo(m_backgroundImage);
  setWidth(m_backgroundImage.cols);
  setHeight(m_backgroundImage.rows);

  //~ LOG_INFO("updating background Image");
}

void ARDrawingContext::updateFurnishImage()
{
	cv::Mat furnish = cv::imread("sdcard/Models/Couch/couch.jpg");
	furnish.copyTo(m_furnishImage);
	cvtColor(m_furnishImage,m_furnishImage,CV_BGR2RGB);
}

void ARDrawingContext::updateWindow()
{
	m_isWindowUpdated = true;
	//~ LOG_INFO("updating window or on_draw_frame");
    //~ cv::updateWindow(m_windowName);
}

void ARDrawingContext::setWidth(unsigned _width){
	m_width = _width;
	//~ LOG_INFO("updating width to %d", m_width);
}

void ARDrawingContext::setHeight(unsigned _height){
	m_height = _height;
	//~ LOG_INFO("updating height to %d", m_height);
}

bool ARDrawingContext::isWindowUpdated(){
	//~ LOG_INFO("returning is window updated...");
	return m_isWindowUpdated;
}

bool ARDrawingContext::isThereAPattern(){
	return m_isPatternPresent;
}

void ARDrawingContext::createTexture() {
	unsigned int numTextures = 2;
	// Initialize texture for background image
	if (!m_isBackgroundTextureInitialized)
	{
		  // we generate both textures
		LOG("Texture Created");
		glGenTextures(numTextures, m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId[0]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_isBackgroundTextureInitialized = true;
	}
	//~ getFurnishTexture(m_textureId[1]);
	getObjectTexture(m_textureId[1],m_furnishImage);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ARDrawingContext::destroyTexture() {
	LOG("Texture destroyed");
	glDeleteTextures(2, m_textureId);
	m_isFurnishTextureInitialized = false;
	m_isBackgroundTextureInitialized = false;
}

void ARDrawingContext::draw()
{
	getObjectTexture(m_textureId[0],m_backgroundImage);
	getObjectTexture(m_textureId[1],m_furnishImage);
	drawCameraFrame();                        // Render background
	drawAugmentedScene();                              // Draw AR
}

// !!!!!!!!!! change BGR to RGB !!!!!!!!!!!!!!
void ARDrawingContext::drawCameraFrame()
{
	int w = m_width;
	int h = m_height;
		
	//~ orthogonalStart();
	
	const GLfloat bgTextureVertices[] = { 0, 0, 0, w, 0, 0, 0, h, 0, w, h, 0 };
	glClearColor(0,0,0,0);
	const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
	const GLfloat proj[]              = { 0, -2.0f/w, 0, 0, -2.0f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

	//~ LOG_INFO("w,h, channels= %d,%d ",w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	startTexture(m_textureId[0]);
	startArrays();

	glVertexPointer(3, GL_FLOAT, 0, bgTextureVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

	glColor4f(1,1,1,1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin2);
	LOG_INFO("x=%f y=%f z=%f background ",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);
	endArrays();
	endTexture();
	glBindTexture(GL_TEXTURE_2D, 0);
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
	m_isPatternPresent = true;

    // Render model
    //~ drawCoordinateAxis();
    //~ drawCubeModel();
    //~ try{
		//~ drawFurnish();
	//~ }
	//~ catch(int e){
		//~ std::cout<< "problem " << e << std::endl;
	//~ }	
	glPushMatrix();
	GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin2);
	glPopMatrix();
	
	if(cameraOrigin2.z <= 1.0 || cameraOrigin2.z >=5.0)
		m_isPatternPresent = false;
	else
		LOG_INFO("Pattern is present"); 

//~ 
	//~ LOG_INFO("x=%f y=%f z=%f",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);
//~ 
  }else{
	  m_isPatternPresent = false;
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
	float scale = 1.0;
	static float lines[] = {0,0,0,scale,0,0, // x
							0,0,0,0,scale,0, // y
							0,0,0,0,0,scale}; // z

	GLfloat colors[] = {
                        //~ 0.0f, 0.0f, 0.0f, 0.0f,
                        //~ 0.0f, 0.0f, 0.0f, 0.0f,
                        //~ 0.0f, 0.0f, 0.0f, 0.0f,
                        //~ 0.0f, 0.0f, 0.0f, 0.0f,
                        1.0f, 0.0f, 0.0f, 1.0f,
                        1.0f, 0.0f, 0.0f, 1.0f,
                        0.0f, 1.0f, 0.0f, 1.0f,
                        0.0f, 1.0f, 0.0f, 1.0f,
                        0.0f, 0.0f, 1.0f, 1.0f,
                        0.0f, 0.0f, 1.0f, 1.0f
                    };
                    
    
	//~ glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//~ glClear(GL_COLOR_BUFFER_BIT);
	//~ glShadeModel(GL_SMOOTH);
	//~ glVertexPointer(3, GL_FLOAT, 0, lineX);
	glVertexPointer(3, GL_FLOAT, 0, lines);
	//~ glVertexPointer(3, GL_FLOAT, 0, bgTextureVertices);
	glColorPointer(4, GL_FLOAT, 0, colors);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	//~ glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1.0,1.0,1.0,1.0);
	glLineWidth(2);
	glDrawArrays(GL_LINES, 0, 6);
	//~ glDrawArrays(GL_LINES, 2, 4);
	//~ glDrawArrays(GL_LINES, 4, 10);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	//~ glFlush();                  
	//~ glLineWidth(2);
	//~ 
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
	float scale = 4.0;

	// Enable texture mapping stuff
	startTexture(m_textureId[1]);
	startArrays();

    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin2);

	LOG_INFO("x=%f y=%f z=%f, furnish",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);
	cameraOrigin2.x *= scale;
	cameraOrigin2.y *= scale;
	cameraOrigin2.z *= scale;
	getFacesNearToCamera(couchNumVerts2,cameraOrigin2,couchTexCoords2,COLORS2,couchVerts2,
	outTexCoords2,outColors2,outVertexes2,&finalVertexSize2);
	
	// scaling the vertexes withon any help
	scaling(scale, outVertexes2, outVertexes2,finalVertexSize2);
	
	glVertexPointer(3, GL_FLOAT, 0, outVertexes2);
    glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords2);
	
	//~ glColor4f(1.0f,1.0f,1.0f,1.0f);
	glDrawArrays(GL_TRIANGLES, 0, finalVertexSize2);

	endArrays();
    endTexture();
	
}
