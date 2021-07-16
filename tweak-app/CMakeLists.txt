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
# Common settings
# ------------------------------------------------------------------------------

set(LIBRARY_NAME app)
set(LIBRARY_SOVERSION 0)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

# Prefer system version of uthash packages. Use a submodule if not available.
find_package(uthash)
if (NOT UTHASH_INCLUDE_DIR)
  find_path(
    UTHASH_INCLUDE_DIR
    NAMES "uthash.h"
    PATHS "${CMAKE_CURRENT_LIST_DIR}/../extern/uthash/include/"
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  if(UTHASH_INCLUDE_DIR)
    message(STATUS "Using uthash from a submodule: ${UTHASH_INCLUDE_DIR}")
  else()
    message(
      FATAL_ERROR "uthash.h is NOT FOUND, please init and update all submodules"
    )
  endif()
endif()

set(${LIBRARY_NAME}_DEPENDENCIES ${PROJECT_NAMESPACE}::common
                                 ${PROJECT_NAMESPACE}::pickle)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/appcommon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/appclient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/appserver.h)

set(${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakappinternal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakappcommon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakappclient.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakappserver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakappqueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakappqueue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakmodel_uri_to_tweak_id_index.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakmodel_uri_to_tweak_id_index.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakmodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakmodel.c)

# ------------------------------------------------------------------------------
# Library generation
# ------------------------------------------------------------------------------

add_library(${LIBRARY_NAME} ${${LIBRARY_NAME}_SOURCES}
                            ${${LIBRARY_NAME}_HEADERS})

add_library(${PROJECT_NAMESPACE}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} PUBLIC ${${LIBRARY_NAME}_DEPENDENCIES}
                                             Threads::Threads)

target_compile_features(${LIBRARY_NAME} PUBLIC c_std_99)

set_target_properties(
  ${LIBRARY_NAME}
  PROPERTIES SOVERSION ${LIBRARY_SOVERSION} SONAME ${LIBRARY_NAME}
             OUTPUT_NAME ${PROJECT_NAME}${LIBRARY_NAME}
             VERSION "${PROJECT_NUMERIC_VERSION}")

# INSTALL_INTERFACE header path is specified during installation
target_include_directories(
  ${LIBRARY_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE $<BUILD_INTERFACE:${UTHASH_INCLUDE_DIR}>)

# ------------------------------------------------------------------------------
# Documentation
# ------------------------------------------------------------------------------

append_to_global_list(DOCUMENTATION_FILES ${${LIBRARY_NAME}_HEADERS})

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

tweak_component_install(${LIBRARY_NAME})

# ------------------------------------------------------------------------------
# Automatic tests
# ------------------------------------------------------------------------------

if(BUILD_TESTS)
  add_subdirectory(test/test-tweakmodel)
  add_subdirectory(test/test-uri-to-index)
  add_subdirectory(test/test-queue)
  add_subdirectory(test/test-app)
endif()
