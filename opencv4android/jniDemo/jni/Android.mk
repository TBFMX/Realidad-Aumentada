LOCAL_PATH	:=	$(call my-dir)

include	$(CLEAR_VARS)

LOCAL_MODULE	:=	mixed_sample
LOCAL_SRC_FILES	:=	code.cc
LOCAL_LDLIBS	:=	-llog

include	$(BUILD_SHARED_LIBRARY)