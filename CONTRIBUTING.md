<!--
Copyright (c) 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-->

# Contributing

## Release Check List

Before making a public release, check the following items:

1. Copyright headers shall be updated for all newly created or modified files.
2. Review every change to Public API headers and make sure they are:
   1. Justified,
   2. Maintain backward compatibility.
3. Make sure all tests run successfully.
4. Run `check-build-configurations.sh` for x86 PC and Yocto aarch64.
5. Validate Jenkins build

## Script for Checking Build Configurations

Script file `check-build-configurations.sh` executes verbose cmake build for all
combinations of input options:

* BUILD_SHARED_LIBS
* CMAKE_BUILD_TYPE
* BUILD_TESTS  (run `make test` if it is on)
* WITH_DOXYGEN  (runs `make docs` if it is on)
* BUILD_GUI

This script assumes that doxygen and all other necessary packages are
pre-installed on the build machine or in the SDK. A simple test project is compiled against
libraries installed into a temporary directory.
