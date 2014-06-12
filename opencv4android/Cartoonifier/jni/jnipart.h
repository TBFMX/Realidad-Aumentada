/*
 * jnipart.h
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

#ifndef JNIPART_H_
#define JNIPART_H_
extern "C" {

	JNIEXPORT void JNICALL Java_com_tbf_cartoonifier_CartoonifierView_ShowPreview(
		JNIEnv* env, jobject, jint width, jint height, jbyteArray yuv, jintArray bgra);

	JNIEXPORT void JNICALL Java_com_tbf_cartoonifier_CartoonifierView_CartoonifyImage(
			JNIEnv *, jobject, jint width, jint height, jbyteArray yuv, jintArray bgra);

}
#endif /* JNIPART_H_ */
