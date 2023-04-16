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

set(LIBRARY_NAME wire)
set(LIBRARY_SOVERSION 1)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

include(CheckIncludeFiles)

if (WITH_WIRE_NNG AND NOT TARGET nng::nng)
  find_package(nng REQUIRED)

  list(APPEND CMAKE_REQUIRED_INCLUDES "${NNG_INCLUDE_DIRS}")
  CHECK_INCLUDE_FILES("nng.h;protocol/hub0/hub.h" HAVE_NNG_HUB_PROTO)
  if (NOT HAVE_NNG_HUB_PROTO)
    message(FATAL_ERROR "Tweaktool required NNG with supported HUB protocol")
  endif()
endif()

set(${LIBRARY_NAME}_DEPENDENCIES ${PROJECT_NAMESPACE}::common)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire.c)

if(WITH_WIRE_NNG)
  list(APPEND ${LIBRARY_NAME}_SOURCES
       ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_nng.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_nng.h)
  list(APPEND ${LIBRARY_NAME}_DEPENDENCIES nng)
endif()

if(WIRE_RPMSG_BACKEND STREQUAL TI_API)
  set(WITH_WIRE_RPMSG ON)
  list(
    APPEND
    ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg_transport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg_transport_ti_api.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg.c)

  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Concerto")
    find_package(TIOVX REQUIRED)
  endif()

elseif(WIRE_RPMSG_BACKEND STREQUAL CHRDEV)
  set(WITH_WIRE_RPMSG ON)
  find_package(RPMSG REQUIRED)
  list(APPEND ${LIBRARY_NAME}_DEPENDENCIES RPMSG)
  list(
    APPEND
    ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg_transport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg_transport_linux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg.c)
elseif(WIRE_RPMSG_BACKEND STREQUAL OFF)
else()
  message(
    FATAL_ERROR
      "Unsupported value of WIRE_RPMSG_BACKEND: ${WIRE_RPMSG_BACKEND}. "
      "Supported values are: ${WIRE_RPMSG_BACKENDS}")
endif()

set(${LIBRARY_NAME}_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/wire.h)

# ------------------------------------------------------------------------------
# Library generation
# ------------------------------------------------------------------------------

add_library(${LIBRARY_NAME} ${${LIBRARY_NAME}_SOURCES}
                            ${${LIBRARY_NAME}_HEADERS})

if (MSVC)
  target_compile_options(${LIBRARY_NAME} PRIVATE /W4 /WX)
endif()

add_library(${PROJECT_NAMESPACE}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(
  ${LIBRARY_NAME} PRIVATE
    $<$<BOOL:${WITH_WIRE_NNG}>:WITH_WIRE_NNG>
    $<$<BOOL:${WITH_WIRE_RPMSG}>:WITH_WIRE_RPMSG>
    WIRE_RPMSG_BACKEND_${WIRE_RPMSG_BACKEND})

target_link_libraries(
  ${LIBRARY_NAME}
  PUBLIC ${PROJECT_NAMESPACE}::common
  PRIVATE
    $<$<BOOL:${WITH_WIRE_NNG}>:nng::nng>
    $<$<BOOL:${RPMSG_FOUND}>:RPMSG::RPMSG>
    $<$<BOOL:${TIOVX_FOUND}>:TIOVX::TIOVX>)

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
