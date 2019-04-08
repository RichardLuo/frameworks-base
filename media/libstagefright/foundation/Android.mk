LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CXX11_ENABLED := yes

LOCAL_SRC_FILES:=                 \
    AAtomizer.cpp                 \
    ABitReader.cpp                \
    ABuffer.cpp                   \
    AHandler.cpp                  \
    AHierarchicalStateMachine.cpp \
    ALooper.cpp                   \
    ALooperRoster.cpp             \
    AMessage.cpp                  \
    XMessage.cpp                  \
    AString.cpp                   \
    base64.cpp                    \
    hexdump.cpp

LOCAL_C_INCLUDES:= \
    frameworks/base/include/media/stagefright/foundation

LOCAL_SHARED_LIBRARIES := \
        libbinder         \
        libutils          \

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE:= libstagefright_foundation

ifneq ($(TARGET_SIMULATOR),true)
LOCAL_C_INCLUDES += bionic		# very important!
LOCAL_C_INCLUDES += external/stlport/stlport 
LOCAL_SHARED_LIBRARIES += libstlport libdl
endif

include $(BUILD_SHARED_LIBRARY)
