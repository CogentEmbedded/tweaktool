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

set(BINARY_NAME tweakmetadata-test)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

find_package(Qt5Test REQUIRED)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(${BINARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TweakMetadata.test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/TweakMetadataParser.cpp)

# ------------------------------------------------------------------------------
# Binary generation
# ------------------------------------------------------------------------------

add_executable(${BINARY_NAME} ${${BINARY_NAME}_SOURCES})

if (MSVC)
  target_compile_options(${BINARY_NAME} PRIVATE /W4 /WX)
endif()

target_link_libraries(${BINARY_NAME} Qt5::Core Qt5::Test
                      ${PROJECT_NAMESPACE}::app ${PROJECT_NAMESPACE}::metadata)

set_target_properties(${BINARY_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

target_compile_features(${BINARY_NAME} PUBLIC cxx_std_14)

target_include_directories(${BINARY_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src)

# ------------------------------------------------------------------------------
# Automatic tests
# ------------------------------------------------------------------------------

add_test(NAME ${BINARY_NAME} COMMAND ${BINARY_NAME})
