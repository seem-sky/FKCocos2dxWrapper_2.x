# macros to include all files with same extension

define all-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find $(1) -name "$(2)" -and -not -name ".*") \
 )
endef

define all-cpp-files-under
$(call all-files-under,$(1),*.cpp)
endef

define all-c-files-under
$(call all-files-under,$(1),*.c)
endef

# module

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sqlite
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_SRC_FILES := $(call all-c-files-under,sqlite3)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/sqlite3
LOCAL_C_INCLUDES := $(LOCAL_EXPORT_C_INCLUDES) \
	$(LOCAL_PATH)/sqlite3
include $(BUILD_STATIC_LIBRARY)
