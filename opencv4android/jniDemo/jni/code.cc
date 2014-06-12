#include <jni.h>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <cassert>
#include <stdexcept>
#include "code.h"
 
using namespace std;
 
JNIEXPORT jstring JNICALL Java_com_tbf_jnidemo_MainActivity_SayHello(JNIEnv *env, jobject thiz, jstring name)
{
  try
  {
    const char *myName = env->GetStringUTFChars(name, 0);
    std::string nameCPP(myName);
    nameCPP = "Hello: " + nameCPP;
    return env->NewStringUTF(nameCPP.c_str());
  }
  catch (exception &ex)
  {
    const char *error = "Failed";
    return env->NewStringUTF(error);
  }
}
