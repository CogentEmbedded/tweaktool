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

option(PACKAGE_RPM "Generate RPM package" ON)
option(PACKAGE_DEB "Generate DEB package" ON)

if(PACKAGE_DEB)
  list(APPEND GEN_PACKAGES DEB)
  set(CPACK_DEB_COMPONENT_INSTALL ON)
endif()

if(PACKAGE_RPM)
  list(APPEND GEN_PACKAGES RPM)
  set(CPACK_RPM_COMPONENT_INSTALL ON)
endif()

set(CPACK_GENERATOR ${GEN_PACKAGES})

set(CPACK_PACKAGE_NAME tweaktool)

set(CPACK_COMPONENTS_ALL dev tools)
set(CPACK_COMPONENTS_GROUPING IGNORE)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_SUMMARY})
set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://cogentembedded.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER
    "Sergey Zykov <sergey.zykov@cogentembedded.com>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION_STRING})
set(CPACK_PACKAGE_INSTALL_DIRECTORY
    "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")

set(CPACK_STRIP_FILES TRUE)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY ">=")

if(BUILD_GUI)
  set(ICON_NAME tweak-icon)

  # Install application icons
  # https://developer.gnome.org/integration-guide/stable/icons.html.en
  install(
    FILES icons/128/${ICON_NAME}.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps
    COMPONENT tools)
  install(
    FILES icons/256/${ICON_NAME}.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
    COMPONENT tools)
  install(
    FILES icons/512/${ICON_NAME}.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps
    COMPONENT tools)

  # Configure desktop file
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tweak.desktop.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${GUI_TOOL_NAME}.desktop @ONLY)

  # Install desktop file
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${GUI_TOOL_NAME}.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    COMPONENT tools)
endif()

include(CPack)
