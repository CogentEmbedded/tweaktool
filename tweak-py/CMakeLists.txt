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
cmake_minimum_required(VERSION 3.10)

set(LIBRARY_NAME pytweaklib)
set(python_module_name tweak2)

find_package(PythonInterp 3 REQUIRED)
find_package(pybind11 QUIET CONFIG)

if(pybind11_FOUND)
  message(STATUS "pybind11: Using system package version ${pybind11_VERSION}")
else()
  set(PYBIND11_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/pybind11")
  if(NOT EXISTS "${PYBIND11_DIR}/CMakeLists.txt")
    message(
      FATAL_ERROR
        "pybind11: System package was not found, built-in in also unavailable."
        " Please init and update all submodules."
        " Searched ${PYBIND11_DIR}")
  endif()

  add_subdirectory("${PYBIND11_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/pybind11)
endif()

set(${PROJECT_NAME}_PYTHON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/py_tweak.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/py_tweak_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/py_tweak_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/py_tweak_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gil_loop.cpp)
pybind11_add_module(${LIBRARY_NAME} THIN_LTO ${${PROJECT_NAME}_PYTHON_SOURCES})

target_link_libraries(${LIBRARY_NAME} PRIVATE ${PROJECT_NAMESPACE}::metadata
  ${PROJECT_NAMESPACE}::app ${PROJECT_NAMESPACE}::server)

set_target_properties(${LIBRARY_NAME} PROPERTIES LIBRARY_OUTPUT_NAME
                                                 ${python_module_name})
target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_11)

configure_file(__init__.py.in __init__.py)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

if(BUILD_TESTS)
  add_test(
    NAME ${PROJECT_NAME}-python-test
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}
            ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------

execute_process(
  COMMAND
    ${PYTHON_EXECUTABLE} -c "
from distutils import sysconfig
print(sysconfig.get_python_lib(prefix=''))
  "
  OUTPUT_VARIABLE PYTHON_SITE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if (MSVC)
    cmake_path(SET PYTHON_SITE_DIR NORMALIZE ${PYTHON_SITE_DIR})
endif()

message(STATUS "Python site packages dir: ${PYTHON_SITE_DIR}")

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${PYTHON_SITE_DIR}/${python_module_name}
  COMPONENT python)

install(
  TARGETS ${LIBRARY_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${PYTHON_SITE_DIR}/${python_module_name}
  COMPONENT python)
