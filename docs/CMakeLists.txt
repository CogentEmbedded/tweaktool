###############################################################################
#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2018-2020 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
#
# The source code contained or described herein and all documents related to the
# source code("Software") or their modified versions are owned by
# Cogent Embedded, Inc. or its affiliates.
#
# No part of the Software may be used, copied, reproduced, modified, published,
# uploaded, posted, transmitted, distributed, or disclosed in any way without
# prior express written permission from Cogent Embedded, Inc.
#
# Cogent Embedded, Inc. grants a nonexclusive, non-transferable, royalty-free
# license to use the Software to Licensee without the right to sublicense.
# Licensee agrees not to distribute the Software to any third-party without
# the prior written permission of Cogent Embedded, Inc.
#
# Unless otherwise agreed by Cogent Embedded, Inc. in writing, you may not remove
# or alter this notice or any other notice embedded in Software in any way.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
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
