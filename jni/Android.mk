LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := preview-processor
LOCAL_SRC_FILES := preview-processor.c

include $(BUILD_SHARED_LIBRARY)
