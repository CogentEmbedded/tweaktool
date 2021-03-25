###############################################################################
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
# #############################################################################

function(generate_doxygen_docs)
    cmake_parse_arguments(GDD "" "TITLE" "FILES" ${ARGN})

    string(REPLACE ";" " " GDD_FILES "${GDD_FILES}")
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs)
    file(MAKE_DIRECTORY ${DOXYGEN_OUT})
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)

    set(DOXYGEN_INPUT ${GDD_FILES})
    set(DOXYGEN_OUTPUT_DIRECTORY ${DOXYGEN_OUT})
    set(DOXYGEN_TITLE ${GDD_TITLE})
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT}/Doxyfile @ONLY)

    add_custom_target(
        docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}/Doxyfile
        WORKING_DIRECTORY ${DOXYGEN_OUT}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM)

endfunction()

if(WITH_DOXYGEN)
    find_package(Doxygen REQUIRED)

    generate_doxygen_docs(
        TITLE "Tweak Tool"
        FILES ${DOCUMENTATION_FILES})
endif(WITH_DOXYGEN)
