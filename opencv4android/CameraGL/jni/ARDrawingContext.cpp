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
#include "Tiger.hpp"
#include "BeautifulGirl.hpp"
#include "TrollHouse.hpp"

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
	//~ cv::Mat m_furnishImage = cv::imread("sdcard/Models/couch.jpg");
	//~ cvtColor(m_furnishImage,m_furnishImage,CV_BGR2RGB);
	//~ LOG_INFO("furnish copied");
	//~ m_furnishImage.copyTo(*rgbImage);
//~ }
//~ #define maxObjSize2 1596
#define maxObjSize2 112254

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
	m_isWindowUpdated = false;
	m_isPatternPresent = false;
	m_objectToDraw = 0;
	m_illuminationToDraw = 0;
	m_shadowTuner = 1.0;
	m_textureSize = 2;
	m_angle = 0.0;
	m_scale = 1.0;
	m_translateX = 0;
	m_translateY = 0;
	
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
	//~ cv::Mat furnishImage = cv::imread("sdcard/Models/couch.jpg");
	//~ cvtColor(furnishImage,furnishImage,CV_BGR2RGBA);
};

ARDrawingContext::ARDrawingContext(cv::Size frameSize, const CameraCalibration& c)
  : m_isBackgroundTextureInitialized(false)
  , m_calibration(c)
  ,m_isWindowUpdated(false)
  ,m_isPatternPresent(false)
  ,m_objectToDraw(0)
  ,m_illuminationToDraw(0)
  ,m_shadowTuner(1.0)
  ,m_textureSize(2)
  ,m_angle(0.0)
  ,m_scale(1.0)
  ,m_translateX(0)
  ,m_translateY(0)
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
		//~ cv::Mat furnishImage = cv::imread("sdcard/Models/couch.jpg");
		//~ cvtColor(furnishImage,furnishImage,CV_BGR2RGBA);
	//~ }
}

ARDrawingContext::~ARDrawingContext()
{
    //~ cv::setOpenGlDrawCallback(m_windowName, 0, 0);
    glBindTexture(GL_TEXTURE_2D, 0); // Set the GL texture to NULL, standard cleanup
}

void ARDrawingContext::updateBackground(cv::Mat& frame)
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
	furnish.copyTo(m_textureImages[0]);
	cvtColor(m_textureImages[0],m_textureImages[0],CV_BGR2RGB);
}

void ARDrawingContext::updateTigerImage()
{
	cv::Mat tiger = cv::imread("sdcard/Models/Bengal/bengalTiger.jpg");
	tiger.copyTo(m_textureImages[0]);
	cvtColor(m_textureImages[0],m_textureImages[0],CV_BGR2RGB);
}

void updateTextureImage(char pathToImage[],cv::Mat *outTextureImage){
	cv::Mat readedImage = cv::imread(pathToImage);
	readedImage.copyTo(*outTextureImage);
	cvtColor(*outTextureImage,*outTextureImage,CV_BGR2RGB);
}


