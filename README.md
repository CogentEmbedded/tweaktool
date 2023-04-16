# Cogent Tweak Tool

## Overview

Tweak tool is a lightweight library for adjusting parameters of realtime systems.

It is written in C99 using POSIX networking and threading facilities.

The idea resembles Windows registry or gconf: user creates configurable values (tweaks)
that can be updated using remote GUI or a command line client.

The protocol is reactive and full duplex by design, so both sides are kept in sync
and get live updates.

## Prebuilt Releases

Download location: <https://github.com/CogentEmbedded/tweaktool/releases>

## Building from Source Code

Tweak tool uses docker to build external dependencies not available directly from  host OS.

Docker is a framework for creating reproducible environments using lightweight virtual machines.
This library provides scripts for reproducible builds of its packages for Ubuntu LTS variants: 18.04, 20.04 and 22.04.

First, you have to install docker:

<https://docs.docker.com/get-docker/>

Ensure current user belongs to "docker" group.

### Step-by-step guide to build on Ubuntu LTS variants

Step 1

Choose appropriate Ubuntu version (18.04, 20.04, 22.04) and set `VERSION` variable.
Prepare docker image with dependencies.

```bash
export VERSION=18.04
DOCKER_BUILDKIT=1 docker build -t tweak-builder:${VERSION} devops/ubuntu${VERSION}
```

To build tweak in host OS install build dependencies as required by `devops/ubuntu${VERSION}/Dockerfile`.

Step 2

Fetch prebuilt NNG library from Docker image.

```bash
mkdir -p nng-prebuilt/${VERSION}
DOCKER_BUILDKIT=1 docker build -f devops/ubuntu${VERSION}/fetch-nng --output type=local,dest=nng-prebuilt/${VERSION} devops/ubuntu${VERSION}
```

Install pre-built NNG packages

```bash
sudo apt-get -y install
sudo dpkg -i nng-prebuilt/${VERSION}/*.deb
```

Step 3

Checkout source code.

```bash
git clone https://github.com/CogentEmbedded/tweaktool.git --recurse-submodules
cd tweaktool
```

Step 4

----

Optional: if using docker

Create docker image with same user as on host to minimize hassle with file permissions:

```bash
DOCKER_BUILDKIT=1 docker build \
    --build-arg USER \
    --build-arg UID=$(id -u) \
    --build-arg GID=$(id -g) \
    -t tweak-user-builder:${VERSION} \
    -f devops/ubuntu${VERSION}/user-builder \
    devops/ubuntu${VERSION}
```

Start temporary docker container with current source code

```bash
docker run --rm -it -v "$PWD:$PWD" --workdir="$PWD" tweak-user-builder:${VERSION} bash
```

----

Following steps can be executed inside docker container or on host.

Option 1: if using cmake 3.23+ use preset

```bash
rm -rf build/pc-linux-release
cmake --preset pc-linux-release
cmake --build --preset pc-linux-release
```

After successful build deb and rpm files are available at build output directory `build/pc-linux-release`.

Option 2: configure build manually according to application needs, e.g. for PC:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_GUI=ON ..
cmake --build .
cmake --build . --target package
```

User could provide additional flags to CMake:

- BUILD_TESTS - Adds "test" target. Default: OFF.
- BUILD_GUI - Builds Qt GUI client. Default: OFF.
- BUILD_MOCK - Build mock tweak server. Default: ON.
- BUILD_CLI - Build console tweak client application. Default: ON, Windows is not supported.
- WITH_DOXYGEN - Generate tweak2 documentation from source code using Doxygen. Default: OFF.
- WITH_PYTHON - enable python bindings. Default: ON.
- WITH_WIRE_NNG - Adds NNG support to connection factory. Default: ON.

For systems with RPMSG IPC:

- WIRE_RPMSG_BACKEND - enable specific backend. Default: OFF. Possible values:
  - OFF - disabled
  - TI_API - for direct use of TI API
  - CHRDEV - for Linux RPMSG driver
- WITH_TWEAK_GW - build gateway application that creates bridge between RPMSG and NNG.
  It is used in case when tweak server is running on core with RTOS and tweak access is required from Linux host.

### Using CMake Presets

Since CMake v3.23 build presets are available.

If default paths are not valid for cross-build toolchains, create
new file `CMakeUserPresets.json` at the project root and fill it with custom settings.

Add as many overrides as needed, user defined presets file is not version controlled.

Visual Studio Code with CMake Tools plugin will allow configuration and building via its UI.

From command line it can be invoked as:

```bash
cmake --preset pc-linux-release
cmake --build --preset pc-linux-release
```

For Cogent Embedded VB4 target prebuilt binary version can be generated with following commands:

```bash

