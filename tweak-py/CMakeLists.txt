#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2018-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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
set(LIBRARY_NAME pytweaklib)
set(python_module_name tweak2)

find_package(Python 3 REQUIRED COMPONENTS Interpreter Development)

# Fix linking with debug python libraries on Windows builds
if (MSVC AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(Py_DEBUG)
endif()

if(CMAKE_CROSSCOMPILING)
  # SDK package must be used to generate correct library suffix e.g. aarch64
  # Bundled pybind would only generate host suffix e.g. x86-64
  find_package(pybind11 REQUIRED)
else()
  set(PYBIND11_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../extern/pybind11")
  if(NOT EXISTS "${PYBIND11_DIR}/CMakeLists.txt")
    message(
      FATAL_ERROR
        "pybind11: package was not found."
        " Please init and update all submodules." " Searched ${PYBIND11_DIR}")
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

if (MSVC)
    # Visual Studio is pedantic about designated initializers which are in standard since C++ 20
    target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_20)
else()
    target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_14)
endif()

configure_file(__init__.py.in __init__.py)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

if(BUILD_TESTS)
  add_test(
    NAME ${PROJECT_NAME}-python-test
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}
            ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------
if (NOT MSVC)
    set(PYTHON_SITE_DIR "${Python_SITEARCH}")
    if (CMAKE_CROSSCOMPILING)
      string(REPLACE "$ENV{OECORE_NATIVE_SYSROOT}/usr/" "" PYTHON_SITE_DIR ${PYTHON_SITE_DIR})
    endif()
    message(STATUS "Python site packages dir: ${PYTHON_SITE_DIR}")

    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
      DESTINATION ${PYTHON_SITE_DIR}/${python_module_name}
      COMPONENT python)

    install(
      TARGETS ${LIBRARY_NAME}
      DESTINATION ${PYTHON_SITE_DIR}/${python_module_name}
      COMPONENT python)
endif()
