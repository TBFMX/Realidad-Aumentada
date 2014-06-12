
#ifndef JNIEXPORTS_H
#define JNIEXPORTS_H
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_initializeAssetManager(JNIEnv* env, jclass clazz, jobject assetManager);
JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_init(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_resize(JNIEnv* env, jclass clazz, jint width, jint height);
JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_render(JNIEnv* env, jclass clazz, jfloatArray projection, jfloatArray pose);
JNIEXPORT void JNICALL Java_com_tbf_glsurface_LibraryClass_videorender(JNIEnv* env, jclass clazz, jfloatArray projection, jfloatArray pose);
JNIEXPORT jstring JNICALL Java_com_tbf_glsurface_LibraryClass_ndkopenfile (JNIEnv *env, jclass clazz);

#ifdef __cplusplus
}
#endif

#endif
