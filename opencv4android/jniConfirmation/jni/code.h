#include <jni.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <cassert>
#include <stdexcept>

#ifndef _CODE_H
#define _CODE_H
#ifdef __cplusplus
extern "C" {
#endif
	JNIEXPORT jstring JNICALL Java_com_tbf_jniconfirmation_Nativa_sayHello(JNIEnv *env, jobject thiz, jstring name);

#ifdef __cplusplus
}
#endif
#endif
