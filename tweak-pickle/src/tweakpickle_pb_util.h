/**
 * @file tweakpickle_pb_util.h
 * @ingroup tweak-internal
 *
 * @brief RPC implementation over transport layer provided by
 * weak2::wire library, common data conversion helpers.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
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

#else

#define tweak_pickle_trace_add_item_req(...)

#define tweak_pickle_trace_change_item_req(...)

#define tweak_pickle_trace_remove_item_req(...)

#define tweak_pickle_trace_subscribe_req(...)

#endif

static inline void reserve_tweak_string(size_t length,
  tweak_variant_string *string, char **buff)
{
  assert(string);
  assert(buff);

  size_t desired_capacity = length + 1;
  if (desired_capacity <= sizeof(string->buffers.small_buffer)) {
    string->capacity = sizeof(string->buffers.small_buffer);
    string->length = length;
    *buff = string->buffers.small_buffer;
  } else {
    string->buffers.large_buffer = calloc(1, desired_capacity);
    assert(string->buffers.large_buffer);
    string->capacity = desired_capacity;
    string->length = length;
    *buff = string->buffers.large_buffer;
  }
}

static inline bool decode_tweak_string(pb_istream_t *stream,
  const pb_field_t *field, void ** arg)
{
  assert(stream);
  (void)field;
  assert(arg);

  tweak_variant_string *string = *(tweak_variant_string **)arg;

  char *buffer;
  reserve_tweak_string(stream->bytes_left, string, &buffer);

  if (!pb_read(stream, (pb_byte_t*)buffer, stream->bytes_left))
    return false;

  return true;
}

static inline pb_callback_t make_decode_callback_for_tweak_variant_string(
  tweak_variant_string *arg)
{
  assert(arg);
  pb_callback_t result = {
      .funcs = {
        .decode = &decode_tweak_string
      },
      .arg = arg};
  return result;
}

static inline bool encode_tweak_string(pb_ostream_t *stream,
  const pb_field_t *field, void *const *arg)
{
  assert(stream);
  assert(field);
  assert(arg);

  tweak_variant_string *string = *((tweak_variant_string *const *)arg);

  if (!pb_encode_tag_for_field(stream, field))
    return false;

  if (!pb_encode_string(stream, (const pb_byte_t *)tweak_variant_string_c_str(string),
                        string->length))
    return false;

  return true;
}

static inline pb_callback_t make_encode_callback_for_tweak_variant_string(
  const tweak_variant_string *arg)
{
  assert(arg);

  pb_callback_t result = {
      .funcs = {
        .encode = &encode_tweak_string
      },
      .arg = (void *)arg};
  return result;
}

tweak_variant tweak_pickle_from_pb_variant(const tweak_pb_value *src);

tweak_pb_value tweak_pickle_to_pb_variant(const tweak_variant *src);

tweak_pickle_call_result tweak_pickle_send_message(tweak_wire_connection wire_connection,
  const pb_msgdesc_t *fields, const void *src_struct);

#endif