cmake --preset vb4-r5f-sysbios-release
cmake --build --preset vb4-r5f-sysbios-release
cd build/vb4-r5f-sysbios-release
DESTDIR=/opt/vb4/ti-processor-sdk-rtos-j721e-evm-07_01_00_11/vision_apps/utils ninja install
cd -

cmake --preset vb4-a72-linux-release
cmake --build --preset vb4-a72-linux-release
for f in build/vb4-a72-linux-release/tweaktool-*.tar.gz; do
    # Update concerto targetfs
    sudo tar xf $f -C /opt/vb4/ti-processor-sdk-rtos-j721e-evm-07_01_00_11/targetfs
    # Update SDK sysroot
    sudo tar xf $f -C /opt/vb4/ti-processor-sdk-linux-j7-evm-07_01_00_10/linux-devkit/sysroots/aarch64-linux/
done
```

Rebuild VB4 BSP afterwards.

List all available presets in command line:

```bash
cmake --list-presets
```

### Building for TDA4 EVM targets

#### Prebuilt SDK 8.04

```bash
export EVM_BSP_PATH=/opt/evm

${DOWNLOADS}/ti-processor-sdk-linux-j7-evm-08_04_00_11-Linux-x86-Install.bin \
    --mode unattended \
    --prefix ${EVM_BSP_PATH}/ti-processor-sdk-linux-j7-evm-08_04_00_11

tar xf ${DOWNLOADS}/ti-processor-sdk-rtos-j721e-evm-08_04_00_02.tar.gz

${DOWNLOADS}/ti-processor-sdk-rtos-j721e-evm-08_04_00_02-addon-linux-x64-installer.run \
    --mode unattended \
    --prefix ${EVM_BSP_PATH}

tar xf ${DOWNLOADS}/ti-processor-sdk-rtos-j721e-evm-08_04_00_02-prebuilt.tar.gz

# Update Linux development kit sysroot with RTOS components

sudo tar xf ${EVM_BSP_PATH}/ti-processor-sdk-rtos-j721e-evm-08_04_00_02-prebuilt/tisdk-default-image-j7-evm.tar.xz \
    -C ${EVM_BSP_PATH}/ti-processor-sdk-linux-j7-evm-08_04_00_11/linux-devkit/sysroots/aarch64-linux

