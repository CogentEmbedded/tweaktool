# ##############################################################################
#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2020 Cogent Embedded Inc. ALL RIGHTS RESERVED.
#
# The source code contained or described herein and all documents related to the
# source code("Software") or their modified versions are owned by Cogent
# Embedded Inc. or its affiliates.
#
# No part of the Software may be used, copied, reproduced, modified, published,
# uploaded, posted, transmitted, distributed, or disclosed in any way without
# prior express written permission from Cogent Embedded Inc.
#
# Cogent Embedded Inc. grants a nonexclusive, non-transferable, royalty-free
# license to use the Software to Licensee without the right to sublicense.
# Licensee agrees not to distribute the Software to any third-party without the
# prior written permission of Cogent Embedded Inc.
#
# Unless otherwise agreed by Cogent Embedded Inc. in writing, you may not remove
# or alter this notice or any other notice embedded in Software in any way.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# ##############################################################################

# CMake older than than 3.10 does not support find_dependency with PATHS
# argument, see https://github.com/ros-industrial/abb_librws/issues/84
if(${CMAKE_VERSION} VERSION_LESS "3.10.0")
  set(FIND_COMPONENT_COMMAND find_package)
else()
  set(FIND_COMPONENT_COMMAND find_dependency)
endif()

# Installs a tweak component using standard component rules
macro(tweak_component_install TARGET_NAME)

  install(
    TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}
    # CMake less than 3.10 require explicit specification of installation paths
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT tools)

  install(
    FILES ${${TARGET_NAME}_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAMESPACE}
    COMPONENT dev)
  target_include_directories(
    ${TARGET_NAME}
    INTERFACE
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

  set(COMPONENT_DEPENDENCIES ${${TARGET_NAME}_DEPENDENCIES})
  install(
    EXPORT ${TARGET_NAME}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAMESPACE}
    FILE ${PROJECT_NAMESPACE}${TARGET_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAMESPACE}::
    COMPONENT dev)

  configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/componentConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAMESPACE}${TARGET_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAMESPACE})

  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAMESPACE}${TARGET_NAME}Version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

  install(
    FILES
      "${PROJECT_BINARY_DIR}/${PROJECT_NAMESPACE}${TARGET_NAME}Version.cmake"
      "${PROJECT_BINARY_DIR}/${PROJECT_NAMESPACE}${TARGET_NAME}Config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAMESPACE}
    COMPONENT dev)

  unset(COMPONENT_DEPENDENCIES)

endmacro()

# Installs tweak executable binary using standard rules
macro(tweak_binary_install TARGET_NAME)

  install(TARGETS ${TARGET_NAME} # CMake less than 3.10 require explicit
                                 # specification of installation paths
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT tools)

endmacro()
