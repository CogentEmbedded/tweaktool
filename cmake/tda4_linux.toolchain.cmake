# CMake toolchain file for TDA4/J721E Linux
#
# Copyright (c) 2022, Cogent Embedded, Inc. All rights reserved.

if(NOT DEFINED ENV{OE_CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Yocto SDK is not configured, failed to find SDK toolchain file")
endif()

include($ENV{OE_CMAKE_TOOLCHAIN_FILE})

set(BSP_TOPDIR "$ENV{PSDKR_PATH}")
set(TDA_PLATFORM_DIR "${BSP_TOPDIR}/targetfs")

set(CMAKE_FIND_ROOT_PATH $ENV{OECORE_TARGET_SYSROOT} ${TDA_PLATFORM_DIR}/usr)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSOC_J721E" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSOC_J721E" CACHE INTERNAL "" FORCE)
