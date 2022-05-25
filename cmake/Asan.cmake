#
# The MIT License (MIT)
#
# * Copyright (c) 2013 Matthew Arsenault.
# * Copyright (c) 2021-2022 Cogent Embedded, Inc.
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
# This module tests if address sanitizer is supported by the compiler, and
# creates a Asan build type (i.e. set CMAKE_BUILD_TYPE=Asan to use it). It sets
# the following variables:
#
# * CMAKE_C_FLAGS_ASAN - Flags to use for C with asan
# * CMAKE_CXX_FLAGS_ASAN - Flags to use for C++ with asan
# * CMAKE_EXE_LINKER_FLAGS_ASAN,
# * CMAKE_SHARED_LINKER_FLAGS_ASAN - Additional link flags required for asan
#   builds
# * HAVE_ADDRESS_SANITIZER - True or false if the Asan build type is available

include(CheckCCompilerFlag)
# Set -Werror to catch "argument unused during compilation" warnings
set(CMAKE_REQUIRED_FLAGS "-Werror -faddress-sanitizer") # Also needs to be a
                                                        # link flag for test to
                                                        # pass
check_c_compiler_flag("-faddress-sanitizer" HAVE_C_FLAG_ADDRESS_SANITIZER)

set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=address") # Also needs to be a link
                                                       # flag for test to pass
check_c_compiler_flag("-fsanitize=address" HAVE_C_FLAG_SANITIZE_ADDRESS)

include(CheckCXXCompilerFlag)
# Set -Werror to catch "argument unused during compilation" warnings
set(CMAKE_REQUIRED_FLAGS "-Werror -faddress-sanitizer") # Also needs to be a
                                                        # link flag for test to
                                                        # pass
check_cxx_compiler_flag("-faddress-sanitizer" HAVE_CXX_FLAG_ADDRESS_SANITIZER)

set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=address") # Also needs to be a link
                                                       # flag for test to pass
check_cxx_compiler_flag("-fsanitize=address" HAVE_CXX_FLAG_SANITIZE_ADDRESS)

unset(CMAKE_REQUIRED_FLAGS)

set(HAVE_ADDRESS_SANITIZER FALSE)
if(HAVE_C_FLAG_SANITIZE_ADDRESS AND HAVE_CXX_FLAG_SANITIZE_ADDRESS)
  # Clang 3.2+ use this version
  set(HAVE_ADDRESS_SANITIZER TRUE)
  set(ADDRESS_SANITIZER_C_FLAG "-fsanitize=address")
  set(ADDRESS_SANITIZER_CXX_FLAG "-fsanitize=address")

elseif(HAVE_C_FLAG_ADDRESS_SANITIZER AND HAVE_CXX_FLAG_ADDRESS_SANITIZER)
  # Older deprecated flag for Asan
  set(HAVE_ADDRESS_SANITIZER TRUE)
  set(ADDRESS_SANITIZER_C_FLAG "-faddress-sanitizer")
  set(ADDRESS_SANITIZER_CXX_FLAG "-faddress-sanitizer")
elseif(HAVE_C_FLAG_ADDRESS_SANITIZER AND NOT HAVE_CXX_FLAG_ADDRESS_SANITIZER)
  message(
    WARNING "C compiler supports asan, but CXX not. Ignoring ASAN support")
elseif(NOT HAVE_C_FLAG_ADDRESS_SANITIZER AND HAVE_CXX_FLAG_ADDRESS_SANITIZER)
  message(
    WARNING "CXX compiler supports asan, but C not. Ignoring ASAN support")
endif()

if(HAVE_ADDRESS_SANITIZER)
  list(APPEND CMAKE_CONFIGURATION_TYPES Asan)

  set(CMAKE_C_FLAGS_ASAN
      "-O1 -g ${ADDRESS_SANITIZER_C_FLAG} -fno-omit-frame-pointer -fno-optimize-sibling-calls"
      CACHE STRING "Flags used by the C compiler during Asan builds." FORCE)
  set(CMAKE_CXX_FLAGS_ASAN
      "-O1 -g ${ADDRESS_SANITIZER_CXX_FLAG} -fno-omit-frame-pointer -fno-optimize-sibling-calls"
      CACHE STRING "Flags used by the C++ compiler during Asan builds." FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_ASAN
      "${ADDRESS_SANITIZER_FLAG}"
      CACHE STRING "Flags used for linking binaries during Asan builds." FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS_ASAN
      "${ADDRESS_SANITIZER_FLAG}"
      CACHE STRING
            "Flags used by the shared libraries linker during Asan builds."
            FORCE)
  set(CMAKE_MODULE_LINKER_FLAGS_ASAN
      "${ADDRESS_SANITIZER_FLAG}"
      CACHE STRING "Flags used by the modules linker during Asan builds." FORCE)
  mark_as_advanced(CMAKE_C_FLAGS_ASAN CMAKE_CXX_FLAGS_ASAN
                   CMAKE_EXE_LINKER_FLAGS_ASAN CMAKE_SHARED_LINKER_FLAGS_ASAN)
endif()

# WARNING: This is wrong for multi-config generators because they don't use and
# typically don't even set CMAKE_BUILD_TYPE
#
# See also https://gitlab.kitware.com/cmake/cmake/-/issues/22591
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#build-configurations
string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if(build_type STREQUAL "asan" AND NOT HAVE_ADDRESS_SANITIZER)
  message(FATAL ERROR
          "Compiler does not support ${CMAKE_BUILD_TYPE} build type")
endif()
