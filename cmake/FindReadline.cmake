# ##############################################################################
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
# ##############################################################################

find_path(Readline_INCLUDE_DIR NAMES readline/readline.h)

find_library(Readline_LIBRARY NAMES readline)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Readline DEFAULT_MSG Readline_INCLUDE_DIR
                                  Readline_LIBRARY)

mark_as_advanced(Readline_INCLUDE_DIR Readline_LIBRARY)

if(Readline_FOUND AND NOT TARGET Readline)
  # Create an imported target if Readline was found
  add_library(Readline UNKNOWN IMPORTED)

  if(${CMAKE_VERSION} VERSION_LESS "3.16.0")
    # Long version of the call is required for CMake < 3.16, see
    # https://gitlab.kitware.com/cmake/cmake/-/issues/19434
    set_target_properties(Readline PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                              "${Readline_INCLUDE_DIR}")
  else()
    target_include_directories(Readline INTERFACE "${Readline_INCLUDE_DIR}")
  endif()

  set_property(TARGET Readline PROPERTY IMPORTED_LOCATION "${Readline_LIBRARY}")
endif()
