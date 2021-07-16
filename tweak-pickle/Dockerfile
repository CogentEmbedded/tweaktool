ARG UBUNTU_VERSION
FROM ubuntu:20.04

RUN apt-get update
RUN apt-get install -y software-properties-common ca-certificates wget

RUN DEBIAN_FRONTEND=noninteractive apt-get -y install tzdata

RUN apt-get -y install build-essential unzip python3 python3-pip

RUN pip3 install protobuf

WORKDIR  /app

RUN wget https://github.com/google/protobuf/releases/download/v3.4.0/protoc-3.4.0-linux-x86_64.zip

RUN unzip protoc-3.4.0-linux-x86_64.zip -d protoc3
RUN mv protoc3/bin/* /usr/local/bin/
RUN mv protoc3/include/* /usr/local/include/

RUN wget https://github.com/nanopb/nanopb/archive/refs/tags/0.4.5.zip
RUN unzip ./0.4.5.zip

RUN mkdir tweakpb
