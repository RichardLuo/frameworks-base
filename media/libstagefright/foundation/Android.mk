LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

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

LOCAL_CFLAGS += -std=c++11

LOCAL_C_INCLUDES:= \
	bionic/libc/kernel/common \
    frameworks/base/include/media/stagefright/foundation

LOCAL_SHARED_LIBRARIES := \
        libbinder         \
        libutils          \

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE:= libstagefright_foundation



include $(BUILD_SHARED_LIBRARY)
