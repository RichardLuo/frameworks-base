# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# we have the common sources, plus some device-specific stuff
sources := \
	AppOpsManager.cpp \
	Binder.cpp \
	BpBinder.cpp \
	BufferedTextOutput.cpp \
	CursorWindow.cpp \
	Debug.cpp \
	IAppOpsCallback.cpp \
	IAppOpsService.cpp \
	IBatteryStats.cpp \
	IInterface.cpp \
	IMemory.cpp \
	IPCThreadState.cpp \
	IPermissionController.cpp \
	IProcessInfoService.cpp \
	IServiceManager.cpp \
	MemoryBase.cpp \
	MemoryDealer.cpp \
	MemoryHeapBase.cpp \
	MemoryHeapPmem.cpp \
	Parcel.cpp \
	PermissionCache.cpp \
	ProcessInfoService.cpp \
	ProcessState.cpp \
	Static.cpp \
	TextOutput.cpp


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS += -lpthread
LOCAL_MODULE := libbinder
LOCAL_CFLAGS += -std=c++11
LOCAL_C_INCLUDES += bionic/libc/kernel/common
LOCAL_SHARED_LIBRARIES := liblog libcutils libutils
LOCAL_SRC_FILES := $(sources)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS += -lpthread
LOCAL_MODULE := libbinder
LOCAL_SRC_FILES := $(sources)
LOCAL_CFLAGS += -std=c++11
LOCAL_C_INCLUDES += bionic/libc/kernel/common
include $(BUILD_STATIC_LIBRARY)
