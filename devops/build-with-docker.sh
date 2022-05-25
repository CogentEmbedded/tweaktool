#!/bin/sh

set -e

build_images() {
    docker-compose up --build --detach
    docker-compose down
}

build_debs() {
    mkdir -p ./docker-build
    docker-compose up --detach
    for ubuntu_version in 18_04 20_04
    do
        docker cp . container_tweaktool_ubuntu_${ubuntu_version}:/app/tweaktool
        docker exec container_tweaktool_ubuntu_${ubuntu_version} /bin/sh \
            -c 'cd /app/deploy && rm -rf ./*'
        docker exec container_tweaktool_ubuntu_${ubuntu_version} /bin/sh \
            -c 'cd /app/deploy && ../tweaktool/build-nng-debs.sh'
        docker exec container_tweaktool_ubuntu_${ubuntu_version} /bin/sh \
            -c 'cd /app/deploy && ../tweaktool/build-debs.sh ../tweaktool'
        docker cp container_tweaktool_ubuntu_${ubuntu_version}:/app/deploy ./docker-build/ubuntu_${ubuntu_version}
    done
    docker-compose down
}

if [ "$1" = "--build-images-only" ]; # with --build-images-only this script only creates build environments
then build_images                    # otherwise it shall build packages.
else build_debs                      # If environments doesn't exist, it shall build images first.
fi
