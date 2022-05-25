ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F A72))

include $(PRELUDE)

TARGET      := cogent_tweaktool
TARGETTYPE  := library

DEFS        :=
CSOURCES    :=
IDIRS		:=

DEFS        += SOC_J721E

ifeq ($(TARGET_CPU),A72)
DEFS        += CPU_mpu1
endif
ifeq ($(TARGET_CPU),R5F)
DEFS        += CPU_mcu2_0
endif

################ Basic directories #################
TWEAK_PATH  := $(VISION_APPS_PATH)/utils/cogent_tweaktool


################ Auto-detect PSDK version #################
PSDK_MAJOR_VERSION ?= $(shell echo $(PSDK_VERSION) | head -c 1)
ifeq ($(PSDK_MAJOR_VERSION),7)
    IDIRS    += $(VISION_APPS_PATH)/apps/basic_demos/app_tirtos
endif
ifeq ($(PSDK_MAJOR_VERSION),8)
    IDIRS    += $(VISION_APPS_PATH)/platform/j721e/rtos
endif

################ Configuration options ################
ifeq ($(TARGET_OS),LINUX)
DEFS        += WIRE_RPMSG_BACKEND_CHRDEV
else
# All other OS (QNX, SYSBIOS, FreeRTOS) use RPMsg API directly
DEFS        += WIRE_RPMSG_BACKEND_TI_API
endif

DEFS        += WITH_WIRE_RPMSG
DEFS        += TWEAK_LOG_LEVEL=3

ifeq ($(TARGET_CPU),A72)
DEFS        += WITH_WIRE_NNG
endif

ifeq ($(TARGET_CPU),R5F)
DEFS        += TI_ARM_R5F
endif

################ Include directories #################
IDIRS       += $(VISION_APPS_PATH)
IDIRS       += $(XDCTOOLS_PATH)/packages

ifeq ($(TARGET_CPU),R5F)
IDIRS       += $(BIOS_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages/ti/posix/ccs
endif

ifeq ($(TARGET_CPU),A72)
IDIRS       += $(TWEAK_PATH)/nng-prebuilt/tda4/include
endif

ifeq ($(TARGET_OS),QNX)
IDIRS       += $(QNX_TARGET)/usr/include
IDIRS       += $(PDK_QNX_PATH)/packages
endif

ifeq ($(TARGET_OS),LINUX)
IDIRS       += $(LINUX_FS_PATH)/usr/include
endif

IDIRS       += $(TWEAK_PATH)/extern/uthash/include
IDIRS       += $(TWEAK_PATH)/tweak-app/include
IDIRS       += $(TWEAK_PATH)/tweak-common/include
IDIRS       += $(TWEAK_PATH)/tweak-json/include
IDIRS       += $(TWEAK_PATH)/tweak-metadata/include
IDIRS       += $(TWEAK_PATH)/tweak-pickle/include
IDIRS       += $(TWEAK_PATH)/tweak-pickle/src/autogen
IDIRS       += $(TWEAK_PATH)/tweak-wire/include
IDIRS       += $(TWEAK_PATH)/tweak2lib/include

################ Source files ########################
CSOURCES    += tweak-app/src/tweakappclient.c
CSOURCES    += tweak-app/src/tweakappcommon.c
CSOURCES    += tweak-app/src/tweakappfeatures.c
CSOURCES    += tweak-app/src/tweakappqueue.c
CSOURCES    += tweak-app/src/tweakappserver.c
CSOURCES    += tweak-app/src/tweakmodel.c
CSOURCES    += tweak-app/src/tweakmodel_uri_to_tweak_id_index.c
CSOURCES    += tweak-common/src/tweakbuffer.c
CSOURCES    += tweak-common/src/tweaklog_format_time_tda4.c
CSOURCES    += tweak-common/src/tweaklog_out_tda4.c
CSOURCES    += tweak-common/src/tweaklog_unix.c
CSOURCES    += tweak-common/src/tweakstring.c
CSOURCES    += tweak-common/src/tweakvariant.c
CSOURCES    += tweak-metadata/src/tweakmetadata.c
CSOURCES    += tweak-json/src/tweakjson.c
CSOURCES    += tweak-pickle/src/autogen/pb_common.c
CSOURCES    += tweak-pickle/src/autogen/pb_decode.c
CSOURCES    += tweak-pickle/src/autogen/pb_encode.c
CSOURCES    += tweak-pickle/src/autogen/tweak.pb.c
CSOURCES    += tweak-pickle/src/tweakpickle_client_pb.c
CSOURCES    += tweak-pickle/src/tweakpickle_pb_util.c
CSOURCES    += tweak-pickle/src/tweakpickle_server_pb.c
CSOURCES    += tweak-wire/src/tweakwire.c
CSOURCES    += tweak-wire/src/tweakwire_rpmsg.c
CSOURCES    += tweak2lib/src/tweak2.c

ifeq ($(TARGET_OS),LINUX)
CSOURCES    += tweak-wire/src/tweakwire_rpmsg_transport_linux.c
else
# All other OS (QNX, SYSBIOS, FreeRTOS) use RPMsg API directly
CSOURCES    += tweak-wire/src/tweakwire_rpmsg_transport_ti_api.c
endif

ifeq ($(TARGET_CPU),R5F)
CSOURCES    += tweak-common/src/tweaklog_out_tda4.c
CSOURCES    += tweak-common/src/tweaklog_thread_id_sysbios.c
CSOURCES    += tweak-common/src/tweak_id_gen_sysbios.c
endif

ifeq ($(TARGET_CPU),A72)
CSOURCES    += tweak-common/src/tweak_id_gen_sync_fetch_and_add.c
CSOURCES    += tweak-common/src/tweaklog_out_stderr.c
CSOURCES    += tweak-wire/src/tweakwire_nng.c

ifeq ($(TARGET_OS),LINUX)
CSOURCES    += tweak-common/src/tweaklog_thread_id_common.c
CSOURCES    += tweak-common/src/tweaklog_thread_id_linux_syscall.c
else
CSOURCES    += tweak-common/src/tweaklog_thread_id_fallback.c
endif

endif

################ Libraries ###########################
STATIC_LIBS += app_utils_console_io

ifeq ($(TARGET_CPU),A72)
STATIC_LIBS += nng
endif

################ Compiler options ####################
ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS      += --display_error_number
CFLAGS      += --c99
endif

ifeq ($(HOST_COMPILER),TIARMCGT_LLVM)
CFLAGS += -std=c99
endif

ifeq ($(TARGET_CPU),A72)
include $(VISION_APPS_PATH)/apps/concerto_a72_inc.mak
endif

include $(FINALE)

endif
