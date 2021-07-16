# ##############################################################################
#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
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

set(BINARY_NAME tweakguitest)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

find_package(Qt5 REQUIRED COMPONENTS QuickTest)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

add_definitions(-DQUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
set(${BINARY_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/tst_ConnectionsPage.qml)

# ------------------------------------------------------------------------------
# Binary generation
# ------------------------------------------------------------------------------

add_executable(${BINARY_NAME} ${${BINARY_NAME}_SOURCES})

target_link_libraries(${BINARY_NAME} Qt5::QuickTest ${GUI_TOOL_NAME}-lib)

set_target_properties(${BINARY_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

target_compile_features(${BINARY_NAME} PUBLIC cxx_std_11)

target_include_directories(${BINARY_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src)

# ------------------------------------------------------------------------------
# Automatic tests
# ------------------------------------------------------------------------------

add_test(NAME ${BINARY_NAME} COMMAND ${BINARY_NAME})