void ARDrawingContext::updateTrollHouseImage()
{
	std::string pathToTexture = "sdcard/Models/TrollHouse/road_stripe.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[0]);
	pathToTexture = "sdcard/Models/TrollHouse/door_outer.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[1]);
	pathToTexture = "sdcard/Models/TrollHouse/door_interior.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[2]);
	pathToTexture = "sdcard/Models/TrollHouse/grass.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[3]);
	pathToTexture = "sdcard/Models/TrollHouse/door_garage.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[4]);
	pathToTexture = "sdcard/Models/TrollHouse/rock.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[5]);
	pathToTexture = "sdcard/Models/TrollHouse/sign.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[6]);
	pathToTexture = "sdcard/Models/TrollHouse/stone_wall.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[7]);
	pathToTexture = "sdcard/Models/TrollHouse/carpet_01.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[8]);
	pathToTexture = "sdcard/Models/TrollHouse/wood_wall.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[9]);
	pathToTexture = "sdcard/Models/TrollHouse/brick_wall_01.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[10]);
	pathToTexture = "sdcard/Models/TrollHouse/brick_wall_02.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[11]);
	pathToTexture = "sdcard/Models/TrollHouse/cement_02.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[12]);
	pathToTexture = "sdcard/Models/TrollHouse/driveway.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[13]);
	pathToTexture = "sdcard/Models/TrollHouse/floor_wood.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[14]);
	pathToTexture = "sdcard/Models/TrollHouse/floor_tiles_01.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[15]);
	pathToTexture = "sdcard/Models/TrollHouse/floor_tiles_02.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[16]);
	pathToTexture = "sdcard/Models/TrollHouse/carpet_01.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[17]);
	pathToTexture = "sdcard/Models/TrollHouse/carpet_02.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[18]);
	pathToTexture = "sdcard/Models/TrollHouse/roof.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[19]);
	pathToTexture = "sdcard/Models/TrollHouse/roof_wood.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[20]);
	pathToTexture = "sdcard/Models/TrollHouse/wall_interior_02.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[21]);
	pathToTexture = "sdcard/Models/TrollHouse/fence_wood.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[22]);
	pathToTexture = "sdcard/Models/TrollHouse/metal.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[23]);
	pathToTexture = "sdcard/Models/TrollHouse/sidewalk.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[24]);
}

