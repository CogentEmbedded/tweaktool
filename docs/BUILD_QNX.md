# Cogent Tweak Tool build for QNX

## Overview

   I assume here that you have following structure:

   `$TOP_DIR/nng`

   `$TOP_DIR/nng-qnx-build`

   `$TOP_DIR/tweaktool`

   `$TOP_DIR/tweaktool-qnx-build`

   `$TOP_DIR/target_root`

Pre-condition: clone tweaktool and nng to `$TOP_DIR/tweaktool`
and `$TOP_DIR/nng`, respectively. Build directories are initially empty. Target root is empty as well.

Build is done using standard cmake cross compilation routines:

 - Make sure that you have `$QNX_HOST` and `$QNX_TARGET` are being set properly.
   In current CE setup, we have script `./sdp_isp/prjtools/qnx/ver_7.0.0/qnxsdp-env.sh`
   for it.

 - Cross compile nng using qnx toolchain file.
```bash
   cd $TOP_DIR/nng-qnx-build
   cmake -DCMAKE_INSTALL_PREFIX:PATH=$TOP_DIR/target_root \
    -DCMAKE_TOOLCHAIN_FILE=../tweaktool/cmake/qnx_7.0.0_linux_x86_64.cmake \
    -DQNX_PROCESSOR=aarch64le ../nng
   make
   make install
```

 - Then cross compile Tweak Tool itself.
```bash
   cd $TOP_DIR/tweaktool-qnx-build
   cmake -DCMAKE_INSTALL_PREFIX:PATH=$TOP_DIR/target_root -DBUILD_CLI=OFF \
         -DCMAKE_TOOLCHAIN_FILE=../tweaktool/cmake/qnx_7.0.0_linux_x86_64.cmake \
         ../tweaktool
   make
   make install
```

After that, Tweak Tool is built. User could check tweak-mock-server on target platform.
