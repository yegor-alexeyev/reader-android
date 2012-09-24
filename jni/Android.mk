LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include /opt/opencv.org/OpenCV-2.4.2-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_LDLIBS += -llog
LOCAL_MODULE := preview-processor
LOCAL_SRC_FILES := preview-processor.cpp

include $(BUILD_SHARED_LIBRARY)