void ARDrawingContext::updateBeautifulGirlImage()
{
	std::string pathToTexture = "sdcard/Models/BeautifulGirl/12c14c70.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[0]);
	pathToTexture = "sdcard/Models/BeautifulGirl/13932ef0.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[1]);
	pathToTexture ="sdcard/Models/BeautifulGirl/19d89130.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[2]);
	pathToTexture ="sdcard/Models/BeautifulGirl/16cecd10.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[3]);
	pathToTexture ="sdcard/Models/BeautifulGirl/16c2e0d0.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[4]);
	pathToTexture ="sdcard/Models/BeautifulGirl/12dbd6d0.jpg";
	updateTextureImage(&pathToTexture[0],&m_textureImages[5]);
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

void ARDrawingContext::setObjectToDraw(int objectId){
	m_objectToDraw = objectId;
	destroyTexture();
	switch(m_objectToDraw){
		case 0:
			m_textureSize = 2;
			updateFurnishImage();
			break;
		case 1: m_textureSize = 2;
			updateTigerImage();
			break;
		case 2: m_textureSize = 7;
			updateBeautifulGirlImage();
			break;
		case 3: m_textureSize = 26;
			updateTrollHouseImage();
			break;
		case 4: m_textureSize = 2;
			updateFurnishImage();
			break;
		case 5: m_textureSize = 2;
			updateTigerImage();
			break;
		default:
			updateFurnishImage();
			m_textureSize = 2;
			break;
	}
	createTexture();
}

void ARDrawingContext::setIlluminationToDraw(int illumination){
	m_illuminationToDraw = illumination;
	switch(m_illuminationToDraw){
		case 0:
			m_shadowTuner = 1.0;
			break;
		case 1:
			m_shadowTuner = 0.8;
			break;
		case 2:
			m_shadowTuner = 0.6;
			break;
		case 3:
			m_shadowTuner = 0.4;
			break;
		case 4:
			m_shadowTuner = 0.2;
			break;
		case 5:
			m_shadowTuner = 0.0;
			break;
		default:
			m_shadowTuner = 1.0;
			break;
	}
}

int ARDrawingContext::objectToDraw(){
	return m_objectToDraw;
}

int ARDrawingContext::illuminationToDraw(){
	return m_illuminationToDraw;
}

bool ARDrawingContext::isThereAPattern(){
	return m_isPatternPresent;
}

float ARDrawingContext::getAngle(){
	return m_angle;
}

void ARDrawingContext::setAngle(float angle){
	if (angle > 360)
		angle = angle - 360;
	else if(angle < - 360)
		angle = angle + 360;

	m_angle = angle;
}

float ARDrawingContext::getScale(){
	return m_scale;
}

void ARDrawingContext::setScale(float scale){
	if (scale < 1.0)
		scale = 1.0;
	else if (scale > 15)
		scale =  15;
	m_scale = scale;
}

//~ void ARDrawingContext::getTranslate(float position[2]){
	//~ position[0] = m_translate[0];
	//~ position[1] = m_translate[1];
//~ }

void ARDrawingContext::setTranslate(float x,float y){
	m_translateX = x;
	m_translateY = y;
}

void ARDrawingContext::createTexture() {
	// Initialize texture for background image
	if (!m_isBackgroundTextureInitialized)
	{
		  // we generate both textures
		//~ LOG("Texture Created");
		glGenTextures(m_textureSize, m_textureId);
		//~ glBindTexture(GL_TEXTURE_2D, m_textureId[0]);

		//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_LINEAR
		m_isBackgroundTextureInitialized = true;
		unsigned int m_depthRenderbuffer = 0;
	}
	//~ getFurnishTexture(m_textureId[1]);
	//~ getObjectTexture(m_textureId[1],m_furnishImage);
	//~ getObjectTexture(m_textureId[2],m_tigerImage);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ARDrawingContext::destroyTexture() {
	//~ LOG("Texture destroyed");
	glDeleteTextures(m_textureSize, m_textureId);
	m_isBackgroundTextureInitialized = false;
}

void ARDrawingContext::draw()
{
	//~ createTexture();
	getObjectTexture(m_textureId[0],m_backgroundImage);
	for(unsigned i = 1; i < m_textureSize; ++i)
		getObjectTexture(m_textureId[i],m_textureImages[i-1]);
	drawCameraFrame();                        // Render background
	validatePatternPresent();
	drawAugmentedScene();                              // Draw AR
	//~ destroyTexture();
}

void ARDrawingContext::drawPersistance()
{
	//~ createTexture();
	getObjectTexture(m_textureId[0],m_backgroundImage);
	for(unsigned i = 1; i < m_textureSize; ++i)
		getObjectTexture(m_textureId[i],m_textureImages[i-1]);
	drawCameraFrame();                        // Render background
	//~ drawPersistance();                              // Draw AR
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
	//~ LOG_INFO("x=%f y=%f z=%f background ",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);
	endArrays();
	endTexture();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ARDrawingContext::validatePatternPresent(){
	if (isPatternPresent)
	{
		Matrix44 projectionMatrix;
		int w = m_backgroundImage.cols;
		int h = m_backgroundImage.rows;
		buildProjectionMatrix(m_calibration, w, h, projectionMatrix);		
		// Set the pattern transformation
		//~ Matrix44 glMatrix = patternPose.getMat44();
		m_persistentProjection = projectionMatrix; // saving a copy of projection
		m_persistentPose = patternPose.getMat44(); // saving a copy of pose
		
		glPushMatrix();
		glLoadMatrixf(reinterpret_cast<const GLfloat*>(&m_persistentPose.data[0]));
		GLfloat mdl[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
		getCameraOrigin(mdl,&cameraOrigin2);
		glPopMatrix();
		
		//~ LOG_INFO("augmented scene");
		if(cameraOrigin2.z <= 0.5) // || cameraOrigin2.z >=10.0)
			m_isPatternPresent = false;
		else
			m_isPatternPresent = true;

	}else{
		m_isPatternPresent = false;
	}
}

float normaVector(point3 *aVector){
	return sqrtf((*aVector).x * (*aVector).x + (*aVector).y * (*aVector).y);
}

float pointProduct(point3 *a,point3 *b){
	return (*a).x * (*b).x + (*a).y * (*b).y;
}

void tabletToOpengl(point3 *fingerInTablet, point3 *fingerInOpengl, point3 *fingerOrientation, point3 *modelOrientation){

	// angle between tiger and light axes
	float divNormal = normaVector(fingerOrientation)*normaVector(modelOrientation);
	
	float preData =pointProduct(modelOrientation,fingerOrientation)/divNormal;
	float theta = 0;
	
	if(preData > 1)
		preData = 1.0;
	else if (preData < -1.0)
		preData = -1.0;

	theta = acosf(preData);
	
	point3 positiveAngle;
	positiveAngle.x = -(*modelOrientation).y;
	positiveAngle.y = (*modelOrientation).x;
	positiveAngle.z = 0;
	if(pointProduct(fingerOrientation,&positiveAngle) > 0)
		theta = -theta;

	(*fingerInOpengl).x = (*fingerInTablet).x * cosf(theta) + (*fingerInTablet).y * sinf(theta);
	(*fingerInOpengl).y = -(*fingerInTablet).x * sinf(theta) + (*fingerInTablet).y * cosf(theta);
	(*fingerInOpengl).z = (*fingerInTablet).z;
}

void ARDrawingContext::drawAugmentedScene()
{
	// Init augmentation projection
	glClear(GL_DEPTH_BUFFER_BIT); // this is the trickiest command

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_persistentProjection.data);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glLoadMatrixf(reinterpret_cast<const GLfloat*>(&m_persistentPose.data[0]));
	
	glPushMatrix();
    GLfloat mdl[16];
    point3 cameraOrigin;
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	glPopMatrix();
	
	point3 modelOrientation;
	modelOrientation.x = mdl[0];
	modelOrientation.y = mdl[1];
	modelOrientation.z = 0;
	// we correct the directions in order to match camera orientation
	point3 fingerInTablet;
	fingerInTablet.x = -m_translateX*m_scale;
	fingerInTablet.y = -m_translateY*m_scale;
	fingerInTablet.z = 0;
	
	point3 fingerInOpengl;
	point3 fingerOrientation; // this orientation corresponds to camera orientation
	fingerOrientation.x = 1;
	fingerOrientation.y = 0;
	fingerOrientation.z = 0;
	
	tabletToOpengl(&fingerInTablet,&fingerInOpengl,&fingerOrientation,&modelOrientation);
	
	glTranslatef(fingerInOpengl.x,fingerInOpengl.y,0);
	
	glRotatef(m_angle,0,0,1);
	
	drawCoordinateAxis();
	
	switch(m_objectToDraw){
		case 0: drawFurnish();
			break;
		case 1: drawTiger();
			break;
		case 2: drawBeautifulGirl();
			break;
		case 3: drawTrollHouse();
			break;
		case 4: drawFurnish();
			break;
		case 5: drawTiger();
			break;
		default:
			drawFurnish();
			break;
	}
}


void ARDrawingContext::drawAugmentedPersistance()
{
	//~ LOG_INFO("augmented persistance");
	// Init augmentation projection
	glClear(GL_DEPTH_BUFFER_BIT); // this is the trickiest command
	//~ glMatrixMode(GL_MODELVIEW);
	//~ glLoadIdentity();
	//~ glRotatef(m_angle,0,0,1);
	// Render model
	drawCoordinateAxis();
	//~ drawCubeModel();
	//~ try{
		//~ drawFurnish();
	switch(m_objectToDraw){
		case 0: drawFurnish();
			break;
		case 1: drawTiger();
			break;
		case 2: drawBeautifulGirl();
			break;
		case 3: drawTrollHouse();
			break;
		case 4: drawFurnish();
			break;
		case 5: drawTiger();
			break;
		default:
			drawFurnish();
			break;
	}
}


void ARDrawingContext::buildProjectionMatrix(const CameraCalibration& calibration, int screen_width, int screen_height, Matrix44& projectionMatrix)
{
	float nearPlane = 0.01f;  // Near clipping distance
	float farPlane  = 100.0f;  // Far clipping distance
	
	Matrix33 cameraMatrix=calibration.getIntrinsic();
    // Camera parameters
    float f_x = cameraMatrix.data[0]; // Focal length in x axis
    float f_y = cameraMatrix.data[4]; // Focal length in y axis (usually the same?)
    float c_x = cameraMatrix.data[2]; // Camera primary point x
    float c_y = cameraMatrix.data[5]; // Camera primary point y

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
}

// !!!!!!!!!!!!!!! change RGB to BGR
void ARDrawingContext::drawFurnish()
{	
	float shadowTuner = m_shadowTuner; // values from 0.0 to 1.0
	//~ float scale = 4.0;
	float scale = m_scale;

	// Enable texture mapping stuff
	//~ glBlendFunc (GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	
	//~ glClearColor(0.0,0.0,0.0,0.0);
	
	//~ glEnable(GL_LINE_SMOOTH);
	//~ glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glEnable(GL_BLEND);
	//~ glEnable(GL_MULTISAMPLE);

	startTexture(m_textureId[1]);
	startArrays();

	glPushMatrix();
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin2);
	glPopMatrix();
	//~ LOG_INFO("x=%f y=%f z=%f, furnish",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);
	
	point3 zeroPoint;
	zeroPoint.x = 0.0;
	zeroPoint.y = 0.0;
	zeroPoint.z = 0.0;
	
	float distanceZC = getDistance(zeroPoint,cameraOrigin2);
	
	float scalatorViewPoint = 1.0;
	float perfectViewPoint = 1.0;
	if(distanceZC > 0)
		scalatorViewPoint = perfectViewPoint/distanceZC;
		
	cameraOrigin2.x *= scalatorViewPoint;
	cameraOrigin2.y *= scalatorViewPoint;
	cameraOrigin2.z *= scalatorViewPoint;

	//~ getFacesNearToCamera(couchNumVerts2,cameraOrigin2,couchTexCoords2,COLORS2,couchVerts2,
	//~ outTexCoords2,outColors2,outVertexes2,&finalVertexSize2);
	getAllSortedFaces(couchNumVerts2,cameraOrigin2,couchTexCoords2,couchVerts2,
	outTexCoords2,outVertexes2,&finalVertexSize2);
	
	// scaling the vertexes withon any help
	scaling(scale, outVertexes2, outVertexes2,finalVertexSize2);
	
	glVertexPointer(3, GL_FLOAT, 0, outVertexes2);
    glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords2);

	
	//~ glColor4f(1.0f,1.0f,1.0f,0.95f);
	glColor4f(shadowTuner,shadowTuner,shadowTuner,1.0f);
	glDrawArrays(GL_TRIANGLES, 0, finalVertexSize2);

	endArrays();
    endTexture();
    //~ glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    //~ glDisable(GL_LINE_SMOOTH);
}

