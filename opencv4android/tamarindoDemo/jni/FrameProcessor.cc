#include <jni.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <cassert>
#include <stdexcept>
#include "FrameProcessor.h"
#include "orbAndroid.h"
#include <opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/features2d/features2d.hpp>

#include <vector>
#include <android/log.h>

#define LOG_TAG "FrameProcessor/FrameProcessor"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

using namespace std;
using namespace cv;

inline void vector_Rect_to_Mat(vector<Rect>& v_rect, Mat& mat)
{
    mat = Mat(v_rect, true);
}

OrbAndroid newPerspective;

JNIEXPORT jlong JNICALL Java_com_tbf_jnitester_FrameProcessor_nativeCreateObject
(JNIEnv * jenv, jclass, jlong addrRgbaMarker )
{
    LOGD("Java_com_tbf_facedetection_FaceDetection_nativeCreateObject enter");

    jlong result = 0;

    try
    {
        result = (jlong) new FrameProcessor();
        Mat& marker = *(Mat*)addrRgbaMarker;
        newPerspective.updateMarker(&marker);
    }
    catch(cv::Exception& e)
    {
        LOGD("nativeCreateObject caught cv::Exception: %s", e.what());
        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if(!je)
            je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, e.what());
    }
    catch (...)
    {
        LOGD("nativeCreateObject caught unknown exception");
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code of FrameProcessor.nativeCreateObject()");
        return 0;
    }

    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeCreateObject exit");
    return result;
}

JNIEXPORT void JNICALL Java_com_tbf_jnitester_FrameProcessor_nativeDestroyObject
(JNIEnv * jenv, jclass, jlong thiz)
{
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeDestroyObject enter");
    try
    {
        if(thiz != 0)
        {
            ((FrameProcessor*)thiz)->stop();
            delete (FrameProcessor*)thiz;
        }
    }
    catch(cv::Exception& e)
    {
        LOGD("nativeestroyObject caught cv::Exception: %s", e.what());
        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if(!je)
            je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, e.what());
    }
    catch (...)
    {
        LOGD("nativeDestroyObject caught unknown exception");
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code of FrameProcessor.nativeDestroyObject()");
    }
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeDestroyObject exit");
}

JNIEXPORT void JNICALL Java_com_tbf_jnitester_FrameProcessor_nativeStart
(JNIEnv * jenv, jclass, jlong thiz)
{
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeStart enter");
    try
    {
        ((FrameProcessor*)thiz)->run();
    }
    catch(cv::Exception& e)
    {
        LOGD("nativeStart caught cv::Exception: %s", e.what());
        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if(!je)
            je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, e.what());
    }
    catch (...)
    {
        LOGD("nativeStart caught unknown exception");
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code of FrameProcessor.nativeStart()");
    }
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeStart exit");
}

JNIEXPORT void JNICALL Java_com_tbf_jnitester_FrameProcessor_nativeStop
(JNIEnv * jenv, jclass, jlong thiz)
{
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeStop enter");
    try
    {
        ((FrameProcessor*)thiz)->stop();
    }
    catch(cv::Exception& e)
    {
        LOGD("nativeStop caught cv::Exception: %s", e.what());
        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if(!je)
            je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, e.what());
    }
    catch (...)
    {
        LOGD("nativeStop caught unknown exception");
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code of FrameProcessor.nativeStop()");
    }
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeStop exit");
}

JNIEXPORT void JNICALL Java_com_tbf_jnitester_FrameProcessor_nativeFindFeatures
(JNIEnv * jenv, jclass, jlong thiz, jlong addrGray, jlong addrRgba)
{
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeDetect enter");
    try
    {
        Mat& mGr  = *(Mat*)addrGray;
        Mat& mRgb = *(Mat*)addrRgba;
        vector<KeyPoint> v;

        FastFeatureDetector detector(50);
        detector.detect(mGr, v);
        for( unsigned int i = 0; i < v.size(); i++ )
        {
            const KeyPoint& kp = v[i];
            circle(mRgb, Point(kp.pt.x, kp.pt.y), 10, Scalar(255,0,0,255));
        }
    }
    catch(cv::Exception& e)
    {
        LOGD("nativeCreateObject caught cv::Exception: %s", e.what());
        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if(!je)
            je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, e.what());
    }
    catch (...)
    {
        LOGD("nativeDetect caught unknown exception");
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code FrameProcessor.nativeProcess()");
    }
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeProcess exit");
}

JNIEXPORT void JNICALL Java_com_tbf_jnitester_FrameProcessor_nativeMarker
(JNIEnv * jenv, jclass, jlong thiz, jlong addrRgbaSrc, jlong addrRgbaDst)
{
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeDetect enter");
    try
    {
        Mat& mSrc = *(Mat*)addrRgbaSrc;
        Mat& mDst = *(Mat*)addrRgbaDst;
        //cv::Mat marker = cv::imread("/storage/emulated/0/Pictures/Second\ Sight/1400165301259.png");
        //cv::Mat marker = cv::imread("./drawable/template.jpg");
//        if (marker.data)
//        {
//        	LOGD("Height: %d Width: %d",marker.rows,marker.cols);
//             //cout << "Height: " << marker.rows << " Width: " << marker.cols << endl;
//        }
        //marker  mSrc = inputFrame , mDst = processed frame
        //OrbAndroid newPerspective(&marker,&mSrc);
        newPerspective.updateFrame(&mSrc);
        newPerspective.estimate_perspective();
        newPerspective.getAugmented(&mDst);
    }
    catch(cv::Exception& e)
    {
        LOGD("nativeCreateObject caught cv::Exception: %s", e.what());
        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if(!je)
            je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, e.what());
    }
    catch (...)
    {
        LOGD("nativeDetect caught unknown exception");
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code FrameProcessor.nativeProcess()");
    }
    LOGD("Java_com_tbf_jnitester_FrameProcessor_nativeProcess exit");
}
