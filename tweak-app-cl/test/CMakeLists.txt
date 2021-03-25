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
# Dependencies
# ------------------------------------------------------------------------------

find_program(EXPECT_BINARY expect)
if(NOT EXPECT_BINARY)
  message(
    FATAL_ERROR
      "expect is required for ${BINARY_NAME} tests.\n"
      "Please disable tests or install expect with 'sudo aapt-get install expect'."
  )
endif()

# ------------------------------------------------------------------------------
# Automatic tests
# ------------------------------------------------------------------------------

function(tweak_app_cl_test_case CASE)
  add_test(
    NAME ${BINARY_NAME}-${CASE}
    COMMAND ${EXPECT_BINARY} -f ${CASE}.exp "$<TARGET_FILE:${BINARY_NAME}>"
            "$<TARGET_FILE:tweak-mock-server>"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

tweak_app_cl_test_case(help)
tweak_app_cl_test_case(disconnected)
tweak_app_cl_test_case(list)
tweak_app_cl_test_case(reconnect)
