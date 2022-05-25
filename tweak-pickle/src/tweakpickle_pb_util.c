/**
 * @file tweakpickle_pb_util.c
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

#include <tweak2/pickle.h>
#include <tweak2/pickle_client.h>
#include <tweak2/wire.h>

#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "tweak.pb.h"
#include "tweakpickle_pb_util.h"

#include <stdbool.h>

#if TWEAK_LOG_LEVEL == 0

void tweak_pickle_trace_add_item_req(const char* direction, const tweak_pickle_add_item *add_item) {
  assert(direction);
  assert(add_item);
  tweak_variant_string current_value_str = tweak_variant_to_json(&add_item->current_value);
  tweak_variant_string default_value_str = tweak_variant_to_json(&add_item->default_value);
  TWEAK_LOG_TRACE("%s request: add_item"
    " {"
    " .id=%" PRId64 ","
    " .uri=\"%s\","
    " .description=\"%s\","
    " .meta=\"%s\","
    " .current_value=\"%s\","
    " .default_value=\"%s\","
    "}"
    , direction
    , add_item->id
    , tweak_variant_string_c_str(&add_item->uri)
    , tweak_variant_string_c_str(&add_item->description)
    , tweak_variant_string_c_str(&add_item->meta)
    , tweak_variant_string_c_str(&current_value_str)
    , tweak_variant_string_c_str(&default_value_str)
  );
  tweak_variant_destroy_string(&default_value_str);
  tweak_variant_destroy_string(&current_value_str);
}

void tweak_pickle_trace_change_item_req(const char* direction, const tweak_pickle_change_item *change_item) {
  assert(direction);
  assert(change_item);
  tweak_variant_string value_str = tweak_variant_to_json(&change_item->value);
  TWEAK_LOG_TRACE("%s request: change_item"
    " {"
    " .id=%" PRId64 ","
    " .value=\"%s\""
    " }"
    , direction
    , change_item->id
    , tweak_variant_string_c_str(&value_str));
    tweak_variant_destroy_string(&value_str);
}

void tweak_pickle_trace_remove_item_req(const char* direction, const tweak_pickle_remove_item *remove_item) {
  assert(direction);
  assert(remove_item);
  TWEAK_LOG_TRACE("%s request: remove_item"
    " {"
    " .id=%" PRId64 ","
    " }",
    direction,
    remove_item->id);
}

void tweak_pickle_trace_subscribe_req(const char* direction, const tweak_pickle_subscribe* subscribe) {
  assert(direction);
  TWEAK_LOG_TRACE("%s request: subscribe"
    " {"
    " .uri_patterns=\"%s\""
    " }",
    direction,
    subscribe ? tweak_variant_string_c_str(&subscribe->uri_patterns) : "*");
}

void tweak_pickle_trace_announce_features_req(const char* direction, const tweak_pickle_features* features) {
  assert(direction);
  TWEAK_LOG_TRACE("%s request: announce_features"
    " {"
    " .features=\"%s\""
    " }",
    direction,
    tweak_variant_string_c_str(&features->features));
}

#endif

bool tweak_pickle_pb_is_scalar(tweak_pb_value *arg) {
  switch (arg->which_values) {
  case tweak_pb_value_is_null_tag:
  case tweak_pb_value_scalar_bool_tag:
  case tweak_pb_value_scalar_sint8_tag:
  case tweak_pb_value_scalar_sint16_tag:
  case tweak_pb_value_scalar_sint32_tag:
  case tweak_pb_value_scalar_sint64_tag:
  case tweak_pb_value_scalar_uint8_tag:
  case tweak_pb_value_scalar_uint16_tag:
  case tweak_pb_value_scalar_uint32_tag:
  case tweak_pb_value_scalar_uint64_tag:
  case tweak_pb_value_scalar_float_tag:
  case tweak_pb_value_scalar_double_tag:
    return true;
  default:
    return false;
  }
}

tweak_variant tweak_pickle_pb_value_to_variant(tweak_pb_value *src) {
  assert(src);
  tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
  switch (src->which_values) {
  case tweak_pb_value_is_null_tag:
    result.type = TWEAK_VARIANT_TYPE_NULL;
    break;
  case tweak_pb_value_scalar_bool_tag:
    result.type = TWEAK_VARIANT_TYPE_BOOL;
    result.value.b = src->values.scalar_bool;
    break;
  case tweak_pb_value_scalar_sint8_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT8;
    result.value.sint8 = (int8_t)src->values.scalar_sint8;
    break;
  case tweak_pb_value_scalar_sint16_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT16;
    result.value.sint16 = (int16_t)src->values.scalar_sint16;
    break;
  case tweak_pb_value_scalar_sint32_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT32;
    result.value.sint32 = (int32_t)src->values.scalar_sint32;
    break;
  case tweak_pb_value_scalar_sint64_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT64;
    result.value.sint64 = src->values.scalar_sint64;
    break;
  case tweak_pb_value_scalar_uint8_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT8;
    result.value.uint8 = (uint8_t)src->values.scalar_sint16;
    break;
  case tweak_pb_value_scalar_uint16_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT16;
    result.value.uint16 = (uint16_t)src->values.scalar_uint16;
    break;
  case tweak_pb_value_scalar_uint32_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT32;
    result.value.uint32 = src->values.scalar_uint32;
    break;
  case tweak_pb_value_scalar_uint64_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT32;
    result.value.uint32 = src->values.scalar_uint32;
    break;
  case tweak_pb_value_scalar_float_tag:
    result.type = TWEAK_VARIANT_TYPE_FLOAT;
    result.value.fp32 = src->values.scalar_float;
    break;
  case tweak_pb_value_scalar_double_tag:
    result.type = TWEAK_VARIANT_TYPE_DOUBLE;
    result.value.fp64 = src->values.scalar_double;
    break;
  default:
    TWEAK_LOG_WARN("Unknown tag %d", src->which_values);
    break;
  }
  return result;
}

bool tweak_pickle_pb_encode_string(pb_ostream_t *stream,
                                   const pb_field_t *field, void *const *arg)
{
  assert(stream);
  assert(field);
  assert(arg);

  tweak_variant_string *string = *((tweak_variant_string *const *)arg);

  if (!pb_encode_tag_for_field(stream, field))
  {
    return false;
  }

  if (!pb_encode_string(stream, (const pb_byte_t *)tweak_variant_string_c_str(string),
                        string->length))
  {
    return false;
  }

  return true;
}

bool tweak_pickle_pb_decode_string(pb_istream_t *stream,
                                   const pb_field_t *field, void **arg)
{
  assert(stream);
  (void)field;
  assert(arg);

  tweak_variant_string *string = *(tweak_variant_string **)arg;

  size_t size = stream->bytes_left + 1;
  char *buffer;
  tweak_pickle_pb_reserve_string(size, string, &buffer);

  if (!pb_read(stream, (pb_byte_t *)buffer, stream->bytes_left))
  {
    return false;
  }
  buffer[size - 1] = '\0';

  return true;
}

static bool pb_variant_encode_string(pb_ostream_t *stream,
                                     const pb_field_t *field, void *const *arg)
{
  (void)field;

  const tweak_variant *variant = *(const tweak_variant *const *)arg;
  const tweak_variant_string *string = &variant->value.string;

  const char *data = tweak_variant_string_c_str(string);
  size_t size = string->length;

  if (!pb_encode_string(stream, (pb_byte_t *)data, size))
  {
    return false;
  }

  return true;
}

static bool pb_variant_decode_string(pb_istream_t *stream,
  const pb_field_t *field, tweak_variant_string *string)
{
    assert(stream);
    (void)field;

    pb_wire_type_t wire_type;
    bool eof;
    uint32_t tag;

    if (!pb_decode_tag(stream, &wire_type, &tag, &eof))
    {
      return false;
    }

    char small_buffer[TWEAK_VARIANT_INLINE_STRING_SIZE];
    char *tmp = small_buffer;
    size_t size = stream->bytes_left + 1;
    if (size > TWEAK_VARIANT_INLINE_STRING_SIZE)
    {
      tmp = calloc(1, size);
    }

    if (!pb_read(stream, (pb_byte_t *)tmp, stream->bytes_left))
    {
      return false;
    }
    tmp[size - 1] = '\0';

    tweak_assign_string(string, tmp);
    if (tmp != small_buffer)
    {
      free(tmp);
    }

    return true;
}

static bool pb_variant_encode_raw_buffer(pb_ostream_t *stream,
  const pb_field_t *field, void * const *arg)
{
  (void)field;
  tweak_variant *variant = *(tweak_variant *const *)arg;
  struct tweak_variant_buffer *buffer = &variant->value.buffer;

  const int8_t* data = tweak_buffer_get_data(buffer);
  size_t size = tweak_buffer_get_size(buffer);

  if (!pb_encode_string(stream, (pb_byte_t *)data, size))
  {
    return false;
  }

  return true;
}

static bool pb_variant_decode_raw_buffer(pb_istream_t *stream,
                                         const pb_field_t *field, struct tweak_variant_buffer *buffer)
{
  assert(stream);
  (void)field;

  pb_wire_type_t wire_type;
  bool eof;
  uint32_t tag;

  if (!pb_decode_tag(stream, &wire_type, &tag, &eof))
  {
    return false;
  }

  int8_t small_buffer[TWEAK_VARIANT_SMALL_BUFFER_SIZE];
  int8_t *tmp = small_buffer;
  size_t size = stream->bytes_left;
  if (buffer->size > TWEAK_VARIANT_SMALL_BUFFER_SIZE)
  {
    tmp = calloc(1, size);
  }

  if (!pb_read(stream, (pb_byte_t *)tmp, size))
  {
    return false;
  }

  *buffer = tweak_buffer_create(tmp, size);
  if (tmp != small_buffer)
  {
    free(tmp);
  }
  return true;
}

#define IMPLEMENT_BUFFER_ENCODER_DECODER(C_TYPE, SUFFIX, ENCODER, DECODER, TARGET_TYPE)           \
static bool pb_variant_encode_##SUFFIX##_buffer(pb_ostream_t *stream,                             \
  const pb_field_t *field, void *const *arg)                                                      \
{                                                                                                 \
  const tweak_variant *variant = *(const tweak_variant *const *)arg;                              \
  const struct tweak_variant_buffer *buffer = &variant->value.buffer;                             \
  const C_TYPE* data = tweak_buffer_get_data_const(buffer);                                       \
  size_t count = tweak_buffer_get_size(buffer) / sizeof(data[0]);                                 \
  for (size_t ix = 0; ix < count; ix++) {                                                         \
    if (!pb_encode_tag_for_field(stream, field))                                                  \
      return false;                                                                               \
                                                                                                  \
    TARGET_TYPE value = data[ix];                                                                 \
    if (!ENCODER(stream, &value))                                                                 \
      return false;                                                                               \
  }                                                                                               \
  return true;                                                                                    \
}                                                                                                 \
                                                                                                  \
static void append_##SUFFIX##_value_to_buffer(struct tweak_variant_buffer *target,                \
  C_TYPE value, size_t* count, size_t* capacity)                                                  \
{                                                                                                 \
  const size_t small_buffer_capacity = sizeof(target->buffers.small_buffer) / sizeof(value);      \
  C_TYPE* data = (C_TYPE*)tweak_buffer_get_data(target);                                          \
  size_t new_count = *count;                                                                      \
  size_t new_capacity = *capacity;                                                                \
  if (new_capacity == 0) {                                                                        \
    new_capacity = small_buffer_capacity;                                                         \
  }                                                                                               \
  ++new_count;                                                                                    \
  if (new_count > new_capacity) {                                                                 \
    new_capacity = new_capacity * 3 / 2;                                                          \
    if (*capacity <= small_buffer_capacity) {                                                     \
      uint8_t* large_buffer = malloc(sizeof(value) * new_capacity);                               \
      memcpy(large_buffer, target->buffers.small_buffer, target->size);                           \
      target->buffers.large_buffer = large_buffer;                                                \
    } else {                                                                                      \
      target->buffers.large_buffer =                                                              \
        realloc(target->buffers.large_buffer, sizeof(value) * new_capacity);                      \
      if (!target->buffers.large_buffer) {                                                        \
        TWEAK_FATAL("malloc() return NULL");                                                      \
      }                                                                                           \
    }                                                                                             \
    data = (C_TYPE*)target->buffers.large_buffer;                                                 \
  }                                                                                               \
  target->size = new_count * sizeof(value);                                                       \
  data[new_count - 1] = value;                                                                    \
  *count = new_count;                                                                             \
  *capacity = new_capacity;                                                                       \
}                                                                                                 \
                                                                                                  \
static bool pb_variant_decode_##SUFFIX##_buffer(pb_istream_t *stream,                             \
  const pb_field_t *field, struct tweak_variant_buffer *buffer)                                   \
{                                                                                                 \
  (void)field;                                                                                    \
  TARGET_TYPE value = 0;                                                                          \
  size_t count = 0;                                                                               \
  size_t capacity = 0;                                                                            \
  struct tweak_variant_buffer result = { 0 };                                                     \
  while (stream->bytes_left) {                                                                    \
    pb_wire_type_t wire_type;                                                                     \
    bool eof;                                                                                     \
    uint32_t tag;                                                                                 \
    if (!pb_decode_tag(stream, &wire_type, &tag, &eof))                                           \
      return false;                                                                               \
    if (!DECODER(stream, &value))                                                                 \
      return false;                                                                               \
    append_##SUFFIX##_value_to_buffer(&result, (C_TYPE)value, &count, &capacity);                 \
  }                                                                                               \
  tweak_buffer_swap(buffer, &result);                                                             \
  tweak_buffer_destroy(&result);                                                                  \
  return true;                                                                                    \
}

IMPLEMENT_BUFFER_ENCODER_DECODER(int16_t, sint16,  pb_encode_fixed32, pb_decode_fixed32, int32_t)
IMPLEMENT_BUFFER_ENCODER_DECODER(uint16_t, uint16, pb_encode_fixed32, pb_decode_fixed32, uint32_t)
IMPLEMENT_BUFFER_ENCODER_DECODER(int32_t, sint32, pb_encode_fixed32, pb_decode_fixed32, int32_t)
IMPLEMENT_BUFFER_ENCODER_DECODER(uint32_t, uint32, pb_encode_fixed32, pb_decode_fixed32, uint32_t)
IMPLEMENT_BUFFER_ENCODER_DECODER(int64_t, sint64, pb_encode_fixed64, pb_decode_fixed64, int64_t)
IMPLEMENT_BUFFER_ENCODER_DECODER(uint64_t, uint64, pb_encode_fixed64, pb_decode_fixed64, uint64_t)
IMPLEMENT_BUFFER_ENCODER_DECODER(float, float, pb_encode_fixed32, pb_decode_fixed32, float)
IMPLEMENT_BUFFER_ENCODER_DECODER(double, double, pb_encode_fixed64, pb_decode_fixed64, double)

tweak_pb_value tweak_pickle_pb_variant_to_value(const tweak_variant *src) {
  assert(src);
  tweak_pb_value result = tweak_pb_value_init_default;
  switch (src->type) {
  case TWEAK_VARIANT_TYPE_NULL:
    result.which_values = tweak_pb_value_is_null_tag;
    result.values.is_null = true;
    break;
  case TWEAK_VARIANT_TYPE_BOOL:
    result.which_values = tweak_pb_value_scalar_bool_tag;
    result.values.scalar_bool = src->value.b;
    break;
  case TWEAK_VARIANT_TYPE_SINT8:
    result.which_values = tweak_pb_value_scalar_sint32_tag;
    result.values.scalar_sint8 = src->value.sint8;
    break;
  case TWEAK_VARIANT_TYPE_SINT16:
    result.which_values = tweak_pb_value_scalar_sint16_tag;
    result.values.scalar_sint16 = src->value.sint16;
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    result.which_values = tweak_pb_value_scalar_sint32_tag;
    result.values.scalar_sint32 = src->value.sint32;
    break;
  case TWEAK_VARIANT_TYPE_SINT64:
    result.which_values = tweak_pb_value_scalar_sint64_tag;
    result.values.scalar_sint64 = src->value.sint64;
    break;
  case TWEAK_VARIANT_TYPE_UINT8:
    result.which_values = tweak_pb_value_scalar_uint8_tag;
    result.values.scalar_uint8 = src->value.uint8;
    break;
  case TWEAK_VARIANT_TYPE_UINT16:
    result.which_values = tweak_pb_value_scalar_uint16_tag;
    result.values.scalar_uint16 = src->value.uint16;
    break;
  case TWEAK_VARIANT_TYPE_UINT32:
    result.which_values = tweak_pb_value_scalar_uint32_tag;
    result.values.scalar_uint32 = src->value.uint32;
    break;
  case TWEAK_VARIANT_TYPE_UINT64:
    result.which_values = tweak_pb_value_scalar_uint64_tag;
    result.values.scalar_uint64 = src->value.uint64;
    break;
  case TWEAK_VARIANT_TYPE_FLOAT:
    result.which_values = tweak_pb_value_scalar_float_tag;
    result.values.scalar_float = src->value.fp32;
    break;
  case TWEAK_VARIANT_TYPE_DOUBLE:
    result.which_values = tweak_pb_value_scalar_double_tag;
    result.values.scalar_double = src->value.fp64;
    break;
  case TWEAK_VARIANT_TYPE_STRING:
    result.which_values = tweak_pb_value_string_tag;
    result.values.string.data.arg = (void*)src;
    result.values.string.data.funcs.encode = &pb_variant_encode_string;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    result.which_values = tweak_pb_value_sint8_buffer_tag;
    result.values.sint8_buffer.data.arg = (void*)src;
    result.values.sint8_buffer.data.funcs.encode = &pb_variant_encode_raw_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    result.which_values = tweak_pb_value_sint16_buffer_tag;
    result.values.sint16_buffer.buffer.arg = (void*)src;
    result.values.sint16_buffer.buffer.funcs.encode = &pb_variant_encode_sint16_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    result.which_values = tweak_pb_value_sint32_buffer_tag;
    result.values.sint32_buffer.buffer.arg = (void*)src;
    result.values.sint32_buffer.buffer.funcs.encode = &pb_variant_encode_sint32_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    result.which_values = tweak_pb_value_sint64_buffer_tag;
    result.values.sint64_buffer.buffer.arg = (void*)src;
    result.values.sint64_buffer.buffer.funcs.encode = &pb_variant_encode_sint64_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    result.which_values = tweak_pb_value_uint8_buffer_tag;
    result.values.uint8_buffer.data.arg = (void*)src;
    result.values.uint8_buffer.data.funcs.encode = &pb_variant_encode_raw_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    result.which_values = tweak_pb_value_uint16_buffer_tag;
    result.values.uint16_buffer.buffer.arg = (void*)src;
    result.values.uint16_buffer.buffer.funcs.encode = &pb_variant_encode_uint16_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    result.which_values = tweak_pb_value_uint32_buffer_tag;
    result.values.uint32_buffer.buffer.arg = (void*)src;
    result.values.uint32_buffer.buffer.funcs.encode = &pb_variant_encode_uint32_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    result.which_values = tweak_pb_value_uint64_buffer_tag;
    result.values.uint64_buffer.buffer.arg = (void*)src;
    result.values.uint64_buffer.buffer.funcs.encode = &pb_variant_encode_uint64_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    result.which_values = tweak_pb_value_fp32_buffer_tag;
    result.values.fp32_buffer.buffer.arg = (void*)src;
    result.values.fp32_buffer.buffer.funcs.encode = &pb_variant_encode_float_buffer;
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    result.which_values = tweak_pb_value_fp64_buffer_tag;
    result.values.fp64_buffer.buffer.arg = (void*)src;
    result.values.fp64_buffer.buffer.funcs.encode = &pb_variant_encode_double_buffer;
    break;
  default:
    TWEAK_LOG_WARN("%s: Unknown tag %d", __func__, src->type);
  }
  return result;
}

enum { DEFAULT_ENCODE_BUFFER_SIZE = 4 * (1 << 10) };

tweak_pickle_call_result tweak_pickle_send_message(
    tweak_wire_connection wire_connection, const pb_msgdesc_t *fields,
    const void *src_struct)
{
  assert(wire_connection);
  assert(fields);
  assert(src_struct);

  tweak_pickle_call_result result = TWEAK_PICKLE_REMOTE_ERROR;

  uint8_t inline_buffer[DEFAULT_ENCODE_BUFFER_SIZE];
  uint8_t *buffer = NULL;

  /* Estimate size first */
  pb_ostream_t sizestream = {0};
  size_t datagram_size;
  pb_ostream_t stream;

  if (!pb_encode(&sizestream, fields, src_struct))
  {
    goto error;
  }

  datagram_size = sizestream.bytes_written;

  /* Avoid heap traffic if possible, but airbag is included */
  buffer = datagram_size < sizeof(inline_buffer) ? inline_buffer
                                                 : calloc(1, datagram_size);

  if (!buffer)
  { /* heap alloc failed */
    goto error;
  }

  stream = pb_ostream_from_buffer(buffer, datagram_size);
  if (!pb_encode(&stream, fields, src_struct))
  {
    goto error;
  }

  if (tweak_wire_transmit(wire_connection, buffer, datagram_size) !=
      TWEAK_WIRE_SUCCESS)
  {
    goto error;
  }

  result = TWEAK_PICKLE_SUCCESS;

