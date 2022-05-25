#!/bin/sh
set -e

TARGET_DIR=$(pwd)

[ $# -eq 0 ] && >&2 echo "Usage: $0 [tweaktool source directory]" && exit 1

cd $@
SOURCE_DIR=$(pwd)
BUILD_DIR=$(mktemp -d -t ci-$(date +%Y-%m-%d-%H-%M-%S)-XXXXXXXXXX)
mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DBUILD_GUI=ON -DBUILD_SHARED_LIBS=ON -DPACKAGE_RPM=OFF \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr $SOURCE_DIR

make -j
make install
make package
find $BUILD_DIR -name "*.deb" | xargs -I '{}' cp {} $TARGET_DIR

cd $TARGET_DIR
rm -rf $BUILD_DIR
