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

macro(append_to_global_list LIST_NAME)
  set(LIST_IN ${ARGN})
  foreach(F IN LISTS LIST_IN)
    if(IS_ABSOLUTE ${F})
      list(APPEND ${LIST_NAME} ${F})
    else()
      list(APPEND ${LIST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${F})
    endif()
  endforeach()
  get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
  if(HAS_PARENT_SCOPE)
    set(${LIST_NAME}
        ${${LIST_NAME}}
        PARENT_SCOPE)
  endif()
endmacro()
