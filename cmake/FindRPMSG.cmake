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

#
# Finds the TI TDA4 RPMSG framework includes and libraries
#
#  RPMSG_FOUND     - True if RPMSG framework was found
#  RPMSG::RPMSG    - RPMSG imported target
#

find_path(RPMSG_INCLUDE_DIR NAMES ti_rpmsg_char.h)

find_library(RPMSG_LIBRARY ti_rpmsg_char)

set(RPMSG_INCLUDE_DIRS
  ${RPMSG_INCLUDE_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("RPMSG" DEFAULT_MSG RPMSG_INCLUDE_DIRS RPMSG_LIBRARY)

if(RPMSG_FOUND)
  if (NOT TARGET RPMSG::RPMSG)
    add_library(RPMSG::RPMSG UNKNOWN IMPORTED)

    set_target_properties(RPMSG::RPMSG PROPERTIES
      IMPORTED_LOCATION "${RPMSG_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${RPMSG_INCLUDE_DIRS}"
    )
  endif()
endif()

mark_as_advanced(RPMSG_INCLUDE_DIRS RPMSG_LIBRARY)
