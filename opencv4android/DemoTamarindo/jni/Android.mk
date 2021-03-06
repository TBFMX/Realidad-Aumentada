LOCAL_PATH := $(call my-dir)

include	$(CLEAR_VARS)

# OpenCV
OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=STATIC

include /home/abel/workspace/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := NativeCamera
LOCAL_CFLAGS    := -Wall
LOCAL_SRC_FILES += GeometryTypes.cpp CameraCalibration.cpp geometryStructs.cpp ARPipeline.cpp  \
					ARDrawingContext.cpp  Pattern.cpp PatternDetector.cpp  CameraRenderer.cpp
LOCAL_LDLIBS    += -llog -landroid -lEGL -lGLESv1_CM

include $(BUILD_SHARED_LIBRARY)
