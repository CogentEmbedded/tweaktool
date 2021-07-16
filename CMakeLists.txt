# ##############################################################################
#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# ##############################################################################

# ------------------------------------------------------------------------------
# Check CMake version
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)

# ------------------------------------------------------------------------------
# Generate project version
# ------------------------------------------------------------------------------
set(PROJECT_NUMERIC_VERSION "2.0.1")
include(cmake/generate_version.cmake)

# ------------------------------------------------------------------------------
# Create project
# ------------------------------------------------------------------------------
project(
  tweak2
  VERSION ${PROJECT_NUMERIC_VERSION}
  LANGUAGES C CXX)

set(PROJECT_SUMMARY "Cogent Real-Time Tuning Tool (Tweak)")

message(
  STATUS
    "Project version: ${PROJECT_VERSION_STRING} (${PROJECT_NUMERIC_VERSION})")
message(STATUS "Project description: ${PROJECT_SUMMARY}")

# ------------------------------------------------------------------------------
# User-configurable options
# ------------------------------------------------------------------------------
# cmake-format: off
# <Option name>        <Description/Help>                <Default value>
option(BUILD_TESTS     "Build tests"                         OFF)
option(BUILD_GUI       "Build host tools"                    OFF)
option(BUILD_CLI       "Build command line client"            ON)
option(WITH_DOXYGEN    "Enable Doxygen-based documentation"  OFF)
option(WITH_WIRE_NNG   "Build NNG wire backend"               ON)
option(WITH_WIRE_RPMSG "Build RPMSG wire backend"            OFF)
# cmake-format: on

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

set(PROJECT_VENDOR "Cogent Embedded Inc.")
add_definitions(-DPROJECT_VENDOR=\"${PROJECT_VENDOR}\")
set(PROJECT_DOMAIN "cogentembedded.com")
add_definitions(-DPROJECT_DOMAIN=\"${PROJECT_DOMAIN}\")
add_definitions(-DPROJECT_SUMMARY=\"${PROJECT_SUMMARY}\")

set(PROJECT_NAMESPACE ${CMAKE_PROJECT_NAME})

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
  find_package(Asan)
endif()

if(BUILD_TESTS)
  enable_testing()
endif()

# ------------------------------------------------------------------------------
# Generate version header
# ------------------------------------------------------------------------------

add_custom_command(OUTPUT always_rebuild COMMAND cmake -E echo)

add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/generated/version.h
  COMMAND
    ${CMAKE_COMMAND} -DHEADER_SRC=${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in
    -DHEADER_DST=${PROJECT_BINARY_DIR}/generated/version.h -P
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.cmake
  COMMENT "Generating version.h"
  MAIN_DEPENDENCY always_rebuild
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in
          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.cmake
  VERBATIM)

include_directories(${PROJECT_BINARY_DIR}/generated)

# ------------------------------------------------------------------------------
# Compilation flags
# ------------------------------------------------------------------------------

add_compile_options(
  -Wall -Wextra -pedantic -Werror
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>
)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-deprecated-copy NO_DEPRECATED_COPY_SUPPORTED)
if (NO_DEPRECATED_COPY_SUPPORTED)
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-copy>
  )
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

add_subdirectory(tweak-mock-server)

if(BUILD_CLI)
  add_subdirectory(tweak-app-cl)
endif()

add_subdirectory(tweak-json)

add_subdirectory(tweak-metadata)

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
