#!/bin/sh

set -e

build_images() {
    docker-compose up --build --detach
    docker-compose down
}

build_stubs() {
    docker-compose up --detach
    docker cp . container_tweaktool_nanopb_ubuntu_20_04:/app/tweakpb
    docker exec container_tweaktool_nanopb_ubuntu_20_04 /bin/sh \
        -c 'cd /app/tweakpb && ./make-stubs.sh'
    rm -rf ./src/autogen
    docker cp container_tweaktool_nanopb_ubuntu_20_04:/app/tweakpb/autogen ./src/autogen
    docker-compose down
}

if [ "$1" = "--build-images-only" ]; # with --build-images-only this script only creates build environments
then build_images                    # otherwise it shall build packages.
else build_stubs                     # If environments doesn't exist, it shall build images first.
fi
