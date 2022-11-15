#!/bin/sh
set -e

TARGET_DIR=$(pwd)
BUILD_DIR=$(mktemp -d -t ci-$(date +%Y-%m-%d-%H-%M-%S)-XXXXXXXXXX)
NNG_VER=1.5.2
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
pull-lp-source nng $NNG_VER-2
sed -ie 's/Build-Depends: debhelper-compat (= .*)/Build-Depends: debhelper-compat (= 11)/g' ./nng-$NNG_VER/debian/control
cd ./nng-$NNG_VER
dch -n "Backported to debhelper-compat 11"
debuild -us -uc -b
find "$BUILD_DIR" -name "*.deb" -exec cp {} "$TARGET_DIR" \;
debi
