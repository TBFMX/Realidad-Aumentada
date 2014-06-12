/*
 * jnipart.cpp
 *
 *  Created on: 05/05/2014
 *      Author: abel
 */


#include <jni.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <cassert>
#include <stdexcept>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "jnipart.h"
#include <stdio.h>

//#include "/home/abel/opencvWork/cartoonifer/Cartoonifier_Desktop/cartoon.h"
using namespace std;
using namespace cv;

// Just show the plain camera image without modifying i.
JNIEXPORT void JNICALL Java_com_tbf_cartoonifier_CartoonifierView_ShowPreview(
		JNIEnv* env, jobject, jint width, jint height, jbyteArray yuv, jintArray bgra){
	 jbyte* _yuv = env->GetByteArrayElements(yuv,0);
	 jint* _bgra = env->GetIntArrayElements(bgra,0);

	 Mat myuv = Mat(height + height / 2, width, CV_8UC1, (uchar *)_yuv);
	 Mat mbgra = Mat(height, width, CV_8UC4, (uchar *)_bgra);

	// Convert the color format from the camera's
	// NV21 "YUV420sp" format to an Androiid BGRA color image.
	cvtColor(myuv, mbgra, CV_YUV420sp2BGRA);
	// OpenCV can now access/modify the BGRA image "mbfgra" ...

	env->ReleaseIntArrayElements(bgra, _bgra, 0);
	env->ReleaseByteArrayElements(yuv, _yuv,0);
}

// Modify the camera image using the Crotoonifier filter.
JNIEXPORT void JNICALL Java_com_tbf_cartoonifier_CartoonifierView_CartoonifyImage(
		JNIEnv* env, jobject, jint width, jint height, jbyteArray yuv, jintArray bgra){
	// Get native access to the given Java arrays.
	jbyte* _yuv = env->GetByteArrayElements(yuv,0);
	jint* _bgra = env->GetIntArrayElements(bgra, 0);

	// Create OpenCV wrappers around the input & output data.
	Mat myuv(height + height / 2, width, CV_8UC4, (uchar *)_yuv);
	Mat mbgra(height, width, CV_8UC4, (uchar *)_bgra);

	// Convert the color format from the camera's YUV420sp // semi-planar
	// format to OpenCV's default BGR color image.
	Mat mbgr(height, width, CV_8UC3); // Allocate a new image buffer.
	cvtColor(myuv, mbgr, CV_YUV420sp2BGR);

	// OpenCV can now access/modify the BGR image "mbgr", and should
	// store the output as the BGR image "displayedFrame".
	Mat displayedFrame(mbgr.size(), CV_8UC3);

	// TEMPORARY: Just show the camera image without modifying it.
	displayedFrame = mbgr;
	//cartoonifyImage(mbgr, displayedFrame);

	// Convert the output form OpenCV's BGR to Android's BGRA format.
	cvtColor(displayedFrame, mbgra, CV_BGR2BGRA);

	// Release the native lock we placed on the Java arrays.
	env->ReleaseIntArrayElements(bgra, _bgra, 0);
	env->ReleaseByteArrayElements(yuv, _yuv, 0);
}

