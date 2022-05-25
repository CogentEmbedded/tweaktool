ifeq ($(TARGET_CPU),A72)
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)

TARGETTYPE  := exe
TARGET      := tweak-gw
CSOURCES    := main.c

STATIC_LIBS += cogent_tweaktool
STATIC_LIBS += nng

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), QNX))
SYS_SHARED_LIBS += socket
endif

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX))
SYS_SHARED_LIBS += pthread
endif

TWEAK_PATH  := $(VISION_APPS_PATH)/utils/cogent_tweaktool

IDIRS       += $(TWEAK_PATH)/tweak-common/include
IDIRS       += $(TWEAK_PATH)/tweak-wire/include

IDIRS       += $(VISION_APPS_PATH)/

include $(VISION_APPS_PATH)/apps/concerto_a72_inc.mak

include $(FINALE)

endif
endif

