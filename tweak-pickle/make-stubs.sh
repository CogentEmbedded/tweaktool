#!/bin/sh
set -e

python3 ../nanopb-0.4.5/generator/nanopb_generator.py tweak.proto
mkdir autogen
cp ./tweak.pb.h ./autogen
cp ./tweak.pb.c ./autogen
cp /app/nanopb-0.4.5/pb_common.c ./autogen
cp /app/nanopb-0.4.5/pb_common.h ./autogen
cp /app/nanopb-0.4.5/pb_decode.c ./autogen
cp /app/nanopb-0.4.5/pb_decode.h ./autogen
cp /app/nanopb-0.4.5/pb_encode.c ./autogen
cp /app/nanopb-0.4.5/pb_encode.h ./autogen
cp /app/nanopb-0.4.5/pb.h ./autogen
