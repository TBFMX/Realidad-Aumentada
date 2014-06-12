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
JNIEXPORT jstring JNICALL Java_blog_neonigma_AndroidActivity_SayHello(JNIEnv *env, jobject thiz, jstring name)
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
}
