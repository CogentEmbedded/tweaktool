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
# Dependencies
# ------------------------------------------------------------------------------

find_package(Readline REQUIRED)

# ------------------------------------------------------------------------------
# Common settings
# ------------------------------------------------------------------------------
set(BINARY_NAME tweak-app-cl)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(${BINARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stringutil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stringutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakuriutil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweakuriutil.h)

# ------------------------------------------------------------------------------
# Binary
# ------------------------------------------------------------------------------

add_executable(${BINARY_NAME} ${${BINARY_NAME}_SOURCES})

target_link_libraries(${BINARY_NAME} ${PROJECT_NAMESPACE}::app Readline)

target_compile_options(${BINARY_NAME} PRIVATE -Wall -Wextra -Werror)

target_compile_features(${BINARY_NAME} PUBLIC c_std_99)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

tweak_binary_install(${BINARY_NAME})

# ------------------------------------------------------------------------------
# Automatic tests
# ------------------------------------------------------------------------------

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
