LOCAL_PATH	:=	$(call my-dir)

include	$(CLEAR_VARS)

# OpenCV
OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=STATIC

include /home/abel/workspace/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE	:=	libraryName
LOCAL_SRC_FILES	:=	code.cc
LOCAL_LDLIBS	:=	-llog	-ldl

include	$(BUILD_SHARED_LIBRARY)