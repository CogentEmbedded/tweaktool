#!/bin/sh
set -e

TARGET_DIR=$(pwd)
BUILD_DIR=$(mktemp -d -t ci-$(date +%Y-%m-%d-%H-%M-%S)-XXXXXXXXXX)
mkdir -p $BUILD_DIR
cd $BUILD_DIR
pull-lp-source nng 1.4.0-1build1
sed -ie 's/Build-Depends: debhelper-compat (= 12)/Build-Depends: debhelper-compat (= 11)/g' ./nng-1.4.0/debian/control
cd ./nng-1.4.0
dch -n "Backported to debhelper-compat 11"
debuild -us -uc -b
find $BUILD_DIR -name "*.deb" | xargs -I '{}' cp {} $TARGET_DIR
debi
cd $TARGET_DIR
