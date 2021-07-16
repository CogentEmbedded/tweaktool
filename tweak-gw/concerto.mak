ifeq ($(TARGET_CPU),A72)
ifeq ($(TARGET_OS), LINUX)

include $(PRELUDE)

TARGETTYPE  := exe
TARGET      := tweak-gw
CSOURCES    := main.c

STATIC_LIBS += cogent_tweaktool
STATIC_LIBS += nng

IDIRS       += $(TIOVX_PATH)/source/tweaktool/v2/tweak-app/include
IDIRS       += $(TIOVX_PATH)/source/tweaktool/v2/tweak-common/include
IDIRS       += $(TIOVX_PATH)/source/tweaktool/v2/tweak-pickle/include
IDIRS       += $(TIOVX_PATH)/source/tweaktool/v2/tweak-wire/include
IDIRS       += $(TIOVX_PATH)/source/tweaktool/v2/tweak2lib/include

IDIRS       += $(VISION_APPS_PATH)/

include $(VISION_APPS_PATH)/apps/concerto_a72_inc.mak

include $(FINALE)

endif
endif

