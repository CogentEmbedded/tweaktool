ARG UBUNTU_VERSION
ARG WITH_DEFAULT_CMAKE
ARG KITWARE_REPOSITORY
FROM ubuntu:${UBUNTU_VERSION}

ARG UBUNTU_VERSION
ARG WITH_DEFAULT_CMAKE
ARG KITWARE_REPOSITORY
ENV KITWARE_REPOSITORY=${KITWARE_REPOSITORY}
ENV WITH_DEFAULT_CMAKE=${WITH_DEFAULT_CMAKE}

RUN apt-get update
RUN apt-get install -y software-properties-common ca-certificates wget

RUN DEBIAN_FRONTEND=noninteractive apt-get -y install tzdata

RUN apt-get -y install git build-essential fakeroot devscripts equivs \
    dh-python debhelper-compat ubuntu-dev-tools libreadline-dev \
    scons protobuf-compiler python3-protobuf libmbedtls-dev \
    qt5-default qml-module-qtqml-models2 qml-module-qtquick-controls2 \
    qtdeclarative5-dev qtquickcontrols2-5-dev expect

RUN if [ "${WITH_DEFAULT_CMAKE}" = "ON" ] ; then \
        wget --no-check-certificate -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add - ; \
        apt-add-repository -y "${KITWARE_REPOSITORY}" ; \
        apt-get -y update ; \
    fi

RUN apt-get install -y cmake

WORKDIR  /app
RUN mkdir deploy
