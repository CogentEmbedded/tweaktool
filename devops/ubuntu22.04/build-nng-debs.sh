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
quilt import -P ce_baseline.patch "$TARGET_DIR/ce_baseline.patch"
quilt push
quilt import -P ce_nng_hub.patch "$TARGET_DIR/ce_nng_hub.patch"
quilt push
dch -n "CE NNG hub protocol support"
quilt import -P ce_fix_mbedtls_lookup.patch "$TARGET_DIR/ce_fix_mbedtls_lookup.patch"
quilt push
dch -n "Fix mbedTLS lookup"
debuild -us -uc -b
find "$BUILD_DIR" -name "*.deb" -exec cp {} "$TARGET_DIR" \;
debi
