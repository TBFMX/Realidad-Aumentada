#include <jni.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <android/log.h>
#include <pthread.h>
#include <time.h>
#include <vector>
#include <math.h>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "ARDrawingContext.hpp"
#include "ARPipeline.hpp"
#include "DebugHelpers.hpp"
#include "logger.h"
#include "geometryStructs.hpp"
//~ #include "CameraRenderer.h"

// Utility for logging:
#define LOG_TAG    "CAMERA_RENDERER"
#define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

unsigned int textureId[2];
cv::VideoCapture capture;
cv::Mat rgbFrame;
cv::Mat inframe;
cv::Mat outframe;
int bufferIndex;
int rgbIndex;
static int capFrameWidth;
static int capFrameHeight;
int frameWidth;
int frameHeight;
int screenWidth;
int screenHeight;
int orientation;

pthread_mutex_t FGmutex;
pthread_t frameGrabber;
pthread_attr_t attr;

struct sched_param param;
float maxAllowedWidth = 1920.0;
float maxAllowedHeight = 1536.0;

//~ cv::Mat m_backgroundImage;
cv::Mat captureBuffer[30];
cv::Mat processedBuffer[30];
cv::Mat patternImage;
CameraCalibration calibration;
bool m_isTextureInitialized = false;
bool m_isFurnishTextureInitialized = false;

