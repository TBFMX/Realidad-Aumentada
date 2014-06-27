#include <jni.h>
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <iostream>

#include "NativeLibrary.h"
#include "jniExports.h"

JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_initializeAssetManager(JNIEnv* env, jclass clazz, jobject assetManager)
{
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
}

JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_init(JNIEnv* env, jclass clazz)
{
	InitializeOpenGL();
}

JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_resize(JNIEnv* env, jclass clazz, jint width, jint height)
{
	resizeViewport(width, height);
}

JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_render(JNIEnv* env, jclass clazz, jfloatArray projection, jfloatArray pose)
{
	jfloat* jprojection = env->GetFloatArrayElements(projection, NULL);
	float* _projection =jprojection;
	jfloat* jpose = env->GetFloatArrayElements(pose,NULL);
	float* _pose = jpose;
	renderFrame(_projection, _pose);
	env->ReleaseFloatArrayElements(projection, jprojection, JNI_ABORT);
	env->ReleaseFloatArrayElements(pose, jpose, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_videorender(JNIEnv* env, jclass clazz, jfloatArray projection, jfloatArray pose)
{
	jfloat* jprojection = env->GetFloatArrayElements(projection, NULL);
	float* _projection =jprojection;
	jfloat* jpose = env->GetFloatArrayElements(pose,NULL);
	float* _pose = jpose;
	renderVideoFrame(_projection, _pose);
	env->ReleaseFloatArrayElements(projection, jprojection, JNI_ABORT);
	env->ReleaseFloatArrayElements(pose, jpose, JNI_ABORT);
}

JNIEXPORT jstring JNICALL Java_com_tbf_glsurface_LibraryClass_ndkopenfile (JNIEnv *env, jclass clazz)
{
        char myStr[20];
        FILE* fp = fopen("/sdcard/x.txt","w+");
        if(fp!=NULL)
        {
            fgets(myStr,20,fp);
            fflush(fp);
            fclose(fp);
            return env->NewStringUTF(myStr);
        }
        else
        {
            fclose(fp);
            return env->NewStringUTF("Error opening file!");
        }
}
