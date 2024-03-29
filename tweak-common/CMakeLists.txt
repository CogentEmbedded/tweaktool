#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2018-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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
#
# ------------------------------------------------------------------------------
# Common settings
# ------------------------------------------------------------------------------

set(LIBRARY_NAME common)
set(LIBRARY_SOVERSION 2)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_DEPENDENCIES ${PROJECT_NAMESPACE}::json)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/variant.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/thread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/defaults.h)

set(${LIBRARY_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakbuffer.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakstring.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakvariant.c)


if(CMAKE_SYSTEM_NAME STREQUAL "Concerto")
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_format_time_tda4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_out_tda4.c)
else()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  list(APPEND ${LIBRARY_NAME}_DEPENDENCIES Threads)

  if (NOT MSVC)
    include(CheckSymbolExists)
    check_symbol_exists(pthread_threadid_np "pthread.h" PTHREAD_THREADID_NP_EXISTS)
  endif()

  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_out_stderr.c)

  if(MSVC)
    list(APPEND ${LIBRARY_NAME}_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_format_time_winapi.c)
  else()
    list(APPEND ${LIBRARY_NAME}_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_format_time_posix.c)
  endif()
endif()

if(PTHREAD_THREADID_NP_EXISTS)
  # Prefer pthread implementation over syscall() for Linux
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_pthread_np.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_linux_syscall.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Concerto")
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_sysbios.c)
elseif(MSVC)
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_winapi.c)
else()
  list(APPEND ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweaklog_thread_id_fallback.c)
endif()

if (MSVC)
  list(APPEND ${LIBRARY_NAME}_SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/src/tweak_id_gen_winapi.c)
else()
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
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Concerto")
    list(APPEND ${LIBRARY_NAME}_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/src/tweak_id_gen_sysbios.c)
  else()
    list(APPEND ${LIBRARY_NAME}_SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/src/tweak_id_gen_fallback.c)
  endif()
endif()

# ------------------------------------------------------------------------------
# Library generation
# ------------------------------------------------------------------------------

add_library(${LIBRARY_NAME} ${${LIBRARY_NAME}_SOURCES}
                            ${${LIBRARY_NAME}_HEADERS})

if (MSVC)
  target_compile_options(${LIBRARY_NAME} PRIVATE /W4 /WX)
endif()

add_library(${PROJECT_NAMESPACE}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

if(NOT MSVC)
  target_link_libraries(${LIBRARY_NAME} PRIVATE m)
endif()

target_link_libraries(${LIBRARY_NAME} PUBLIC $<$<BOOL:${Threads_FOUND}>:Threads::Threads>
  PRIVATE ${PROJECT_NAMESPACE}::json)

target_compile_features(${LIBRARY_NAME} PUBLIC c_std_99)

set_target_properties(
  ${LIBRARY_NAME}
  PROPERTIES SOVERSION ${LIBRARY_SOVERSION} SONAME ${LIBRARY_NAME}
             OUTPUT_NAME ${PROJECT_NAME}${LIBRARY_NAME}
             VERSION "${PROJECT_NUMERIC_VERSION}")

target_include_directories(
  ${LIBRARY_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

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
  add_subdirectory(test/test-string)
  add_subdirectory(test/test-variant)
endif()
