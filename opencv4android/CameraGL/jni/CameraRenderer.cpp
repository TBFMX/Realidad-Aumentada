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

cv::VideoCapture capture;
cv::Mat inframe;
cv::Mat outframe;

int bufferIndex = 0;
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

bool gbIsWindowUpdated = false;
bool gbIsProcessing = false;

extern "C" {
	


void createTexture();
void destroyTexture();
void *frameRetriever(void*);


ARPipeline gbPipeline;
ARDrawingContext gbDrawingCtx;
/**
 * Performs full detection routine on camera frame and draws the scene using drawing context.
 * In addition, this function draw overlay with debug information on top of the AR window.
 * Returns true if processing loop should be stopped; otherwise - false.
 */
bool processFrame(const cv::Mat& cameraFrame, ARPipeline& pipeline, ARDrawingContext& drawingCtx)
{
	//~ cv::Mat img;
    // Clone image used for background (we will draw overlay on it)
    //~ pthread_mutex_lock(&FGmutex);
    //~ img = cameraFrame.clone();
    //~ pthread_mutex_unlock(&FGmutex);
    //~ LOG_INFO("Cloning camera frame ....img w, h = %d, %d", img.cols, img.rows);
	//~ drawingCtx.setWidth(cameraFrame.cols);
	//~ drawingCtx.setHeight(cameraFrame.rows);
	
    // Draw information:
    //~ if (pipeline.m_patternDetector.enableHomographyRefinement)
        //~ cv::putText(img, "Pose refinement: On   ('h' to switch off)", cv::Point(10,15), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));
    //~ else
        //~ cv::putText(img, "Pose refinement: Off  ('h' to switch on)",  cv::Point(10,15), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));
//~ 
    //~ cv::putText(img, "RANSAC threshold: " + ToString(pipeline.m_patternDetector.homographyReprojectionThreshold) + "( Use'-'/'+' to adjust)", cv::Point(10, 30), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));

    // Set a new camera frame:
     //~ pthread_mutex_lock(&FGmutex);
    LOG_INFO("current buffer index %d",bufferIndex);
    drawingCtx.updateBackground(cameraFrame);
    //~ drawingCtx.updateBackground(img);
     //~ pthread_mutex_unlock(&FGmutex);

    // Find a pattern and update it's detection status:
    drawingCtx.isPatternPresent = pipeline.processFrame(cameraFrame);
    //~ LOG_INFO("frame processed");

    // Update a pattern pose:
    drawingCtx.patternPose = pipeline.getPatternLocation();

    // Request redraw of the window:
    drawingCtx.updateWindow(); // callback to window draw

    // Read the keyboard input:
    //~ int keyCode = cv::waitKey(5); 

    return drawingCtx.isWindowUpdated();
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
    ARDrawingContext drawingCtx(frameSize, calibration);

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

    //~ m_isTextureInitialized = false;
    //~ m_isFurnishTextureInitialized = false;
    
    // defining ARDrawing context (inside process video)
    gbIsWindowUpdated = false;
	cv::Size frameSize(frameWidth, frameHeight);
    ARPipeline  pipeline(patternImage, calibration);
    gbPipeline = pipeline;
    ARDrawingContext drawingCtx(frameSize, calibration);
    gbDrawingCtx = drawingCtx;
    gbDrawingCtx.updateFurnishImage();

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	memset(&param, 0, sizeof(param));
	param.sched_priority = 100;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&frameGrabber, &attr, frameRetriever, NULL);
	pthread_attr_destroy(&attr);
	
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
	glViewport(0, 0, width, height);
	if(orien==1) {
		screenWidth = width;
		screenHeight = height;
		orientation = 1;
	} else {
		screenWidth = height;
		screenHeight = width;
		orientation = 2;
	}


	LOG("screenWidth = %d",width);
	LOG("screenHeight = %d",height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = width / height;
	//~ float bt = (float) tan(45 / 2);
	//~ float lr = bt * aspect;
	//~ glFrustumf(-lr * 0.1f, lr * 0.1f, -bt * 0.1f, bt * 0.1f, 0.1f, 100.0f);
	
	float fovY = 628.7519411113429;
    const float pi = 3.1415926535897932384626433832795;
    float fW, fH, zNear = 0.01, zFar = 100;
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
	if(gbIsProcessing == false){
		//~ gbDrawingCtx.setWidth(width);
		//~ gbDrawingCtx.setHeight(height);
		gbDrawingCtx.createTexture();
	}
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_releaseCamera(JNIEnv*, jobject)
{
	LOG("Camera Released");
	capture.release();
	if(gbIsProcessing == false){
		gbDrawingCtx.destroyTexture();
	}
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_renderBackground(JNIEnv*, jobject) {
	//~ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //~ glShadeModel(GL_FLAT);
   //~ glEnable(GL_DEPTH_TEST);
	//~ getBackgroundTextures();
	
	//~ pthread_mutex_lock(&FGmutex);
	//~ drawBackground(textureId[0],capFrameWidth,capFrameHeight);
	//~ drawBackground(textureId[1],capFrameWidth,capFrameHeight);
	//~ glutSwapBuffers();
	//~ drawFurnish(textureId[1],screenWidth,screenHeight);
	//~ glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//~ drawFurnish(textureId[1],capFrameWidth,capFrameHeight);
	//~ for(int i =0; i < 10000; ++i);
	//~ pthread_mutex_unlock(&FGmutex);
		
	if(bufferIndex > 0 && gbIsWindowUpdated == true && gbIsProcessing == false){
		// pthread_mutex_lock(&FGmutex);
		//~ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gbDrawingCtx.draw();
		// pthread_mutex_unlock(&FGmutex);
		gbIsWindowUpdated = false;
	}
	//~ glFlush();
}

void *frameRetriever(void*) {
	while (capture.isOpened()) {
		capture.read(inframe);
		if (!inframe.empty()) {
			//~ LOG("Starting main thread.....");
			capFrameWidth=inframe.cols;
			capFrameHeight=inframe.rows;
			pthread_mutex_lock(&FGmutex);
			inframe.copyTo(captureBuffer[(bufferIndex++) % 30]);
			pthread_mutex_unlock(&FGmutex);
			//~ if(gbIsProcessing==false){
				//~ (*gbDrawingCtx).setWidth(capFrameWidth);
				//~ (*gbDrawingCtx).setHeight(capFrameHeight);
			//~ }
			if(bufferIndex == 30)
				bufferIndex = 0;

		}
			// loosing the zero frame
		if(bufferIndex > 0 && gbIsWindowUpdated == false && gbIsProcessing == false){
			cv::Mat rgbFrame;
			pthread_mutex_lock(&FGmutex);
			cvtColor(captureBuffer[(bufferIndex - 1) % 30], rgbFrame, CV_BGR2RGB);

			pthread_mutex_unlock(&FGmutex);
			if(!rgbFrame.empty()){
				//~ pthread_mutex_lock(&FGmutex);
				//~ gbDrawingCtx.updateBackground(rgbFrame);
				gbIsProcessing = true;
				gbIsWindowUpdated = processFrame(rgbFrame, gbPipeline, gbDrawingCtx);
				gbIsProcessing = false;
				//~ pthread_mutex_unlock(&FGmutex);
				
			}
		}		
		
		//~ else
			//~ captureBuffer[29].copyTo(currentFrame);		
	}
	LOG("Camera Closed");
	pthread_exit (NULL);
}

}
