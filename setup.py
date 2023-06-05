# Configuration of Python3 binding
#
# Copyright 2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

import re
import subprocess

from skbuild import setup


def get_git_version() -> str:
    try:
        res = subprocess.run(['git', 'describe', '--tags', '--always'],
                             capture_output=True,
                             check=True)
        line = res.stdout.decode('utf-8')
        for m in re.finditer(r'[0-9]+\.[0-9]+(\.[0-9]+(-[0-9]+)?)?', line):
            return m[0]
    except Exception:
        pass
    return "2.1.8"


def exclude_extra_files(cmake_manifest):
    def filter_keep(name: str) -> bool:
        if name.endswith((".a", ".h", ".cmake", "bin/nngcat")):
            return False

        return True

    return list(filter(filter_keep, cmake_manifest))


setup(
    name="tweak2",
    version=get_git_version(),
    description="",
    author="Cogent Embedded Inc.",
    license="MIT",
    packages=["tweak2"],
    package_dir={"tweak2": "tweak-py"},
    cmake_install_dir="tweak-py",
    cmake_args=[
        "-DBUILD_CLI=OFF",
        "-DBUILD_GUI=OFF",
        "-DBUILD_MOCK=OFF",
        "-DBUILD_TESTS=OFF",
        "-DWITH_NNG_SUBMODULE=ON",
    ],
    python_requires=">=3.6",
    setup_requires=[
        "setuptools",
        "scikit-build>=0.13",
        "cmake",
        "ninja"
    ],
    cmake_process_manifest_hook=exclude_extra_files,
)