void ARDrawingContext::drawTiger()
{	
	float shadowTuner = m_shadowTuner; // values from 0.0 to 1.0
	//~ float scale = 4.0;
	//~ float scale = 12.0;
	float scale = m_scale;
	
		
	//~ glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	//~ glEnable(GL_MULTISAMPLE);
	// Enable texture mapping stuff
	startTexture(m_textureId[1]);
	startArrays();

	glPushMatrix();
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin2);
	glPopMatrix();
	//~ LOG_INFO("x=%f y=%f z=%f, furnish",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);
	
	point3 zeroPoint;
	zeroPoint.x = 0.0;
	zeroPoint.y = 0.0;
	zeroPoint.z = 0.0;
	
	float distanceZC = getDistance(zeroPoint,cameraOrigin2);
	
	float scalatorViewPoint = 1.0;
	float perfectViewPoint = 1.0;
	if(distanceZC > 0)
		scalatorViewPoint = perfectViewPoint/distanceZC;
		
	cameraOrigin2.x *= scalatorViewPoint;
	cameraOrigin2.y *= scalatorViewPoint;
	cameraOrigin2.z *= scalatorViewPoint;

	//~ getFacesNearToCamera(couchNumVerts2,cameraOrigin2,couchTexCoords2,COLORS2,couchVerts2,
	//~ outTexCoords2,outColors2,outVertexes2,&finalVertexSize2);
	getAllSortedFaces(TigerVertices,cameraOrigin2,TigerTexels,TigerPositions,
	outTexCoords2,outVertexes2,&finalVertexSize2);
	
	// scaling the vertexes withon any help
	scaling(scale, outVertexes2, outVertexes2,finalVertexSize2);
	
	glVertexPointer(3, GL_FLOAT, 0, outVertexes2);
    glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords2);

	
	//~ glColor4f(0.9f,0.9f,0.9f,1.0f);
	glColor4f(shadowTuner,shadowTuner,shadowTuner,1.0f);
	glDrawArrays(GL_TRIANGLES, 0, finalVertexSize2);

	endArrays();
    endTexture();
    //~ glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    
    //~ glDisable(GL_LINE_SMOOTH);
}

