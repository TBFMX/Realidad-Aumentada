#include <jni.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <cassert>
#include <stdexcept>

extern "C"
{
	JNIEXPORT jstring JNICALL Java_com_tbf_camtest_MainActivity_SayHello(JNIEnv *env, jobject thiz, jstring name);
}
