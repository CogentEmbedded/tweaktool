#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2018-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

# ------------------------------------------------------------------------------
# Check CMake version
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)

# ------------------------------------------------------------------------------
# Generate project version
# ------------------------------------------------------------------------------
set(PROJECT_NUMERIC_VERSION "2.1.2") # fallback
include(cmake/generate_version.cmake)

# ------------------------------------------------------------------------------
# Set default compiler options policy for Visual Studio
# ------------------------------------------------------------------------------
if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()

# ------------------------------------------------------------------------------
# Create project
# ------------------------------------------------------------------------------
project(
  tweak2
  VERSION "${PROJECT_NUMERIC_VERSION}"
  LANGUAGES C CXX)

set(PROJECT_FULL_NAME "Cogent Tweak Tool")
set(PROJECT_SUMMARY "Cogent Real-Time Tuning Tool (Tweak)")

message(
  STATUS
    "Project version: ${PROJECT_VERSION_STRING} (${PROJECT_NUMERIC_VERSION})")
message(STATUS "Project description: ${PROJECT_SUMMARY}")

# ------------------------------------------------------------------------------
# User-configurable options
# ------------------------------------------------------------------------------
include(CMakeDependentOption)

# cmake-format: off
# <Option name>        <Description/Help>                <Default value>
option(BUILD_TESTS     "Build tests"                         OFF)
option(BUILD_GUI       "Build GUI tools"                     OFF)
option(BUILD_MOCK      "Build mock server"                   ON)
# CLI not ported to win32
cmake_dependent_option(BUILD_CLI "Build command line client" ON "NOT MSVC" OFF)
option(WITH_DOXYGEN    "Enable Doxygen-based documentation"  OFF)
option(WITH_PYTHON     "Build Python 3 binding"               ON)
option(WITH_WIRE_NNG   "Build NNG wire backend"               ON)
# cmake-format: on

set(WIRE_RPMSG_BACKENDS OFF TI_API CHRDEV)
set(WIRE_RPMSG_BACKEND
    OFF
    CACHE STRING "Select backend for RPMSG wire")
set_property(CACHE WIRE_RPMSG_BACKEND PROPERTY STRINGS ${WIRE_RPMSG_BACKENDS})

set(TWEAK_COMMON_LOG_LEVELS Trace Debug Test Warn Error Fatal)
list(LENGTH TWEAK_COMMON_LOG_LEVELS TWEAK_COMMON_LOG_LEVELS_LENGTH)
set(TWEAK_COMMON_LOG_LEVEL
    Test
    CACHE STRING "Statically disable log levels less severe than this")
set_property(CACHE TWEAK_COMMON_LOG_LEVEL PROPERTY STRINGS
                                                   ${TWEAK_COMMON_LOG_LEVELS})

# ------------------------------------------------------------------------------
# Global project configuration
# ------------------------------------------------------------------------------

set(CMAKE_POSITION_INDEPENDENT_CODE YES)

