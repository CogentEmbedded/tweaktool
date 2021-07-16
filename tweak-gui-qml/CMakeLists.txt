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

# ------------------------------------------------------------------------------
# Common settings
# ------------------------------------------------------------------------------

set(LIBRARY_NAME ${GUI_TOOL_NAME}-lib)
set(BINARY_NAME ${GUI_TOOL_NAME})

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

find_package(Qt5 REQUIRED COMPONENTS Core Quick QuickControls2)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(${LIBRARY_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QTweakVariant.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakComponents.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakComponents.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakMetadataParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakMetadataParser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakQmlApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakQmlApp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakMetadataParser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakMetadataParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakQmlApp_p.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakTreeModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakTreeModel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakUriTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TweakUriTree.hpp)

set(${BINARY_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

qt5_add_resources(${LIBRARY_NAME}_RESOURCES
                  ${CMAKE_CURRENT_SOURCE_DIR}/resources/main.qrc)

# ------------------------------------------------------------------------------
# Library
# ------------------------------------------------------------------------------
add_library(${LIBRARY_NAME} STATIC ${${LIBRARY_NAME}_SOURCES}
                                   ${${LIBRARY_NAME}_RESOURCES})

target_link_libraries(
  ${LIBRARY_NAME}
  Qt5::Core
  Qt5::Quick
  Qt5::QuickControls2
  ${PROJECT_NAMESPACE}::app
  ${PROJECT_NAMESPACE}::metadata
  SortFilterProxyModel)

set_target_properties(${LIBRARY_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_11)

# ------------------------------------------------------------------------------
# Binary
# ------------------------------------------------------------------------------
add_executable(${BINARY_NAME} ${${BINARY_NAME}_SOURCES})

target_link_libraries(${BINARY_NAME} ${LIBRARY_NAME})

set_target_properties(${BINARY_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

target_compile_features(${BINARY_NAME} PUBLIC cxx_std_11)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

tweak_binary_install(${BINARY_NAME})

# ------------------------------------------------------------------------------
# Automatic tests
# ------------------------------------------------------------------------------

if(BUILD_TESTS)
  add_subdirectory(test/qtweakvariant)
  add_subdirectory(test/tweaknodetree)
  add_subdirectory(test/tweakqmlapp)
  add_subdirectory(test/tweakmetadata)
  if(QT_VERSION VERSION_GREATER_EQUAL 5.11.0)
    add_subdirectory(test/tweakguitest)
  endif()
endif()