void ARDrawingContext::drawBeautifulGirl(){
	float outTexCoords2[BeautifulGirlVertexes*2];
	float outVertexes2[BeautifulGirlVertexes*3];
	float outNormals2[BeautifulGirlVertexes*3];
	unsigned inTexBinder[BeautifulGirlVertexes];
	unsigned outTexBinder[BeautifulGirlVertexes];
	unsigned finalVertexSize2;
		
	float shadowTuner = m_shadowTuner; // values from 0.0 to 1.0
	//~ float scale = 4.0;
	float scale = m_scale;
	//~ glEnable(GL_LINE_SMOOTH);
	//~ glDisable(GL_COLOR_MATERIAL);
	//~ startBeautifulGirlLight(lPosition);
	glEnable(GL_BLEND);
	//~ glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_DST_ALPHA,GL_ONE_MINUS_DST_ALPHA);
	//~ glBlendFunc(GL_SRC_COLOR,GL_SRC_COLOR);
	//~ glEnable(GL_MULTISAMPLE);
	// Enable texture mapping stuff

	glEnableClientState(GL_NORMAL_ARRAY);
	startArrays();

	glPushMatrix();
	point3 cameraOrigin;
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin);
	glPopMatrix();
	//~ LOG_INFO("x=%f y=%f z=%f, furnish",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);

	point3 zeroPoint;
	zeroPoint.x = 0.0;
	zeroPoint.y = 0.0;
	zeroPoint.z = 0.0;

	float distanceZC = getDistance(zeroPoint,cameraOrigin);

	float scalatorViewPoint = 1.0;
	float perfectViewPoint = 1.0;
	if(distanceZC > 0)
		scalatorViewPoint = perfectViewPoint/distanceZC;

	cameraOrigin.x *= scalatorViewPoint;
	cameraOrigin.y *= scalatorViewPoint;
	cameraOrigin.z *= scalatorViewPoint;

	getTextureBinders(BeautifulGirlMaterials, m_textureId, BeautifulGirlFirsts,BeautifulGirlCounts,inTexBinder);
	//~ getFacesNearToCamera(couchNumVerts2,cameraOrigin2,couchTexCoords2,COLORS2,couchVerts2,
	//~ outTexCoords2,outColors2,outVertexes2,&finalVertexSize2);
	getAllSortedFacesMT(BeautifulGirlVertexes,cameraOrigin,BeautifulGirlTexels,BeautifulGirlPositions,BeautifulGirlNormals,
	outTexCoords2,outVertexes2,outNormals2,&finalVertexSize2,inTexBinder,outTexBinder);

	// scaling the vertexes withon any help
	scaling(scale, outVertexes2, outVertexes2,finalVertexSize2);

	glNormalPointer(GL_FLOAT,0,outNormals2);
	glVertexPointer(3,GL_FLOAT, 0, outVertexes2);
    glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords2);

	glEnable(GL_TEXTURE_2D);
	glColor4f(shadowTuner,shadowTuner,shadowTuner,1.0f);
	//we divide vertex size over 3 because each face have 3 vertexes
	for(unsigned iFace = 0; iFace < finalVertexSize2 / 3; ++iFace){
		glBindTexture(GL_TEXTURE_2D, outTexBinder[iFace]);
		glDrawArrays(GL_TRIANGLES, 3*iFace, 3); //each face have 3 vertexes
	}
	glDisable(GL_TEXTURE_2D);

	endArrays();
	glDisableClientState(GL_NORMAL_ARRAY);

    //~ glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    //~ endBeautifulGirlLight();
    //~ glDisable(GL_LINE_SMOOTH);	
}

