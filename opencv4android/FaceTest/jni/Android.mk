LOCAL_PATH	:=	$(call my-dir)

include	$(CLEAR_VARS)

include /home/abel/workspace/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE	:=	native_sample
LOCAL_SRC_FILES	:=	jni_part.ccp
LOCAL_LDLIBS	:=	-llog	-ldl

include	$(BUILD_SHARED_LIBRARY)