#include <jni.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <android/log.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <opencv/cv.h>
#include "opencv2/highgui/highgui.hpp"
#include "drawer.h"
// Utility for logging:
//~ #define LOG_TAG    "CAMERA_RENDERER"
#define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

unsigned int textureId[2];
cv::VideoCapture capture;
cv::Mat rgbFrame;
cv::Mat inframe;
cv::Mat outframe;
int bufferIndex;
int rgbIndex;
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
bool m_isTextureInitialized = false;
//~ cv::Mat m_backgroundImage;
cv::Mat buffer[30];

extern "C" {

void drawBackground(unsigned int);
void getBackgroundTextures();
void createTexture();
void destroyTexture();
void *frameRetriever(void*);

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_initCamera(JNIEnv*, jobject,jint width,jint height)
{
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
	float bt = (float) tan(45 / 2);
	float lr = bt * aspect;
	glFrustumf(-lr * 0.1f, lr * 0.1f, -bt * 0.1f, bt * 0.1f, 0.1f, 100.0f);
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
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

void destroyTexture() {
	LOG("Texture destroyed");
	glDeleteTextures(2, textureId);
	m_isTextureInitialized = false;
}

JNIEXPORT void JNICALL Java_com_tbf_cameragl_Native_renderBackground(JNIEnv*, jobject) {
	getBackgroundTextures();
	getFurnishTexture(textureId[1]);
	drawBackground(textureId[0]);
	drawFrame(textureId[1]);
	//~ glFlush();
}

void getBackgroundTextures() {
	int w = buffer[0].cols;
	int h = buffer[0].rows;

	if(bufferIndex > 0){
		pthread_mutex_lock(&FGmutex);
		cvtColor(buffer[(bufferIndex - 1) % 30], rgbFrame, CV_BGR2RGB);
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

void drawBackground(unsigned int texName){
	
	//~ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int w = buffer[0].cols;
	int h = buffer[0].rows;
  const GLfloat bgTextureVertices[] = { 0, 0, w, 0, 0, h, w, h };
  const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
  const GLfloat proj[]              = { 0, -2.0f/w, 0, 0, -2.0f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

	LOG_INFO("w,h, channels= %d,%d ",w,h);
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

	glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

	glColor4f(1,1,1,1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void *frameRetriever(void*) {
	while (capture.isOpened()) {
		capture.read(inframe);
		if (!inframe.empty()) {
			pthread_mutex_lock(&FGmutex);
			inframe.copyTo(buffer[(bufferIndex++) % 30]);
			pthread_mutex_unlock(&FGmutex);
		}
	}
	LOG("Camera Closed");
	pthread_exit (NULL);
}

}
