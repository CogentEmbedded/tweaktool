#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

  foreach(_dependency ${COMPONENT_DEPENDENCIES})
    set(DEPENDENCY_LOOKUP_MODULE
        "${PROJECT_SOURCE_DIR}/cmake/Find${_dependency}.cmake")
    if(EXISTS ${DEPENDENCY_LOOKUP_MODULE})
      install(
        FILES ${DEPENDENCY_LOOKUP_MODULE}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAMESPACE}
        COMPONENT dev)
    endif()
  endforeach()

  unset(COMPONENT_DEPENDENCIES)

endmacro()

# Installs tweak executable binary using standard rules
macro(tweak_binary_install TARGET_NAME)

  install(
    TARGETS ${TARGET_NAME} # CMake less than 3.10 require explicit specification
                           # of installation paths
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT tools
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT tools)

endmacro()

# Installs service files in appropriate systemd folder
macro(tweak_service_install FILE_NAMES)

  install(
    FILES ${FILE_NAMES}
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/../lib/systemd/system
    COMPONENT tools)

endmacro()
