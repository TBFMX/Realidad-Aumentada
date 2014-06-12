Create a java class with
$ javac SquareWrapper.java
Create a header file with the next command
$ javah SquareWrapper

If you application includes other OpenCV-dependent native libraries you should load them after OpenCV initialization:
static {
    if (!OpenCVLoader.initDebug()) {
        // Handle initialization error
    } else {
        System.loadLibrary("my_jni_lib1");
        System.loadLibrary("my_jni_lib2");
    }
}


# for NDK r8b and later:
${NDKROOT}/platforms/android-9/arch-arm/usr/include
${NDKROOT}/sources/cxx-stl/gnu-libstdc++/4.6/include
${NDKROOT}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include
${ProjDirPath}/../../sdk/native/jni/include

#adjusted according our system
${ANDROID_NDK}/platforms/android-19/arch-arm/usr/include
${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.6/include
${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include
/home/abel/workspace/OpenCV-2.4.9-android-sdk/sdk/native/jni/include