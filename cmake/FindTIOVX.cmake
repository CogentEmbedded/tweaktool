#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2022-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

#
# Finds the TIOVX framework includes and libraries
#
#  TIOVX_FOUND      - True if TIOVX framework was found
#  TIOVX::TIOVX     - TIOVX imported target
#

# For PC simulator and QNX builds use absolute paths
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_NAME STREQUAL QNX)
  if (NOT TI_BUILD_PROFILE)
    set(TI_BUILD_PROFILE "release")
  endif()

  find_path(TIDL_INCLUDE_DIR
    NAMES itidl_ti.h
    PATHS ${PSDKR_TIDL_PATH}/ti_dl/inc
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )

  find_path(TIOVX_COMMON_INCLUDE_DIR
    NAMES include/TI/tivx.h
    PATHS ${BSP_TOPDIR}/tiovx
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )

  find_path(VISION_APPS_INCLUDE_DIR
    NAMES utils/console_io/include/app_log.h
    PATHS ${BSP_TOPDIR}/vision_apps
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )

  find_path(VISION_APPS_INIT_INCLUDE_DIR
    NAMES app_init.h
    PATHS
      ${BSP_TOPDIR}/vision_apps/apps/basic_demos/app_tirtos/tirtos_qnx/mpu1 # PSDK 7.x
      ${BSP_TOPDIR}/vision_apps/utils/app_init/include # PSDK 8
      NO_DEFAULT_PATH
      NO_CMAKE_FIND_ROOT_PATH
    )

  find_path(VISION_APPS_RTOS_INCLUDE_DIR
    NAMES common/app.h
    PATHS
      ${BSP_TOPDIR}/vision_apps/apps/basic_demos/app_tirtos # PSDK 7.x
      ${BSP_TOPDIR}/vision_apps/platform/j721e/rtos # PSDK 8
      NO_DEFAULT_PATH
      NO_CMAKE_FIND_ROOT_PATH
    )

  find_path(IMAGING_INCLUDE_DIR
    NAMES sensor_drv/include/iss_sensors.h
    PATHS ${BSP_TOPDIR}/imaging
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )

  find_path(CE_IMAGING_INCLUDE_DIR
    NAMES sensor_drv/include/ice_sensors.h
    PATHS ${BSP_TOPDIR}/ce-imaging
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )

  find_path(IVISION_INCLUDE_DIR
    NAMES ivision.h
    PATHS ${BSP_TOPDIR}/ivision
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )

  find_path(PDK_INCLUDE_DIR
    NAMES packages/ti/csl/csl.h
    PATHS ${PSDKR_PDK_JACINTO_PATH}
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )

  if (CMAKE_SYSTEM_NAME STREQUAL QNX)
    find_library(TIOVX_LIBRARY
      tivision_apps
      PATHS "${BSP_TOPDIR}/vision_apps/out/J7/A72/QNX/${TI_BUILD_PROFILE}"
      NO_DEFAULT_PATH
      NO_CMAKE_FIND_ROOT_PATH)

   find_path(PDK_QNX_DIR
      NAMES ti/drv/ipc/ipc.h
      PATHS ${BSP_TOPDIR}/psdkqa/pdk/packages
      NO_DEFAULT_PATH
      NO_CMAKE_FIND_ROOT_PATH)

  else()
    find_library(TIOVX_LIBRARY
      tivision_apps
      PATHS "${BSP_TOPDIR}/vision_apps/out/PC/x86_64/LINUX/${TI_BUILD_PROFILE}"
      NO_DEFAULT_PATH
      NO_CMAKE_FIND_ROOT_PATH)
  endif()