error:
  if (buffer != inline_buffer)
  {
    free(buffer);
  }
  return result;
}

void tweak_pickle_pb_reserve_string(size_t length,
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

pb_callback_t tweak_pickle_pb_make_string_decode_callback(
  tweak_variant_string *arg)
{
  assert(arg);
  pb_callback_t result = {
      .funcs = {
        .decode = &tweak_pickle_pb_decode_string
      },
      .arg = arg};
  return result;
}

pb_callback_t tweak_pickle_pb_make_string_encode_callback(
  const tweak_variant_string *arg)
{
  assert(arg);

  pb_callback_t result = {
      .funcs = {
        .encode = &tweak_pickle_pb_encode_string
      },
      .arg = (void *)arg};
  return result;
}

static bool tweak_pickle_pb_decode_variant(pb_istream_t *stream,
  const pb_field_t *field, void ** arg)
{
  (void)stream;
  (void)arg;
  uint32_t tag = field->tag;
  bool rv = false;
  tweak_variant *variant = *(tweak_variant**)arg;
  switch (tag) {
  case tweak_pb_value_string_tag:
    variant->type = TWEAK_VARIANT_TYPE_STRING;
    rv = pb_variant_decode_string(stream, field, &variant->value.string);
    break;
  case tweak_pb_value_sint8_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_SINT8;
    rv = pb_variant_decode_raw_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_sint16_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_SINT16;
    rv = pb_variant_decode_sint16_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_sint32_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_SINT32;
    rv = pb_variant_decode_sint32_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_sint64_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_SINT64;
    rv = pb_variant_decode_sint64_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_uint8_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_UINT8;
    rv = pb_variant_decode_raw_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_uint16_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_UINT16;
    rv = pb_variant_decode_uint16_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_uint32_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_UINT32;
    rv = pb_variant_decode_uint32_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_uint64_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_UINT64;
    rv = pb_variant_decode_uint64_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_fp32_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_FLOAT;
    rv = pb_variant_decode_float_buffer(stream, field, &variant->value.buffer);
    break;
  case tweak_pb_value_fp64_buffer_tag:
    variant->type = TWEAK_VARIANT_TYPE_VECTOR_DOUBLE;
    rv = pb_variant_decode_double_buffer(stream, field, &variant->value.buffer);
    break;
  default:
    TWEAK_LOG_WARN("Unknown message tag : %u", tag);
    rv = false;
    break;
  }
  return rv;
}

pb_callback_t tweak_pickle_pb_make_variant_decode_callback(
  tweak_variant *arg)
{
  assert(arg);
  pb_callback_t result = {
      .funcs = {
        .decode = &tweak_pickle_pb_decode_variant
      },
      .arg = arg
  };
  return result;
}
