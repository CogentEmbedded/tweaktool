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

set(LIBRARY_NAME common)
set(LIBRARY_SOVERSION 0)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_DEPENDENCIES)

set(CMAKE_REQUIRED_LIBRARIES Threads::Threads)
include(CheckSymbolExists)
check_symbol_exists(pthread_threadid_np "pthread.h" PTHREAD_THREADID_NP_EXISTS)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/variant.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/log.h)

set(${LIBRARY_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_out_stderr.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_unix.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakstring.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakvariant.c)

if(PTHREAD_THREADID_NP_EXISTS)
  # Prefer pthread implementation over syscall() for Linux
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_pthread_np.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_linux_syscall.c)
else()
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_fallback.c)
endif()

include(CheckCSourceCompiles)

check_c_source_compiles(
"int main() {
  long long value = 0UL;
  value = __sync_fetch_and_add(&value, 1UL);
  return 0;
}" SYNC_FETCH_AND_ADD_EXISTS)

check_c_source_compiles(
"#include <atomic.h>

static volatile unsigned seed = 1UL;

int main() {
  unsigned value;
  value = atomic_add_value(&seed, 1);
  return 0;
}" ATOMIC_ADD_VALUE_EXISTS)

if(SYNC_FETCH_AND_ADD_EXISTS)
  list(APPEND ${LIBRARY_NAME}_SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/src/tweak_id_gen_sync_fetch_and_add.c)
elseif(ATOMIC_ADD_VALUE_EXISTS)
  list(APPEND ${LIBRARY_NAME}_SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/src/tweak_id_gen_atomic_add_value.c)
else()
  list(APPEND ${LIBRARY_NAME}_SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/src/tweak_id_gen_fallback.c)
endif()

# ------------------------------------------------------------------------------
# Library generation
# ------------------------------------------------------------------------------

add_library(${LIBRARY_NAME} ${${LIBRARY_NAME}_SOURCES}
                            ${${LIBRARY_NAME}_HEADERS})

add_library(${PROJECT_NAMESPACE}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} PRIVATE m Threads::Threads)

target_compile_features(${LIBRARY_NAME} PUBLIC c_std_99)

set_target_properties(
  ${LIBRARY_NAME}
  PROPERTIES SOVERSION ${LIBRARY_SOVERSION} SONAME ${LIBRARY_NAME}
             OUTPUT_NAME ${PROJECT_NAME}${LIBRARY_NAME}
             VERSION "${PROJECT_NUMERIC_VERSION}")

# INSTALL_INTERFACE header path is specified during installation
target_include_directories(
  ${LIBRARY_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

list(FIND TWEAK_COMMON_LOG_LEVELS ${TWEAK_COMMON_LOG_LEVEL} TWEAK_COMMON_LOG_LEVEL_INDEX)
if (TWEAK_COMMON_LOG_LEVEL_INDEX LESS 0 OR
    TWEAK_COMMON_LOG_LEVEL_INDEX GREATER ${TWEAK_COMMON_LOG_LEVELS_LENGTH})
    message(FATAL_ERROR "TWEAK_COMMON_LOG_LEVEL must be one of ${TWEAK_COMMON_LOG_LEVELS}")
endif()

target_compile_definitions(${LIBRARY_NAME} PUBLIC -DTWEAK_LOG_LEVEL=${TWEAK_COMMON_LOG_LEVEL_INDEX})

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
  add_subdirectory(test)
endif()
