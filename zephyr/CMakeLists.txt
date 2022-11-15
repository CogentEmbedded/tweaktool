# SPDX-License-Identifier: MIT

#
# CMake build configuration for Cogent Tweak Tool.
#
# Copyright (c) 2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

if(CONFIG_TWEAKTOOL)

  set(TWEAKTOOL_DIR ${ZEPHYR_CURRENT_MODULE_DIR})

  zephyr_library()

  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak-pickle/src/autogen)
  zephyr_include_directories(${TWEAKTOOL_DIR}/extern/uthash/src)
  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak-app/include)
  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak-common/include)
  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak-json/include)
  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak-metadata/include)
  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak-pickle/include)
  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak-wire/include)
  zephyr_include_directories(${TWEAKTOOL_DIR}/tweak2lib/include)

  # Custom options
  zephyr_compile_definitions(TWEAK_LOG_LEVEL=2) # Test
  zephyr_compile_definitions(WITH_WIRE_NNG)

  zephyr_library_sources(
    ${TWEAKTOOL_DIR}/tweak-app/src/tweakappclient.c
    ${TWEAKTOOL_DIR}/tweak-app/src/tweakappcommon.c
    ${TWEAKTOOL_DIR}/tweak-app/src/tweakappfeatures.c
    ${TWEAKTOOL_DIR}/tweak-app/src/tweakappqueue.c
    ${TWEAKTOOL_DIR}/tweak-app/src/tweakappserver.c
    ${TWEAKTOOL_DIR}/tweak-app/src/tweakmodel.c
    ${TWEAKTOOL_DIR}/tweak-app/src/tweakmodel_uri_to_tweak_id_index.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweak_id_gen_zephyr.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweakbuffer.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweaklog.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweaklog_format_time_zephyr.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweaklog_out_stderr.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweaklog_thread_id_zephyr.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweakstring.c
    ${TWEAKTOOL_DIR}/tweak-common/src/tweakvariant.c
    ${TWEAKTOOL_DIR}/tweak-json/src/tweakjson.c
    ${TWEAKTOOL_DIR}/tweak-metadata/src/tweakmetadata.c
    ${TWEAKTOOL_DIR}/tweak-pickle/src/autogen/pb_common.c
    ${TWEAKTOOL_DIR}/tweak-pickle/src/autogen/pb_decode.c
    ${TWEAKTOOL_DIR}/tweak-pickle/src/autogen/pb_encode.c
    ${TWEAKTOOL_DIR}/tweak-pickle/src/autogen/tweak.pb.c
    ${TWEAKTOOL_DIR}/tweak-pickle/src/tweakpickle_client_pb.c
    ${TWEAKTOOL_DIR}/tweak-pickle/src/tweakpickle_pb_util.c
    ${TWEAKTOOL_DIR}/tweak-pickle/src/tweakpickle_pb_util.h
    ${TWEAKTOOL_DIR}/tweak-pickle/src/tweakpickle_server_pb.c
    ${TWEAKTOOL_DIR}/tweak-wire/src/tweakwire.c
    ${TWEAKTOOL_DIR}/tweak-wire/src/tweakwire_nng.c
    ${TWEAKTOOL_DIR}/tweak2lib/src/tweak2.c)
endif()
