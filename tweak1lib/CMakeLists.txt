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

set(LIBRARY_NAME compatibility-server)
set(LIBRARY_SOVERSION 0)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_DEPENDENCIES ${PROJECT_NAMESPACE}::common
                                 ${PROJECT_NAMESPACE}::app)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_HEADERS)
set(${LIBRARY_NAME}_COMPAT_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tweakconfig.h)

set(${LIBRARY_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakcompat.c)

# ------------------------------------------------------------------------------
# Library generation
# ------------------------------------------------------------------------------

add_library(
  ${LIBRARY_NAME} ${${LIBRARY_NAME}_SOURCES} ${${LIBRARY_NAME}_HEADERS}
                  ${${LIBRARY_NAME}_COMPAT_HEADERS})

if (MSVC)
  target_compile_options(${LIBRARY_NAME} PRIVATE /W4 /WX)
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakcompat.c
    PROPERTIES
    COMPILE_FLAGS ${VC_UTHASH_WARNINGS})
endif()

add_library(${PROJECT_NAMESPACE}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} PRIVATE ${${LIBRARY_NAME}_DEPENDENCIES})

set_target_properties(
  ${LIBRARY_NAME}
  PROPERTIES SOVERSION ${LIBRARY_SOVERSION} SONAME ${LIBRARY_NAME}
             OUTPUT_NAME ${PROJECT_NAME}${LIBRARY_NAME}
             VERSION "${PROJECT_NUMERIC_VERSION}")

target_compile_features(${LIBRARY_NAME} PUBLIC c_std_99)

target_include_directories(
  ${LIBRARY_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE $<BUILD_INTERFACE:${UTHASH_INCLUDE_DIR}>)

# ------------------------------------------------------------------------------
# Documentation
# ------------------------------------------------------------------------------

append_to_global_list(DOCUMENTATION_FILES ${${LIBRARY_NAME}_HEADERS})

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

tweak_component_install(${LIBRARY_NAME})

# Manually install old tweak.h into /usr/include or similar path
install(
  FILES ${${LIBRARY_NAME}_COMPAT_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT dev)

# ------------------------------------------------------------------------------
# Automatic tests
# ------------------------------------------------------------------------------

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