else()
  # For target build lookup only in devkit sysroot
  find_path(TIDL_INCLUDE_DIR
    NAMES itidl_ti.h
    PATH_SUFFIXES processor_sdk/tidl_j7_01_03_00_11/ti_dl/inc # PSDK 7.01
                  processor_sdk/tidl_j7_02_00_00_07/ti_dl/inc # PSDK 7.03
                  processor_sdk/tidl_j7_08_00_00_10/ti_dl/inc # PSDK 8.00
                  processor_sdk/tidl_j7/ti_dl/inc # Symlink since PSDK 8.01
                  processor_sdk/tidl_j721e_08_04_00_12/ti_dl/inc # Symlink since PSDK 8.01
  )

  find_path(TIOVX_COMMON_INCLUDE_DIR
    NAMES include/TI/tivx.h
    PATH_SUFFIXES processor_sdk/tiovx)

  find_path(VISION_APPS_INCLUDE_DIR
    NAMES utils/console_io/include/app_log.h
    PATH_SUFFIXES processor_sdk/vision_apps)

  find_path(VISION_APPS_INIT_INCLUDE_DIR
    NAMES app_init.h
    PATH_SUFFIXES
      processor_sdk/vision_apps/apps/basic_demos/app_tirtos/tirtos_linux/mpu1 # PSDK 7.x
      processor_sdk/vision_apps/apps/basic_demos/app_rtos/rtos_linux/mpu1 # PSDK 8.00
      processor_sdk/vision_apps/utils/app_init/include # PSDK 8+
  )

  find_path(VISION_APPS_RTOS_INCLUDE_DIR
    NAMES common/app.h
    PATH_SUFFIXES
      processor_sdk/vision_apps/apps/basic_demos/app_tirtos # PSDK 7.x
      processor_sdk/vision_apps/apps/basic_demos/app_rtos # PSDK 8.00
      processor_sdk/vision_apps/platform/j721e/rtos # PSDK 8+
  )

  find_path(IMAGING_INCLUDE_DIR
    NAMES sensor_drv/include/iss_sensors.h
    PATH_SUFFIXES processor_sdk/imaging)

  find_path(CE_IMAGING_INCLUDE_DIR
    NAMES sensor_drv/include/ice_sensors.h
    PATH_SUFFIXES processor_sdk/ce-imaging)

  find_path(IVISION_INCLUDE_DIR
    NAMES ivision.h
    PATH_SUFFIXES processor_sdk/ivision)

  find_library(TIOVX_LIBRARY tivision_apps)
endif()

set(TIOVX_INCLUDE_DIRS
  ${TIDL_INCLUDE_DIR}
  ${TIOVX_COMMON_INCLUDE_DIR}/include
  ${TIOVX_COMMON_INCLUDE_DIR}/kernels/include
  ${TIOVX_COMMON_INCLUDE_DIR}/kernels_j7/include
  ${TIOVX_COMMON_INCLUDE_DIR}/utils/include
  ${VISION_APPS_INCLUDE_DIR}
  ${VISION_APPS_INCLUDE_DIR}/kernels/img_proc/include
  ${VISION_APPS_INIT_INCLUDE_DIR}
  ${VISION_APPS_RTOS_INCLUDE_DIR}
  ${IMAGING_INCLUDE_DIR}
  ${IMAGING_INCLUDE_DIR}/kernels/include
  ${IVISION_INCLUDE_DIR}
)

# It is used on selected platforms, append if found
if (CE_IMAGING_INCLUDE_DIR)
  list(APPEND TIOVX_INCLUDE_DIRS ${CE_IMAGING_INCLUDE_DIR})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL QNX)
  list(APPEND TIOVX_INCLUDE_DIRS ${PDK_QNX_DIR})
endif()

# SDK 8.01 or later
if (EXISTS ${VISION_APPS_INCLUDE_DIR}/utils/app_init/include)
  list(APPEND TIOVX_INCLUDE_DIRS ${VISION_APPS_INCLUDE_DIR}/utils/app_init/include)
endif()

message(STATUS "TIOVX_INCLUDE_DIRS: ${TIOVX_INCLUDE_DIRS}")
message(STATUS "TIOVX_LIBRARY: ${TIOVX_LIBRARY}")

