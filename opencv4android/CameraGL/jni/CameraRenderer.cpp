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
#include "MarkerDetector.hpp"
#include "DebugHelpers.hpp"
#include "logger.h"
#include "geometryStructs.hpp"
//~ #include "CameraRenderer.h"

// Utility for logging:
#define LOG_TAG    "CAMERA_RENDERER"
#define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define MAX_PERSISTANCE 60

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
int gbModelId;
bool gbIsCaptureOpened =true;
bool gbIsCameraTextureInitialized = false;
unsigned int gbCameraTexture=0;
unsigned int gbPersistance = 0;

pthread_mutex_t FGmutex;
pthread_t frameRetrieverThread;
//~ pthread_attr_t attr;

struct sched_param Rparam;

float maxAllowedWidth = 1920.0;
float maxAllowedHeight = 1536.0;


//~ cv::Mat m_backgroundImage;
cv::Mat captureBuffer[30];
//~ cv::Mat processedBuffer[30];
cv::Mat patternImage;
CameraCalibration calibration;

bool gbIsWindowUpdated = false;
bool gbIsProcessing = false;

extern "C" {

void createTexture();
void destroyTexture();
void *frameRetriever(void*);

MarkerDetector gbMarkerDetector;
ARDrawingContext gbDrawingCtx;

void createCameraTexture() {
	unsigned int numTextures = 1;
	// Initialize texture for background image
	if (!gbIsCameraTextureInitialized)
	{
		  // we generate both textures
		LOG("Camera texture Created");
		glGenTextures(numTextures, &gbCameraTexture);
		glBindTexture(GL_TEXTURE_2D, gbCameraTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gbIsCameraTextureInitialized = true;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void destroyCameraTexture() {
	LOG("Camera texture destroyed");
	glDeleteTextures(1, &gbCameraTexture);
	gbIsCameraTextureInitialized = false;
}

void drawCurrentCameraFrame(int texName, int bufferIndex, cv::Mat captureBuffer[]) {

	if(bufferIndex > 0){
		//~ pthread_mutex_lock(&FGmutex);
		cv::Mat rgbCameraFrame;
		cvtColor(captureBuffer[(bufferIndex - 1) % 30], rgbCameraFrame, CV_BGR2RGB);
		//~ pthread_mutex_unlock(&FGmutex);
		int w=rgbCameraFrame.cols;
		int h=rgbCameraFrame.rows;
		//LOG_INFO("w,h, channels= %d,%d, %d, %d ",w,h,rgbFrame.channels(), textureId[0]);
		
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, texName);

		if (texName != 0){
			// Upload new texture data:
		if (rgbCameraFrame.channels() == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbCameraFrame.data);
		else if(rgbCameraFrame.channels() == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbCameraFrame.data);
		else if (rgbCameraFrame.channels()==1)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, rgbCameraFrame.data);
		}
		
		//~ glColor4f(1.0f,1.0f,1.0f,1.0f);
		
		// drawing the current frame
		const GLfloat bgTextureVertices[] = { 0, 0, w, 0, 0, h, w, h };
		const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
		const GLfloat proj[]              = { 0, -2.f/w, 0, 0, -2.f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(proj);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texName);

		// Update attribute values.
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		
		//~ GLfloat mdl[16];
		//~ glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
		//~ getCameraOrigin(mdl,&cameraOrigin);

		glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
		glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

		glColor4f(1,1,1,1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);	
	
	}
}

/**
 * Performs full detection routine on camera frame and draws the scene using drawing context.
 * In addition, this function draw overlay with debug information on top of the AR window.
 * Returns true if processing loop should be stopped; otherwise - false.
 */
bool processFrame(cv::Mat& cameraFrame, MarkerDetector& markerDetector, ARDrawingContext& drawingCtx)
{
    //~ LOG_INFO("current buffer index %d",bufferIndex);
    drawingCtx.updateBackground(cameraFrame);

	//~ pipeline.m_patternDetector.homographyReprojectionThreshold += 0.2;
	//~ pipeline.m_patternDetector.homographyReprojectionThreshold = std::min(10.0f, pipeline.m_patternDetector.homographyReprojectionThreshold);
	
    // Find a pattern and update it's detection status:
    drawingCtx.isPatternPresent = markerDetector.processFrame(cameraFrame);
    //~ LOG_INFO("pattern testing");

	std::vector<Transformation> m_transformations;
	m_transformations = markerDetector.getTransformations();
    // Update a pattern pose:
    //~ for( int i = 0; i < m_transformations.size(); ++i)
		//~ drawingCtx.patternPose = m_transformations[i];
    if(m_transformations.size() > 0)
		drawingCtx.patternPose = m_transformations[0];

     LOG_INFO("pose testing %d transformations", m_transformations.size());

    // Request redraw of the window:
    drawingCtx.updateWindow(); // callback to window draw
	LOG_INFO("can draw");
    //~ return drawingCtx.isWindowUpdated(); // always true
    return true;
}

/**
 * Processes single image. The processing goes in a loop.
 * It allows you to control the detection process by adjusting homography refinement switch and 
 * reprojection threshold in runtime.
 */
void processSingleImage(const cv::Mat& patternImage, CameraCalibration& calibration, cv::Mat& image)
{
    cv::Size frameSize(image.cols, image.rows);
    MarkerDetector markerDetector(calibration);
    ARDrawingContext drawingCtx(frameSize, calibration);

    bool shouldQuit = false;
    do
    {
        shouldQuit = processFrame(image, markerDetector, drawingCtx);
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
	LOG_INFO("Entering");
	// franquy parameters 640x480
	float fx = 695.4521167717107;
	float fy = 694.5519610122569;
	float cx = 337.2059936807979;
	float cy = 231.1645822893514;
	
	// tablet parameters 640x480
	fx=628.6341119951087;
	fy=628.7519411113429;
	cx=325.3443919995285;
	cy=236.0028199018263;
	// tablet parameters 1024x768
	fx=695.398588072418;
	fy=672.4933734804648;
	cx=448.2380923498616;
	cy=261.186397004368;
	//~ // tablet parameters 2048x1536
	//~ fx=1194.514196322121;
	//~ fy=1103.525587362546;
	//~ cx=950.3227406860865;
	//~ cy=745.1507425384597;
	
	
    // Change this calibration to yours:
    //~ CameraCalibration calibration(526.58037684199849f, 524.65577209994706f, 318.41744018680112f, 202.96659047014398f);
    calibration.set4Params(fx,fy,cx,cy);
    //~ patternImage=cv::imread("sdcard/Models/PyramidPattern.jpg");
    
	LOG("Camera Created");
	//~ capture.open(CV_CAP_ANDROID + 0);
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
    MarkerDetector  markerDetector(calibration);
    gbMarkerDetector = markerDetector;
    ARDrawingContext drawingCtx(frameSize, calibration);
    gbDrawingCtx = drawingCtx;
    gbDrawingCtx.updateFurnishImage();
    gbDrawingCtx.updateTigerImage();

	pthread_attr_t Rattr;
	pthread_attr_init(&Rattr);
	pthread_attr_setdetachstate(&Rattr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&Rattr, SCHED_FIFO);
	memset(&Rparam, 0, sizeof(Rparam));
	Rparam.sched_priority = 100;
	pthread_attr_setschedparam(&Rattr, &Rparam);
	pthread_create(&frameRetrieverThread, &Rattr, frameRetriever, NULL);
	pthread_attr_destroy(&Rattr);
	
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_surfaceChanged(JNIEnv*, jobject,jint width,jint height,jint orien)
{
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
	
	//~ float fovY = 628.7519411113429;
	float fovY = 672.4933734804648;
	//~ float fovX = 695.398588072418;
	//~ float fovY = 1103.525587362546;
    const float pi = 3.1415926535897932384626433832795;
    float fW, fH, zNear = 0.01, zFar = 100;
    fH = (float) tan( fovY / 360 * pi ) * zNear;
    
    //~ fW = (float) tan( fovX / 360 * pi ) * zNear;
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
		gbDrawingCtx.createTexture();
	}
	createCameraTexture();
	
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_releaseCamera(JNIEnv*, jobject)
{
	LOG("Camera Released");
	capture.release();
	if(gbIsProcessing == false){
		gbDrawingCtx.destroyTexture();
	}
	destroyCameraTexture();
	gbIsCameraTextureInitialized = false;
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_renderBackground(JNIEnv*, jobject) {
	//~ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//~ glClear(GL_DEPTH_BUFFER_BIT); // this is the trickiest command
	//~ glClearColor(1,1,1,1);
//~ 
	if(bufferIndex > 0 && gbIsWindowUpdated == true && gbIsProcessing == false){
		pthread_mutex_lock(&FGmutex);
		// we validate if the pattern found was the best
		gbDrawingCtx.validatePatternPresent(); 
		pthread_mutex_unlock(&FGmutex);
		gbIsWindowUpdated = false;
	}
	
	pthread_mutex_lock(&FGmutex);
	if(gbDrawingCtx.isThereAPattern()){
		//~ // special code for low dispositives-----------
		if (gbIsWindowUpdated || bufferIndex > 0){
			cv::Mat rgbFrame;
			//pthread_mutex_lock(&FGmutex);
			cvtColor(captureBuffer[bufferIndex % 30], rgbFrame, CV_BGR2RGB);
			gbDrawingCtx.updateBackground(rgbFrame);
			//pthread_mutex_unlock(&FGmutex);
		}
		
		//~ //----------------------------------------------
		gbDrawingCtx.draw();
		gbPersistance = 1;
	}else{
		if(gbPersistance > 0){
			// special code for low dispositives---------
			if (gbIsWindowUpdated || bufferIndex > 0){
				cv::Mat rgbFrame;
				//pthread_mutex_lock(&FGmutex);
				cvtColor(captureBuffer[bufferIndex % 30], rgbFrame, CV_BGR2RGB);
				gbDrawingCtx.updateBackground(rgbFrame);
				//pthread_mutex_unlock(&FGmutex);
			}
			//-------------------------------------------

			gbDrawingCtx.draw();
			++gbPersistance;
			LOG_INFO("persistance %d", gbPersistance);
			if(gbPersistance > MAX_PERSISTANCE)
				gbPersistance = 0;
			
		}else{
			drawCurrentCameraFrame(gbCameraTexture, bufferIndex, captureBuffer);
		}
	}
	
	pthread_mutex_unlock(&FGmutex);
	
	glFlush();
}


JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_changeShownModel(JNIEnv*, jobject){
	pthread_mutex_lock(&FGmutex);
	
	switch(gbDrawingCtx.objectToDraw()){
		case 0:
			gbDrawingCtx.setObjectToDraw(1);
			break;
		case 1:
			gbDrawingCtx.setObjectToDraw(2);
			break;
		case 2:
			gbDrawingCtx.setObjectToDraw(3);
			break;
		case 3:
			gbDrawingCtx.setObjectToDraw(4);
			break;
		case 4:
			gbDrawingCtx.setObjectToDraw(5);
			break;
		case 5:
			gbDrawingCtx.setObjectToDraw(0);
			break;
		default:
			gbDrawingCtx.setObjectToDraw(0);
			break;
	}
		
	pthread_mutex_unlock(&FGmutex);
}

JNIEXPORT jfloat JNICALL Java_com_tbf_cameragl_Native_getAngle(JNIEnv*, jobject){
	pthread_mutex_lock(&FGmutex);
	
	jfloat angle = gbDrawingCtx.getAngle();
		
	pthread_mutex_unlock(&FGmutex);
	return angle;
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_setAngle(JNIEnv*, jobject, jfloat angle){
	pthread_mutex_lock(&FGmutex);
	gbDrawingCtx.setAngle(angle);
	pthread_mutex_unlock(&FGmutex);
}

JNIEXPORT jfloat JNICALL Java_com_tbf_cameragl_Native_getScale(JNIEnv*, jobject){
	pthread_mutex_lock(&FGmutex);
	
	jfloat scale = gbDrawingCtx.getScale();
		
	pthread_mutex_unlock(&FGmutex);
	return scale;
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_setScale(JNIEnv*, jobject, jfloat scale){
	pthread_mutex_lock(&FGmutex);
	gbDrawingCtx.setScale(scale);
	pthread_mutex_unlock(&FGmutex);
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_setTranslate(JNIEnv*, jobject, jfloat translateX, jfloat translateY){
	pthread_mutex_lock(&FGmutex);
	gbDrawingCtx.setTranslate(translateX,translateY);
	pthread_mutex_unlock(&FGmutex);
}

void *frameRetriever(void*) {
	LOG_INFO("Enter retrieving");	
	pthread_mutex_lock(&FGmutex);
	gbIsCaptureOpened = true;
	pthread_mutex_unlock(&FGmutex);
	int passFrame = 6;
	int countFrames = 0;
	while (capture.isOpened()) {
		capture.read(inframe);
		if (!inframe.empty()) {
			//~ LOG("Starting main thread.....");
			capFrameWidth=inframe.cols;
			capFrameHeight=inframe.rows;
			pthread_mutex_lock(&FGmutex);
			inframe.copyTo(captureBuffer[(bufferIndex++) % 30]);
			pthread_mutex_unlock(&FGmutex);
			pthread_mutex_lock(&FGmutex);
			if(bufferIndex == 30)
				bufferIndex = 0;
			//~ LOG_INFO("index %d",bufferIndex);	
			pthread_mutex_unlock(&FGmutex);
		}
		
		// bufferIndex > 0 and pair
			// loosing the zero frame
		if(bufferIndex > 0 && countFrames % passFrame == 0 && gbIsWindowUpdated == false && gbIsProcessing == false){
			cv::Mat rgbFrame;
			
			pthread_mutex_lock(&FGmutex);
			cvtColor(captureBuffer[(bufferIndex - 1) % 30], rgbFrame, CV_BGR2RGB);
			pthread_mutex_unlock(&FGmutex);
	
			//~ if(!rgbFrame.empty()){
			gbIsProcessing = true;
			gbIsWindowUpdated = processFrame(rgbFrame, gbMarkerDetector, gbDrawingCtx);
			gbIsProcessing = false;
			//~ }
			countFrames = 0;
		}
		
		++countFrames;
		
		//~ else
			//~ captureBuffer[29].copyTo(currentFrame);
	}
	pthread_mutex_lock(&FGmutex);
	gbIsCaptureOpened = false;
	pthread_mutex_unlock(&FGmutex);
	LOG("Camera Closed");
	pthread_exit (NULL);
}

}
