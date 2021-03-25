# ##############################################################################
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
# Create project
# ------------------------------------------------------------------------------
project(use-case-tweak-1 LANGUAGES C)

# ------------------------------------------------------------------------------
# Global project configuration
# ------------------------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
find_package(tweak REQUIRED)

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(${PROJECT_NAME}_SOURCES main.c)

# ------------------------------------------------------------------------------
# Binary
# ------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME} tweak-client m)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 99
                                                 C_STANDARD_REQUIRED YES)
