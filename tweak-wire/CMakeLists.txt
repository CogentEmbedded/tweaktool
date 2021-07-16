# ##############################################################################
#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2020 Cogent Embedded Inc. ALL RIGHTS RESERVED.
#
# The source code contained or described herein and all documents related to the
# source code("Software") or their modified versions are owned by Cogent
# Embedded Inc. or its affiliates.
#
# No part of the Software may be used, copied, reproduced, modified, published,
# uploaded, posted, transmitted, distributed, or disclosed in any way without
# prior express written permission from Cogent Embedded Inc.
#
# Cogent Embedded Inc. grants a nonexclusive, non-transferable, royalty-free
# license to use the Software to Licensee without the right to sublicense.
# Licensee agrees not to distribute the Software to any third-party without the
# prior written permission of Cogent Embedded Inc.
#
# Unless otherwise agreed by Cogent Embedded Inc. in writing, you may not remove
# or alter this notice or any other notice embedded in Software in any way.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# ##############################################################################

# ------------------------------------------------------------------------------
# Common settings
# ------------------------------------------------------------------------------

set(LIBRARY_NAME wire)
set(LIBRARY_SOVERSION 0)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

if(WITH_WIRE_NNG)
  find_package(nng REQUIRED)
endif()

set(${LIBRARY_NAME}_DEPENDENCIES ${PROJECT_NAMESPACE}::common Threads)

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

if(WITH_WIRE_RPMSG)
  list(
    APPEND
    ${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg_transport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg_transport_linux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakwire_rpmsg.c)
endif()

set(${LIBRARY_NAME}_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/tweak2/wire.h)

# ------------------------------------------------------------------------------
# Library generation
# ------------------------------------------------------------------------------

add_library(${LIBRARY_NAME} ${${LIBRARY_NAME}_SOURCES}
                            ${${LIBRARY_NAME}_HEADERS})

add_library(${PROJECT_NAMESPACE}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(
  ${LIBRARY_NAME}
  PRIVATE $<$<BOOL:${WITH_WIRE_NNG}>:WITH_WIRE_NNG>
  PRIVATE $<$<BOOL:${WITH_WIRE_RPMSG}>:WITH_WIRE_RPMSG>)

target_link_libraries(
  ${LIBRARY_NAME}
  PUBLIC ${PROJECT_NAMESPACE}::common
  PRIVATE Threads::Threads $<$<BOOL:${WITH_WIRE_NNG}>:nng::nng>)

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
