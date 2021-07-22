# Cogent Tweak Tool

## Overview

Tweak tool is a lightweight library for adjusting parameters of realtime systems.

It is written in C99 using POSIX networking and threading facilities.

The idea resembles Windows registry or gconf: user creates configurable values (tweaks)
that can be updated using remote GUI or a command line client.

The protocol is reactive and full duplex by design, so both sides are kept in sync
an get live updates.

## Installation and Configuration

Two options are available: using docker or local build.
Docker is better option for making deliveries. With this option, builds are reproducible
and user's environment doesn't interfere with build process in any way.

Local builds are better for developers. With this option, user could play with compilation
options, build program with Debug symbols and/or trace logs.

### Step-by-step guide to build tweaktool2 on Ubuntu 20.0

Step 1

Install basic Ubuntu build utilities and Qt developer's packages.

```bash
sudo apt-get -y install git cmake build-essential \
    libreadline-dev qt5-default qml-module-qtqml-models2 qml-module-qtquick-controls2 \
    qml-module-qtquick-* qml-module-qt-labs-settings \
    qtdeclarative5-dev qtquickcontrols2-5-dev expect
```

Step 2

Checkout actual tweaktool2 branch.

```bash
git clone https://github.com/CogentEmbedded/tweaktool.git --recursive
```

Step 3

Install NNG library for tweaktool 2.

Option A: Install pre-built NNG packages

```bash
sudo apt-get -y install libmbedtls-dev libmbedcrypto3 libmbedtls12 libmbedx509-0
sudo dpkg -i ./tweaktool/nng-prebuilt/ubuntu_20_04/libnng1_1.4.0-1build1.1_amd64.deb \
             ./tweaktool/nng-prebuilt/ubuntu_20_04/libnng-dev_1.4.0-1build1.1_amd64.deb \
             ./tweaktool/nng-prebuilt/ubuntu_20_04/nng-utils_1.4.0-1build1.1_amd64.deb
```

Option B: Build NNG packages manually

Infrastructure for deb packaging is needed here.
This script installs NNG packages being built, thus it requires sudo.

```bash
sudo apt-get -y install fakeroot devscripts equivs dh-python debhelper-compat ubuntu-dev-tools \
    scons protobuf-compiler libmbedtls-dev libmbedcrypto3 libmbedtls12 libmbedx509-0 python3-protobuf
sudo ../tweaktool/build-nng-debs.sh
```

Step 4

```bash
mkdir -p ./tweaktool-build

cd ./tweaktool-build
```

Step 5

```bash
../tweaktool/build-debs.sh ../tweaktool
```

After all these commands user should get all the debs inside ./tweak-build directory.

### Step-by-step guide to build tweaktool2 on Ubuntu 18.04

Step 1

Install basic Ubuntu build utilities and Qt developer's packages.

```bash
sudo apt-get -y install git cmake build-essential \
    libreadline-dev qt5-default qml-module-qtqml-models2 qml-module-qtquick-controls2 \
    qml-module-qtquick-* qml-module-qt-labs-settings \
    qtdeclarative5-dev qtquickcontrols2-5-dev expect
```

Step 2

Checkout actual tweaktool2 branch.

```bash
git clone https://github.com/CogentEmbedded/tweaktool.git --recursive
```

Step 3

Install NNG library for tweaktool 2.

---
**NOTE**
There's no step-by-step entry on how to build NNG packages for Ubuntu 18.
It requires non trivial build environment.

If the user aren't satisfied with prebuilt deb files, one have to refer
advanced build recipes.
---

```bash
sudo apt-get -y install libmbedtls-dev libmbedcrypto1 libmbedtls10 libmbedx509-0
sudo dpkg -i ./tweaktool/nng-prebuilt/ubuntu_18_04/libnng1_1.4.0-1build1.1_amd64.deb \
             ./tweaktool/nng-prebuilt/ubuntu_18_04/libnng-dev_1.4.0-1build1.1_amd64.deb \
             ./tweaktool/nng-prebuilt/ubuntu_18_04/nng-utils_1.4.0-1build1.1_amd64.deb
```

Step 4

---
**NOTE**
There's currently a defect not allowing to build tweaktool with cmake provided with Ubuntu 18.04

build-debs.sh could fail with error in target "SortFilterProxyModel".

