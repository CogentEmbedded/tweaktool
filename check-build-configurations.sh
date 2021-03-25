#!/bin/bash

set -xe
shopt -s expand_aliases

# For cross-compiling use-case, source your toolchain environment file here
#  and comment `make test` or `make package` appropriately.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

# Print current cmake status and version
which cmake
cmake --version

run_build() {
    SUFFIX="$1"
    shift

    BUILD_DIR="${ROOT_DIR}/build-test/${SUFFIX}"
    EXTERNAL_BUILD_DIR="${ROOT_DIR}/build-test/external-${SUFFIX}"

    # Install with a weird directory name to check that all CMAke paths are derived correctly
    PREFIX="/usr2"

    (# subshell for cd commands below
        rm -rf "${BUILD_DIR}"
        rm -rf "${EXTERNAL_BUILD_DIR}"

        mkdir -p "${BUILD_DIR}"
        cd "${BUILD_DIR}" || exit

        echo "$@" > ./build_condifugration.txt

        # Build the selected configuration
        cmake "${ROOT_DIR}" "-DCMAKE_INSTALL_PREFIX=${PREFIX}" "$@"
        pwd
        make -j VERBOSE=1

        BUILD_TESTS=$(cmake -LA -N .. | grep BUILD_TESTS | cut -d "=" -f2)
        echo "BUILD_TESTS=${BUILD_TESTS}"
        if [ "xON" = "x${BUILD_TESTS}" ]; then
            export CTEST_OUTPUT_ON_FAILURE=1
            make test ARGS=-j12
        fi

        WITH_DOXYGEN=$(cmake -LA -N .. | grep WITH_DOXYGEN | cut -d "=" -f2)
        echo "WITH_DOXYGEN=${WITH_DOXYGEN}"
        if [ "xON" = "x${WITH_DOXYGEN}" ]; then
            make docs
        fi

        make install DESTDIR="${BUILD_DIR}/install"
        make package

        LIBDIR=$(cmake -LA -N .. | grep CMAKE_INSTALL_LIBDIR | cut -d "=" -f2)

        # Check that installed cmake configuration is usable
        mkdir -p "${EXTERNAL_BUILD_DIR}"
        cd "${EXTERNAL_BUILD_DIR}" || exit

        cmake "${ROOT_DIR}/cmake/external-usage-test" \
            "-Dtweak2_DIR=${BUILD_DIR}/install/${PREFIX}/${LIBDIR}/cmake/tweak2"
        pwd
        make -j VERBOSE=1
    )
}

for arg1 in -DCMAKE_BUILD_TYPE={Debug,Release}; do
    for arg2 in -DBUILD_SHARED_LIBS={ON,OFF}; do
        for arg3 in -DBUILD_TESTS={ON,OFF}; do
            for arg4 in -DBUILD_GUI={ON,OFF}; do
                for arg5 in -DWITH_DOXYGEN={ON,OFF}; do
                    for arg6 in -DWITH_QTCHARTS={ON,OFF}; do
                        for arg7 in -DTWEAK_COMMON_LOG_LEVEL={Trace,Debug,Test,Warn,Error,Fatal}; do
                            run_build xxx $arg1 $arg2 $arg3 $arg4 $arg5 $arg6 $arg7 "$@"
                        done
                    done
                done
            done
        done
    done
done

