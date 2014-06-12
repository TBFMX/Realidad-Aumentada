#include <jni.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <cassert>
#include <stdexcept>

using namespace std;

extern "C"
{
	JNIEXPORT jstring JNICALL Java_HelloJNI_SayHello(JNIEnv *env, jobject thiz, jstring name);
}
