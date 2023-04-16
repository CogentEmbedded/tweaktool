#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2022-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

find_package(TIOVX REQUIRED)

add_executable(tweak-gw ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

target_link_libraries(tweak-gw PRIVATE ${PROJECT_NAMESPACE}::app TIOVX::TIOVX)

tweak_binary_install(tweak-gw)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tweak-gw.service
               ${CMAKE_CURRENT_BINARY_DIR}/tweak-gw.service @ONLY)
tweak_service_install(${CMAKE_CURRENT_BINARY_DIR}/tweak-gw.service)