Until there's a solution, there's no option but to upgrade cmake, as in Step 3/Option B.
---

```bash
mkdir -p ./tweaktool-build
cd ./tweaktool-build
../tweaktool/build-debs.sh ../tweaktool
```

After all these commands user should get all the debs inside ./tweak-build directory.

### Build Using Docker

Docker is a framework for creating reproducible environments using lightweight virtual machines.
This library provides scripts for reproducible builds of its packages for Ubuntu 18.04
and Ubuntu 20.04.

First, you have to install docker:

<https://docs.docker.com/get-docker/>

Then, you have to install docker-compose utility:
<https://docs.docker.com/compose/install/>

Then, start script build-with-docker.sh --build-images-only from project root directory.
This shall build reproducible build environment. This has to be run only once.
Since then, existing build environment could be re-used many times.
When there's build environment available, run build-with-docker.sh without arguments.
It shall build deb packages in ./docker-build directory.

Note: If your linux user doesn't belong to "docker" users' group,
the script build-with-docker.sh won't work without sudo.

## Build on local linux installation

This section covers Ubuntu 18.04 and Ubuntu 20.04 Linux distributions.
This library is reported to work in Fedora 33. Other modern Linux
distributions should work with library without any problems.

### Build pre-conditions

```bash
sudo apt-get install git build-essential libreadline-dev \
    qt5-default qml-module-qtqml-models2 qml-module-qtquick-controls2 \
    qml-module-qtquick-* qml-module-qt-labs-settings \
    qtdeclarative5-dev qtquickcontrols2-5-dev expect
```

Also, there's dependency on nng v1.4.0 library. Version prior to that version
are known to have deadlock in nng_close during TCP disconnect.

User could run docker build and take libnng and libnng-dev debs from docker build directory,
thus skipping need for the new step and allowing one to go straight to fetch tweaktool/build tweaktool
sections.

### Build NNG on Ubuntu 18.04

This library is available on [github](https://github.com/nanomsg/nng).

Ubuntu 18.04 version is known to have older version of cmake that is not sufficient to build NNG.
User should install cmake from KitWare.

Cmake developers (Kitware) have their own PPA with newer cmake releases.

```bash
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
sudo apt-get update
sudo apt-get install cmake
```

Check if cmake has been updated:

```bash
cmake -version
```
Version greater than 3.13  is expected.

```bash
git clone https://github.com/nanomsg/nng.git
cd ./nng
git checkout tags/v1.4.0
mkdir build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
make -j
sudo make install
```

---
**NOTE**

Another and probably more preferred option is to refer script
[build-nng-deps.sh](build-nng-deps.sh) from project directory
to build proper Ubuntu-styled deb packages.

Pre-conditions for that script are:

```bash
apt-get -y install git build-essential fakeroot devscripts equivs \
    dh-python debhelper-compat ubuntu-dev-tools libreadline-dev \
    scons protobuf-compiler python3-protobuf libmbedtls-dev
```
---

### Build NNG on Ubuntu 20.04

Known issues:

Ubuntu 20.04 LTS cmake 3.16.+ will not create properly export files for default configuration
(NNG as a static library).

See: <https://gitlab.kitware.com/cmake/cmake/-/issues/20204>

Thus, user have to install cmake from KitWare to build library, selecting focal repository 

```bash
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt-get update
sudo apt-get install cmake
```

Other steps are the same as in Ubuntu 20.04 section. The script build-nng-deps.sh
works with this Ubuntu version as well.

## Fetch tweaktool sources

```bash
git clone https://github.com/CogentEmbedded/tweaktool.git --recursive
```

## Build tweaktool

```bash
cd tweaktool
mkdir ./build
cd ./build
cmake -DBUILD_GUI=ON -DBUILD_TESTS=ON ..
make -j
make package
```

User could provide additional flags to CMake:

 - BUILD_TESTS Adds "make test" target. Default: OFF.
 - BUILD_GUI Builds gui client. Requires Qt5. Default: OFF.
 - WITH_DOXYGEN Run doxygen on source code. Default: OFF.
 - WITH_WIRE_NNG Adds NNG support to connection factory. Default: ON.
 - WITH_WIRE_RPMSG Adds RPMSG support to connection factory. Default: OFF.

## Components

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

## License

Tweak Tool is developed under MIT license. For full license text, see [COPYING](COPYING).
