/**
 * @file tweakpickle_pb_util.h
 * @ingroup tweak-internal
 *
 * @brief RPC implementation over transport layer provided by
 * weak2::wire library, common data conversion helpers.
 *
 * @copyright 2020-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef TWEAK_PICKLE_PB_UTIL_INCLUDED
#define TWEAK_PICKLE_PB_UTIL_INCLUDED

#include <tweak2/log.h>

#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>

#define TRIGGER_EVENT(event, ...) ((event).callback(__VA_ARGS__, (event).cookie))

#if TWEAK_LOG_LEVEL == 0

void tweak_pickle_trace_add_item_req(const char* direction, const tweak_pickle_add_item *add_item);

void tweak_pickle_trace_change_item_req(const char* direction, const tweak_pickle_change_item *change_item);

void tweak_pickle_trace_remove_item_req(const char* direction, const tweak_pickle_remove_item *remove_item);

void tweak_pickle_trace_subscribe_req(const char* direction, const tweak_pickle_subscribe* subscribe);

void tweak_pickle_trace_announce_features_req(const char* direction, const tweak_pickle_features* features);

#else

#define tweak_pickle_trace_add_item_req(...)

#define tweak_pickle_trace_change_item_req(...)

#define tweak_pickle_trace_remove_item_req(...)

#define tweak_pickle_trace_subscribe_req(...)

#define tweak_pickle_trace_announce_features_req(...)

#endif

bool tweak_pickle_pb_is_scalar(tweak_pb_value *arg);

void tweak_pickle_pb_reserve_string(size_t length,
  tweak_variant_string *string, char **buff);

bool tweak_pickle_pb_decode_string(pb_istream_t *stream,
  const pb_field_t *field, void ** arg);

pb_callback_t tweak_pickle_pb_make_string_decode_callback(
  tweak_variant_string *arg);

bool tweak_pickle_pb_encode_string(pb_ostream_t *stream,
  const pb_field_t *field, void *const *arg);

pb_callback_t tweak_pickle_pb_make_string_encode_callback(
  const tweak_variant_string *arg);

pb_callback_t tweak_pickle_pb_make_variant_decode_callback(
  tweak_variant *arg);

tweak_variant tweak_pickle_pb_value_to_variant(tweak_pb_value *src);

tweak_pb_value tweak_pickle_pb_variant_to_value(const tweak_variant *src);

tweak_pickle_call_result tweak_pickle_send_message(tweak_wire_connection wire_connection,
  const pb_msgdesc_t *fields, const void *src_struct);

#endif
