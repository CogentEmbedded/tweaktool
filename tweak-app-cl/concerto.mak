
TWEAK_PATH  := $(VISION_APPS_PATH)/utils/cogent_tweaktool

########################################################################
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU),A72)

include $(PRELUDE)

TARGET      := tweak-app-cl
TARGETTYPE  := exe
CSOURCES    :=

IDIRS       += $(TWEAK_PATH)/extern/uthash/include
IDIRS       += $(TWEAK_PATH)/tweak-app/include
IDIRS       += $(TWEAK_PATH)/tweak-common/include
IDIRS       += $(TWEAK_PATH)/tweak-json/include
IDIRS       += $(TWEAK_PATH)/tweak-metadata/include

CSOURCES    += src/main.c
CSOURCES    += src/metadatautil.c
CSOURCES    += src/stringutil.c
CSOURCES    += src/tweakuriutil.c

ifeq ($(TARGET_OS),LINUX)
    IDIRS       += $(LINUX_FS_PATH)/usr/include
    LDIRS       += $(LINUX_FS_PATH)/lib
    STATIC_LIBS += nng
    SYS_SHARED_LIBS += pthread
endif

ifeq ($(TARGET_OS),QNX)
    IDIRS       += $(QNX_TARGET)/usr/include
    STATIC_LIBS += nng
    SYS_SHARED_LIBS += socket
    ifneq ("$(wildcard $(QNX_TARGET)/aarch64le/lib/libregex.*)","")
        # SDP 710 has a separate regex library
        SYS_SHARED_LIBS += regex
    endif
endif

include $(VISION_APPS_PATH)/apps/concerto_a72_inc.mak

STATIC_LIBS += cogent_tweaktool
SYS_SHARED_LIBS += readline ncurses

include $(FINALE)

endif
endif
