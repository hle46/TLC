MY_LOCAL_PATH := $(call my-dir)

include $(call all-subdir-makefiles)

LOCAL_PATH := $(MY_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE		:= tlc
LOCAL_SRC_FILES	 	:= imtoolbox.cc uiuc_bioassay_tlc_TLCApplication.cc
LOCAL_C_INCLUDES 	+= $(LOCAL_PATH)/libjpeg $(LOCAL_PATH)/libpng $(LOCAL_PATH)
LOCAL_STATIC_LIBRARIES 	:= libpng libjpeg 
LOCAL_CPP_FEATURES 	+= exceptions
LOCAL_CFLAGS            := -Wall -Wextra -funroll-loops -O3 -DANDROID
LOCAL_LDLIBS 		:= -lz -llog

include $(BUILD_SHARED_LIBRARY)