extern "C" {
	
void getFurnishTexture(unsigned int);
void getBackgroundTextures();

void createTexture();
void destroyTexture();
void *frameRetriever(void*);

/**
 * Performs full detection routine on camera frame and draws the scene using drawing context.
 * In addition, this function draw overlay with debug information on top of the AR window.
 * Returns true if processing loop should be stopped; otherwise - false.
 */
bool processFrame(const cv::Mat& cameraFrame, ARPipeline& pipeline, ARDrawingContext& drawingCtx)
{
    // Clone image used for background (we will draw overlay on it)
    cv::Mat img = cameraFrame.clone();

    // Draw information:
    //~ if (pipeline.m_patternDetector.enableHomographyRefinement)
        //~ cv::putText(img, "Pose refinement: On   ('h' to switch off)", cv::Point(10,15), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));
    //~ else
        //~ cv::putText(img, "Pose refinement: Off  ('h' to switch on)",  cv::Point(10,15), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));
//~ 
    //~ cv::putText(img, "RANSAC threshold: " + ToString(pipeline.m_patternDetector.homographyReprojectionThreshold) + "( Use'-'/'+' to adjust)", cv::Point(10, 30), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));

    // Set a new camera frame:
    drawingCtx.updateBackground(img);

    // Find a pattern and update it's detection status:
    drawingCtx.isPatternPresent = pipeline.processFrame(cameraFrame);

    // Update a pattern pose:
    drawingCtx.patternPose = pipeline.getPatternLocation();

    // Request redraw of the window:
    drawingCtx.updateWindow();

    // Read the keyboard input:
    //~ int keyCode = cv::waitKey(5); 

    bool shouldQuit = false;
    //~ if (keyCode == '+' || keyCode == '=')
    //~ {
        //~ pipeline.m_patternDetector.homographyReprojectionThreshold += 0.2f;
        //~ pipeline.m_patternDetector.homographyReprojectionThreshold = std::min(10.0f, pipeline.m_patternDetector.homographyReprojectionThreshold);
    //~ }
    //~ else if (keyCode == '-')
    //~ {
        //~ pipeline.m_patternDetector.homographyReprojectionThreshold -= 0.2f;
        //~ pipeline.m_patternDetector.homographyReprojectionThreshold = std::max(0.0f, pipeline.m_patternDetector.homographyReprojectionThreshold);
    //~ }
    //~ else if (keyCode == 'h')
    //~ {
        //~ pipeline.m_patternDetector.enableHomographyRefinement = !pipeline.m_patternDetector.enableHomographyRefinement;
    //~ }
    //~ else if (keyCode == 27 || keyCode == 'q')
    //~ {
        //~ shouldQuit = true;
    //~ }

    return shouldQuit;
}


/**
 * Processes a recorded video or live view from web-camera and allows you to adjust homography refinement and 
 * reprojection threshold in runtime.
 */
void processVideo(const cv::Mat& patternImage, CameraCalibration& calibration)
{
    // Grab first frame to get the frame dimensions
    cv::Mat currentFrame;
	if(bufferIndex > 0)
		captureBuffer[bufferIndex - 1].copyTo(currentFrame);
	else
		captureBuffer[29].copyTo(currentFrame);

    // Check the capture succeeded:
    if (currentFrame.empty())
    {
        std::cout << "Cannot open video capture device" << std::endl;
        return;
    }

    cv::Size frameSize(currentFrame.cols, currentFrame.rows);

    ARPipeline pipeline(patternImage, calibration);
    ARDrawingContext drawingCtx("Markerless AR", frameSize, calibration);

    bool shouldQuit = false;
    do
    {
		if(bufferIndex > 0)
			captureBuffer[bufferIndex - 1].copyTo(currentFrame);
		else
			captureBuffer[29].copyTo(currentFrame);
        if (currentFrame.empty())
        {
            shouldQuit = true;
            continue;
        }

        shouldQuit = processFrame(currentFrame, pipeline, drawingCtx);
    } while (!shouldQuit);
}


/**
 * Processes single image. The processing goes in a loop.
 * It allows you to control the detection process by adjusting homography refinement switch and 
 * reprojection threshold in runtime.
 */
void processSingleImage(const cv::Mat& patternImage, CameraCalibration& calibration, const cv::Mat& image)
{
    cv::Size frameSize(image.cols, image.rows);
    ARPipeline pipeline(patternImage, calibration);
    ARDrawingContext drawingCtx("Markerless AR", frameSize, calibration);

    bool shouldQuit = false;
    do
    {
        shouldQuit = processFrame(image, pipeline, drawingCtx);
    } while (!shouldQuit);
}

void applyCanny(cv::Mat *image, cv::Mat *result){
	//~ cv::Mat contours;
	//~ cv::Canny(*image, contours, 125,350);
	//~ cv::threshold(contours, *result,128, 255, cv::THRESH_BINARY_INV);
	std::vector<cv::Mat> rgb;
	cv::split(*image, rgb);

	cv::Canny( rgb[0], rgb[0], 125, 350);
	cv::Canny( rgb[1], rgb[1], 125, 350);
	cv::Canny( rgb[2], rgb[2], 125, 350);	
	cv::Mat mergedImage;
	bitwise_or(rgb[0], rgb[1], mergedImage);
	bitwise_or(mergedImage, rgb[2], mergedImage);
	mergedImage.copyTo(*result);
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_initCamera(JNIEnv*, jobject,jint width,jint height)
{
	// franquy parameters
	float fx = 695.4521167717107;
	float fy = 694.5519610122569;
	float cx = 337.2059936807979;
	float cy = 231.1645822893514;
	
	// tablet parameters
	fx=628.6341119951087;
	fy=628.7519411113429;
	cx=325.3443919995285;
	cy=236.0028199018263;
	
    // Change this calibration to yours:
    //~ CameraCalibration calibration(526.58037684199849f, 524.65577209994706f, 318.41744018680112f, 202.96659047014398f);
    calibration.set4Params(fx,fy,cx,cy);
    patternImage=cv::imread("sdcard/Models/PyramidPattern.jpg");
    
	LOG("Camera Created");
	capture.open(CV_CAP_ANDROID + 0);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, width);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	frameWidth =width;
	frameHeight = height;
	LOG("frameWidth = %d",frameWidth);
	LOG("frameHeight = %d",frameHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glShadeModel(GL_SMOOTH);
	glClearDepthf(1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	memset(&param, 0, sizeof(param));
	param.sched_priority = 100;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&frameGrabber, &attr, frameRetriever, NULL);
	pthread_attr_destroy(&attr);
	
    m_isTextureInitialized = false;
    m_isFurnishTextureInitialized = false;
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_surfaceChanged(JNIEnv*, jobject,jint width,jint height,jint orien)
{
    //~ glDisable(GL_DITHER);
    //~ glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //~ glClearColor(0, 0, 0, 1);
    //~ glEnable(GL_CULL_FACE);
    //~ glShadeModel(GL_SMOOTH);
    //~ glEnable(GL_DEPTH_TEST);
    //~ 
	//~ LOG("Surface Changed");
	//~ glViewport(0, 0, width,height);
	//~ if(orien==1) {
		//~ screenWidth = width;
		//~ screenHeight = height;
		//~ orientation = 1;
	//~ } else {
		//~ screenWidth = height;
		//~ screenHeight = width;
		//~ orientation = 2;
	//~ }
//~ 
	//~ LOG("screenWidth = %d",screenWidth);
	//~ LOG("screenHeight = %d",screenHeight);
//~ 
    //~ GLfloat ratio = (GLfloat) width / height;
    //~ glMatrixMode(GL_PROJECTION);
    //~ glLoadIdentity();
    //~ glFrustumf(-ratio, ratio, -1, 1, 1, 10000);	
	LOG("Surface Changed");
	glViewport(0, 0, width,height);
	if(orien==1) {
		screenWidth = width;
		screenHeight = height;
		orientation = 1;
	} else {
		screenWidth = height;
		screenHeight = width;
		orientation = 2;
	}


	LOG("screenWidth = %d",screenWidth);
	LOG("screenHeight = %d",screenHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = screenWidth / screenHeight;
	//~ float bt = (float) tan(45 / 2);
	//~ float lr = bt * aspect;
	//~ glFrustumf(-lr * 0.1f, lr * 0.1f, -bt * 0.1f, bt * 0.1f, 0.1f, 100.0f);
	
	float fovY =628.7519411113429;
    const float pi = 3.1415926535897932384626433832795;
    float fW, fH, zNear = 0.1, zFar = 100;
    fH = (float) tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect *zNear;
    glFrustumf( -fW, fW, -fH, fH, zNear, zFar );
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepthf(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	createTexture();
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_releaseCamera(JNIEnv*, jobject)
{
	LOG("Camera Released");
	capture.release();
	destroyTexture();

}

void createTexture() {
	// Initialize texture for background image
	if (!m_isTextureInitialized)
	{
		  // we generate both textures
		LOG("Texture Created");
		glGenTextures(2,textureId);
		glBindTexture(GL_TEXTURE_2D, textureId[0]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_isTextureInitialized = true;
	}
	getFurnishTexture(textureId[1]);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void destroyTexture() {
	LOG("Texture destroyed");
	glDeleteTextures(2, textureId);
	m_isTextureInitialized = false;
	m_isFurnishTextureInitialized = false;
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_renderBackground(JNIEnv*, jobject) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //~ glShadeModel(GL_FLAT);
   //~ glEnable(GL_DEPTH_TEST);
	getBackgroundTextures();
	
	//~ pthread_mutex_lock(&FGmutex);
	drawBackground(textureId[0],capFrameWidth,capFrameHeight);
	//~ drawBackground(textureId[1],capFrameWidth,capFrameHeight);
	//~ glutSwapBuffers();
	//~ drawFurnish(textureId[1],screenWidth,screenHeight);
	//~ glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawFurnish(textureId[1],capFrameWidth,capFrameHeight);
	//~ for(int i =0; i < 10000; ++i);
	//~ pthread_mutex_unlock(&FGmutex);
	//~ glFlush();
}

void getBackgroundTextures() {
	int w = capFrameWidth;
	int h = capFrameHeight;

	if(bufferIndex > 0){
		pthread_mutex_lock(&FGmutex);
		//~ cv::Mat auxCap;
		cvtColor(captureBuffer[(bufferIndex - 1) % 30], rgbFrame, CV_BGR2RGB);
		//~ applyCanny(&auxCap,&rgbFrame);
		pthread_mutex_unlock(&FGmutex);
		w=rgbFrame.cols;
		h=rgbFrame.rows;
		//LOG_INFO("w,h, channels= %d,%d, %d, %d ",w,h,rgbFrame.channels(), textureId[0]);
		
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, textureId[0]);

		if (textureId[0] != 0){
			// Upload new texture data:
		if (rgbFrame.channels() == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbFrame.data);
		else if(rgbFrame.channels() == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbFrame.data);
		else if (rgbFrame.channels()==1)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, rgbFrame.data);
		}
		
		if (bufferIndex==30)
			bufferIndex = 0;
	}
}

void getFurnishTexture(unsigned int texName){
	if (!m_isFurnishTextureInitialized)
	{
		//texture image
		cv::Mat m_furnishImage = cv::imread("sdcard/Models/couch.jpg");
		//~ /*if (m_furnishImage.rows > 0){
			//~ LOG_INFO("model open correctly...OK : %d, %d, %d",m_furnishImage.rows,m_furnishImage.cols,m_furnishImage.channels() );
		//~ }else{
			//~ LOG_INFO("texture file is not loaded");
			//~ std::ifstream myfile("sdcard/Models/someToast.txt");
			//~ std::string someString;
			//~ if(myfile.is_open()){
				//~ getline(myfile,someString);
				//~ char *someChar = &someString[0];
				//~ LOG_INFO("correct access %s", someChar);
				//~ myfile.close();
			//~ }
		//~ }*/
		cvtColor(m_furnishImage,m_furnishImage,CV_BGR2RGBA);
		
		//~ //glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int w = m_furnishImage.cols;
		int h = m_furnishImage.rows;

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, texName);

		// Upload new texture data:
		if (m_furnishImage.channels() == 3)
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_furnishImage.data);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_furnishImage.data);
		else if(m_furnishImage.channels() == 4)
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_furnishImage.data);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_furnishImage.data);
		else if (m_furnishImage.channels()==1)
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_furnishImage.data);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_furnishImage.data);		

		m_isFurnishTextureInitialized = true;
	}	
}

void *frameRetriever(void*) {
	while (capture.isOpened()) {
		capture.read(inframe);
		if (!inframe.empty()) {
			capFrameWidth=inframe.cols;
			capFrameHeight=inframe.rows;
			pthread_mutex_lock(&FGmutex);
			inframe.copyTo(captureBuffer[(bufferIndex++) % 30]);
			pthread_mutex_unlock(&FGmutex);
		}
		//~ processVideo(patternImage, calibration);
	}
	LOG("Camera Closed");
	pthread_exit (NULL);
}

}