set(PROJECT_VENDOR "Cogent Embedded, Inc.")
add_definitions(-DPROJECT_VENDOR=\"${PROJECT_VENDOR}\")
set(PROJECT_DOMAIN "cogentembedded.com")
set(PROJECT_URI "http://cogentembedded.com")
add_definitions(-DPROJECT_DOMAIN=\"${PROJECT_DOMAIN}\")
add_definitions(-DPROJECT_SUMMARY=\"${PROJECT_SUMMARY}\")
add_definitions(-DPROJECT_FULL_NAME=\"${PROJECT_FULL_NAME}\")

set(PROJECT_NAMESPACE tweak2)

set(GUI_TOOL_NAME tweak-gui)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

# search for additional CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(BUILD_TESTS)

  # Acutest has MIT license
  include(ExternalProject)
  ExternalProject_Add(
    Acutest
    GIT_REPOSITORY https://github.com/mity/acutest.git
    GIT_TAG 181179d747fbec02f0123429bdd0a8bff8c22064
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND "")
  ExternalProject_Get_Property(Acutest SOURCE_DIR)
  message(STATUS "Acutest directory: ${SOURCE_DIR}")

  include_directories(${SOURCE_DIR}/include)
  unset(SOURCE_DIR)

endif()

# TODO: Reconsider if we still need nanopb in the main CMakeLists.txt
set(NANOPB_SRC_ROOT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/extern/nanopb)
set(NANOPB_GENERATOR_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/nanopb/generator)
list(APPEND CMAKE_MODULE_PATH ${NANOPB_SRC_ROOT_FOLDER}/extra)

include(InstallRequiredSystemLibraries)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(GenerateExportHeader)
include(AppendToGlobalList)
include(TweakComponentInstall)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Asan")
  if(MSVC)
    add_compile_options(/Zi /fsanitize=address
                        /fsanitize-address-use-after-return)
  else()
    include(Asan)
  endif()
endif()

# Prefer system version of uthash packages. Use a submodule if not available.
find_package(uthash)
if(UTHASH_INCLUDE_DIR)
  message(STATUS "Using system uthash: ${UTHASH_INCLUDE_DIR}")
else()
  find_path(
    UTHASH_INCLUDE_DIR
    NAMES "uthash.h"
    PATHS "${CMAKE_CURRENT_LIST_DIR}/extern/uthash/include/"
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  if(UTHASH_INCLUDE_DIR)
    message(STATUS "Using uthash from a submodule: ${UTHASH_INCLUDE_DIR}")
  else()
    message(
      FATAL_ERROR "uthash.h is NOT FOUND, please init and update all submodules"
    )
  endif()
endif()

if(BUILD_TESTS)
  enable_testing()
endif()

# ------------------------------------------------------------------------------
# Compilation flags
# ------------------------------------------------------------------------------

if (MSVC)
  add_compile_definitions(NOMINMAX _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
  set(VC_UTHASH_WARNINGS "/wd4127 /wd4702")
elseif()
  add_compile_options(
    -Wall -Wextra -pedantic -Werror
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>)
endif()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-deprecated-copy NO_DEPRECATED_COPY_SUPPORTED)
if(NO_DEPRECATED_COPY_SUPPORTED)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-copy>)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-ffile-prefix-map=${CMAKE_CURRENT_LIST_DIR}=
                          CXX_FILE_PREFIX_MAP_SUPPORTED)
  if(CXX_FILE_PREFIX_MAP_SUPPORTED)
    # Prevent build path leakage
    add_compile_options(-ffile-prefix-map=${CMAKE_CURRENT_LIST_DIR}=)
  endif()
endif()

# ------------------------------------------------------------------------------
# Libraries and executables
# ------------------------------------------------------------------------------

add_subdirectory(tweak-common)

add_subdirectory(tweak-wire)

add_subdirectory(tweak-pickle)

add_subdirectory(tweak-app)

add_subdirectory(tweak1lib)

add_subdirectory(tweak2lib)

if(BUILD_MOCK)
  add_subdirectory(tweak-mock-server)
endif()

if(BUILD_CLI)
  add_subdirectory(tweak-app-cl)
endif()

add_subdirectory(tweak-json)

add_subdirectory(tweak-metadata)

if(WITH_PYTHON)
  add_subdirectory(tweak-py)
endif()

if(BUILD_GUI)
  add_subdirectory(extern/SortFilterProxyModel)
  add_subdirectory(tweak-gui-qml)
endif()

# ------------------------------------------------------------------------------
# Generate CMake infrastructure
# ------------------------------------------------------------------------------

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/projectConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAMESPACE})

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

# ------------------------------------------------------------------------------
# Install CMake infrastructure
# ------------------------------------------------------------------------------

install(
  FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Version.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAMESPACE}
  COMPONENT dev)

# ------------------------------------------------------------------------------
# Building docs with collected sets of files
# ------------------------------------------------------------------------------

if(WITH_DOXYGEN)
  add_subdirectory(docs)
endif()

# ------------------------------------------------------------------------------
# Packaging
# ------------------------------------------------------------------------------

add_subdirectory(packaging)