get_filename_component(TIOVX_LIBRARY_REALPATH ${TIOVX_LIBRARY} REALPATH)
get_filename_component(TIOVX_LIBRARY_EXT ${TIOVX_LIBRARY_REALPATH} EXT)

if (TIOVX_LIBRARY_EXT MATCHES "^\\.so\\..+$")
  string(REGEX REPLACE "^\\.so\\.([0-9]+)\\.[0-9]+\\.[0-9]+$" "\\1" PSDK_MAJOR_VERSION ${TIOVX_LIBRARY_EXT})
  string(REGEX REPLACE "^\\.so\\.[0-9]+\\.([0-9]+)\\.[0-9]+$" "\\1" PSDK_MINOR_VERSION ${TIOVX_LIBRARY_EXT})
  message(STATUS "PSDK VERSION: ${PSDK_MAJOR_VERSION}.${PSDK_MINOR_VERSION}")
  if (PSDK_MAJOR_VERSION LESS "8" OR (PSDK_MAJOR_VERSION EQUAL "8" AND PSDK_MINOR_VERSION EQUAL "0"))
    add_definitions(-DHAS_TIOVX_J7_INCLUDE)
  endif()
else()
  message(STATUS "TIOVX library is not a symlink to soversion, realpath: ${TIOVX_LIBRARY_REALPATH}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("TIOVX" DEFAULT_MSG TIOVX_INCLUDE_DIRS TIOVX_LIBRARY)

if(TIOVX_FOUND)
  if (NOT TARGET TIOVX::TIOVX)
    add_library(TIOVX::TIOVX UNKNOWN IMPORTED)

    if (CMAKE_CROSSCOMPILING)
      if (NOT DEFINED TDA_PLATFORM_DIR)
        message(CRITICAL "TDA_PLATFORM_DIR is not set in toolchain file")
      endif()

      target_link_options(TIOVX::TIOVX INTERFACE
        "-Wl,-rpath,/usr/lib"
        "-Wl,-rpath-link,${TDA_PLATFORM_DIR}/usr/lib"
        "-Wl,-rpath-link,${TDA_PLATFORM_DIR}/lib"
      )

      if (EXISTS "${TDA_PLATFORM_DIR}/tilib")
        target_link_options(TIOVX::TIOVX INTERFACE "-Wl,-rpath-link,${TDA_PLATFORM_DIR}/tilib")
      endif()

      find_package(Python QUIET COMPONENTS Interpreter Development)
      if (Python_FOUND)
        string(REPLACE x86_64-arago-linux aarch64-linux TARGET_SITELIB ${Python_SITELIB})
        find_library(DLR_LIBRARY dlr
          ${TDA_PLATFORM_DIR}/usr/lib/python3.8/site-packages/dlr
          ${TARGET_SITELIB}/dlr
        )
        if (DLR_LIBRARY)
          target_link_options(TIOVX::TIOVX INTERFACE
            "-Wl,-rpath=/usr/lib/python3.8/site-packages/dlr"
            "-Wl,-rpath-link=${TDA_PLATFORM_DIR}/usr/lib/python3.8/site-packages/dlr"
          )
          target_link_libraries(TIOVX::TIOVX INTERFACE ${DLR_LIBRARY})
        else()
          message(WARNING "dlr library: not found")
        endif()
      endif()
    else()
      target_link_options(TIOVX::TIOVX INTERFACE "-Wl,--allow-shlib-undefined")
      # Some TI includes have static inline/struct definitions that depend on these defines
      target_compile_definitions(TIOVX::TIOVX INTERFACE x86_64 HOST_EMULATION)
    endif()

    set_target_properties(TIOVX::TIOVX PROPERTIES
      IMPORTED_LOCATION "${TIOVX_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${TIOVX_INCLUDE_DIRS}"
    )
  endif()
endif()

mark_as_advanced(TIOVX_INCLUDE_DIRS TIOVX_LIBRARY)