void ARDrawingContext::drawTrollHouse()
{
	float outTexCoords2[TrollHouseVertexes*2];
	float outVertexes2[TrollHouseVertexes*3];
	float outNormals2[TrollHouseVertexes*3];
	unsigned inTexBinder[TrollHouseVertexes];
	unsigned outTexBinder[TrollHouseVertexes];
	unsigned finalVertexSize2;
		
	float shadowTuner = m_shadowTuner; // values from 0.0 to 1.0
	//~ //float scale = 4.0;
	float scale = m_scale;
	//~ //glEnable(GL_LINE_SMOOTH);
	//~ //glDisable(GL_COLOR_MATERIAL);
	//~ //startTrollHouseLight(lPosition);
	glEnable(GL_BLEND);
	//~ //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_DST_ALPHA,GL_ONE_MINUS_DST_ALPHA);
	//~ //glBlendFunc(GL_SRC_COLOR,GL_SRC_COLOR);
	//~ //glEnable(GL_MULTISAMPLE);
	// Enable texture mapping stuff

	glEnableClientState(GL_NORMAL_ARRAY);
	startArrays();

	glPushMatrix();
	point3 cameraOrigin;
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin);
	glPopMatrix();
	//~ //LOG_INFO("x=%f y=%f z=%f, furnish",cameraOrigin2.x,cameraOrigin2.y,cameraOrigin2.z);

	point3 zeroPoint;
	zeroPoint.x = 0.0;
	zeroPoint.y = 0.0;
	zeroPoint.z = 0.0;

	float distanceZC = getDistance(zeroPoint,cameraOrigin);

	float scalatorViewPoint = 1.0;
	float perfectViewPoint = 1.0;
	if(distanceZC > 0)
		scalatorViewPoint = perfectViewPoint/distanceZC;

	cameraOrigin.x *= scalatorViewPoint;
	cameraOrigin.y *= scalatorViewPoint;
	cameraOrigin.z *= scalatorViewPoint;

	getTextureBinders(TrollHouseMaterials, m_textureId, TrollHouseFirsts,TrollHouseCounts,inTexBinder);
	//~ //getFacesNearToCamera(couchNumVerts2,cameraOrigin2,couchTexCoords2,COLORS2,couchVerts2,
	//~ //outTexCoords2,outColors2,outVertexes2,&finalVertexSize2);
	getAllSortedFacesMT(TrollHouseVertexes,cameraOrigin,TrollHouseTexels,TrollHousePositions,TrollHouseNormals,
	outTexCoords2,outVertexes2,outNormals2,&finalVertexSize2,inTexBinder,outTexBinder);

	// scaling the vertexes withon any help
	scaling(scale, outVertexes2, outVertexes2,finalVertexSize2);

	glNormalPointer(GL_FLOAT,0,outNormals2);
	glVertexPointer(3,GL_FLOAT, 0, outVertexes2);
    glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords2);


	//~ //glColor4f(0.9f,0.9f,0.9f,1.0f);
	glEnable(GL_TEXTURE_2D);
	glColor4f(shadowTuner,shadowTuner,shadowTuner,1.0f);
	//we divide vertex size over 3 because each face have 3 vertexes
	for(unsigned iFace = 0; iFace < finalVertexSize2 / 3; ++iFace){
		glBindTexture(GL_TEXTURE_2D, outTexBinder[iFace]);
		glDrawArrays(GL_TRIANGLES, 3*iFace, 3); //each face have 3 vertexes
	}
	glDisable(GL_TEXTURE_2D);

	endArrays();
	glDisableClientState(GL_NORMAL_ARRAY);

    //~ //glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    //~ //endTrollHouseLight();
    //~ //glDisable(GL_LINE_SMOOTH);
}

