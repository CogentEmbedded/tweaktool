#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2018-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

# ------------------------------------------------------------------------------
# Extract current version tag from git and set version variables
# ------------------------------------------------------------------------------
execute_process(
  COMMAND git describe --tags --always
  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  OUTPUT_VARIABLE GIT_HEAD_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

if(GIT_HEAD_TAG)
  string(REGEX MATCH "[0-9]+\\.[0-9]+(\\.[0-9]+(-[0-9]+)?)?"
               PROJECT_NUMERIC_VERSION "${GIT_HEAD_TAG}")
  string(REPLACE "-" "." PROJECT_NUMERIC_VERSION "${PROJECT_NUMERIC_VERSION}")
  set(PROJECT_VERSION_STRING "${GIT_HEAD_TAG}")
else()
  set(PROJECT_VERSION_STRING "${PROJECT_NUMERIC_VERSION}-git")
endif()

# ------------------------------------------------------------------------------
# Generate header file if needed
# ------------------------------------------------------------------------------
if(HEADER_SRC AND HEADER_DST)
  configure_file(${HEADER_SRC} ${HEADER_DST} @ONLY)
endif()
