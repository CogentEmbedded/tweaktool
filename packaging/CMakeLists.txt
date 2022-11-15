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

set(INSTALL_COMPONENTS dev tools python)

if(APPLE)
  set(GEN_PACKAGES DragNDrop)
elseif(MSVC)
  set(GEN_PACKAGES NSIS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Concerto")
  set(GEN_PACKAGES TGZ)
  set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
  set(INSTALL_COMPONENTS dev)
elseif(CMAKE_CROSSCOMPILING)
  set(GEN_PACKAGES TGZ)
else()
  set(GEN_PACKAGES DEB)
  find_program(RPMBUILD rpmbuild)

  if (RPMBUILD)
    list(APPEND GEN_PACKAGES RPM)
  endif()
endif()

set(CPACK_GENERATOR ${GEN_PACKAGES})

set(CPACK_COMPONENTS_ALL ${INSTALL_COMPONENTS})
set(CPACK_COMPONENTS_GROUPING IGNORE)

set(CPACK_PACKAGE_NAME tweaktool)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_SUMMARY})
set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION_STRING})

set(CPACK_STRIP_FILES TRUE)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_CURRENT_LIST_DIR}/generators.cmake)

if(BUILD_GUI
   AND NOT MSVC
   AND NOT APPLE)

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