sudo cp -a ${EVM_BSP_PATH}/ti-processor-sdk-rtos-j721e-evm-08_04_00_02-prebuilt/rootfs/* \
    ${EVM_BSP_PATH}/ti-processor-sdk-linux-j7-evm-08_04_00_11/linux-devkit/sysroots/aarch64-linux

```

#### Building NNG

```bash
# Use mainline after https://github.com/nanomsg/nng/pull/1623 is merged
git clone https://github.com/CogentEmbedded/nng.git
cd nng

mkdir build
cd build

source ${EVM_BSP_PATH}/ti-processor-sdk-linux-j7-evm-08_04_00_11/linux-devkit/environment-setup-aarch64-linux

# Use cmake 3.17+, otherwise exported target files are not valid for static library dependencies, see https://gitlab.kitware.com/cmake/cmake/-/issues/20204
/usr/bin/cmake \
    -DCMAKE_TOOLCHAIN_FILE=${OE_CMAKE_TOOLCHAIN_FILE} \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    ..

make -j

# Update BSP SDK
sudo make DESTDIR=${OECORE_TARGET_SYSROOT} install

# Update Concerto targetfs if apps from BSP build use tweak on Linux
sudo make DESTDIR=${EVM_BSP_PATH}/ti-processor-sdk-rtos-j721e-evm-08_02_00_05/targetfs install

```

#### Build Tweak for R5F FREERTOS

```bash
cd tweaktool
rm -rf build/evm-r5f-freertos-release
cmake --preset evm-r5f-freertos-release
cmake --build --preset evm-r5f-freertos-release

```

On successful build `build/evm-r5f-freertos-release/tweaktool-${TWEAK_VERSION}-Concerto-dev.tar.gz` package is generated.

To integrate it into BSP for R5F app development with tweak support:

```bash
tar xf build/evm-r5f-freertos-release/tweaktool-*-Concerto-dev.tar.gz -C ${VISION_APPS}/utils
```

Open `${VISION_APPS}/platform/j721e/rtos/mcu2_0/concerto_mcu2_0_inc.mak` in editor
and add tweak libraries into R5F image:

```mak
STATIC_LIBS += tweak2app tweak2server tweak2json tweak2metadata tweak2pickle tweak2wire tweak2common
```

R5F application that uses tweak shall add include search path into its own concerto.mak

```mak
IDIRS  += $(VISION_APPS_PATH)/utils/cogent_tweaktool/include
```

Launch `tweak-gw` A72 Linux application on target to initialize RPC gateway and get access to R5F tweak server
via Linux network.

#### Build Tweak for A72 Linux

```bash
cd tweaktool
rm -rf build/evm-a72-linux-release
cmake --preset evm-a72-linux-release
cmake --build --preset evm-a72-linux-release

```

Update Linux development kit sysroot to build apps externally and targetfs for concerto system

```bash
tar xf build/evm-a72-linux-release/tweaktool-*-Linux-dev.tar.gz  -C ${EVM_BSP_PATH}/ti-processor-sdk-linux-j7-evm-08_04_00_11/linux-devkit/sysroots/aarch64-linux
tar xf build/evm-a72-linux-release/tweaktool-*-Linux-dev.tar.gz  -C ${EVM_BSP_PATH}/targetfs
```

Unpack tweak tools from `build/evm-a72-linux-release/tweaktool-*-Linux-tools.tar.gz` to target rootfs.

Applications that are built with concerto.mak and use tweak shall add:

```mak
IDIRS  += $(VISION_APPS_PATH)/utils/cogent_tweaktool/include
STATIC_LIBS += tweak2app tweak2server tweak2json tweak2metadata tweak2pickle tweak2wire tweak2common nng
```


To build with BSP at non-default location (`/opt/$DEVICE_NAME`) provide `BSP_ROOT` via env.

```bash
function build_combo()
{
    TARGET=$1
    CPU=$2
    OS=$3
    BUILD_TYPE=$4
    COMBO="${TARGET}-${CPU}-${OS}-${BUILD_TYPE}"

    rm -rf build/${COMBO}-env
    cmake --preset ${COMBO}-env
    cmake --build --preset ${COMBO}-env
}

# Adjust to proper location for VB4
export BSP_ROOT=/opt/vb4
build_combo vb4 r5f sysbios release
build_combo vb4 a72 linux release

# Adjust to proper location for EVM
export BSP_ROOT=/opt/evm
build_combo evm r5f freertos release
build_combo evm a72 linux release

```

### Building on Visual Studio

#### Preconditions

- Microsoft Visual Studio Community 2019 or later
- Integrated vcpkg
- Installed vcpkg packages:
  - zstd
  - nng
  - getopt
  - qt5-base
  - qt5-declarative
  - qt5-quickcontrols
  - qt5-quickcontrols2
  - qt5-tools
- NSIS installer for packaging

#### Compilation

In MSVC++ IDE, open tweak2 directory with `File/Open folder...` menu.

Choose build configuration preset: "PC Windows Release Config" (or debug variant).

Build project.

#### Components

- `tweak2::server` is a thin wrapper around `tweak2::app`. It provides simple C99 context-less API to create a collection of items, alter item's values and monitor
changes initiated by client application.
This library is a good starting point for most applications.

- `tweak2::compatibility-server` library. This library provides
legacy API for migration from versions 1.0 and 1.1 of this library.

- `tweak2::app` is the core library. It provides client and server endpoints for
tweak2 protocol.
In contrast to tweak2::server library, it allows to have multiple server endpoints with
and gives more detailed error diagnostics.

- `tweak2::pickle` is a RPC protocol library used by `tweak2::app`.

- `tweak2::wire` transport library used by `tweak2::pickle`. It operates on chunks of
binary data.

Only NNG and RPMSG backends are supported at the moment. Serial and CAN could be added without API change.

- `tweak-mock-server`. This program emulates a user application linked to `tweak2:s:server`. It creates N items, changes them randomly.

- `tweak-app-cl` is an interactive console client.

- `tweak-gui` is a GUI client based on Qt/QML.

This application can be installed using deb or rpm package
managers. User can build deb and rpb packages using cmake,
see "Build tweaktool" section.

### Building for QNX 7.x

Build is done using standard cmake cross compilation routines:

- Make sure that you have `$QNX_HOST` and `$QNX_TARGET` set properly.
  This can be done by sourcing the appropriate QNX SDP environment file, for example `source /opt/qnx700//qnxsdp-env.sh`.
- Then cross compile Tweak Tool with nng as a submodule:

```bash
cd tweaktool
rm -rf build/qnx-x86_64-release
cmake --preset qnx-x86_64-release
cmake --build --preset qnx-x86_64-release
```

After that, Tweak Tool is built. User could check tweak-mock-server on target platform.

## License

Tweak Tool is developed under MIT license. For full license text, see [COPYING](COPYING).
