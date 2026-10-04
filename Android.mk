LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cpp .cc
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
	LOCAL_MODULE := BytEmber.SA
else
	LOCAL_MODULE := BytEmber.SA64
endif
LOCAL_SRC_FILES := main.cpp game_sa.cpp script_host.cpp mod/logger.cpp
LOCAL_SRC_FILES += BytEmber/src/registry.cpp BytEmber/src/runtime.cpp BytEmber/src/value.cpp \
                   BytEmber/src/compiler.cpp BytEmber/src/bytecode.cpp BytEmber/src/vm.cpp \
                   BytEmber/src/native.cpp BytEmber/src/std/stdlib.cpp BytEmber/src/std/math.cpp \
                   BytEmber/src/std/memory.cpp BytEmber/src/std/strings.cpp BytEmber/src/std/format.cpp \
                   BytEmber/src/std/utility.cpp BytEmber/src/std/scan.cpp BytEmber/src/std/sort.cpp \
                   BytEmber/src/std/constants.cpp BytEmber/src/std/time.cpp
LOCAL_CFLAGS += -O2 -DNDEBUG -std=c++14 -fexceptions -fvisibility=hidden -fno-rtti -fno-threadsafe-statics -fno-unwind-tables -fno-asynchronous-unwind-tables
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_CFLAGS += -mfloat-abi=softfp
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/BytEmber $(LOCAL_PATH)/BytEmber/include
LOCAL_LDLIBS += -llog -ldl
include $(BUILD_SHARED_LIBRARY)
