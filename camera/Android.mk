LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    CameraWrapper.cpp \
    Camera2Wrapper.cpp \
    Camera3Wrapper.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware liblog libcamera_client libutils libcutils

LOCAL_C_INCLUDES += \
    system/core/include \
    system/media/camera/include

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE := camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
