/**
 * @file tweakpickle_pb_util.c
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
    " .tweak_id=%" PRId64 ","
    " .uri=\"%s\","
    " .description=\"%s\","
    " .meta=\"%s\","
    " .current_value=\"%s\","
    " .default_value=\"%s\","
    "}"
    , direction
    , add_item->tweak_id
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
    " .tweak_id=%" PRId64 ","
    " .value=\"%s\""
    " }"
    , direction
    , change_item->tweak_id
    , tweak_variant_string_c_str(&value_str));
    tweak_variant_destroy_string(&value_str);
}

void tweak_pickle_trace_remove_item_req(const char* direction, const tweak_pickle_remove_item *remove_item) {
  assert(direction);
  assert(remove_item);
  TWEAK_LOG_TRACE("%s request: remove_item"
    " {"
    " .tweak_id=%" PRId64 ","
    " }",
    direction,
    remove_item->tweak_id);
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

#endif

tweak_variant tweak_pickle_from_pb_variant(const tweak_pb_value *src) {
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
    result.value.sint8 = src->values.scalar_sint8;
    break;
  case tweak_pb_value_scalar_sint16_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT16;
    result.value.sint16 = src->values.scalar_sint16;
    break;
  case tweak_pb_value_scalar_sint32_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT32;
    result.value.sint32 = src->values.scalar_sint32;
    break;
  case tweak_pb_value_scalar_sint64_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT64;
    result.value.sint64 = src->values.scalar_sint64;
    break;
  case tweak_pb_value_scalar_uint8_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT8;
    result.value.uint8 = src->values.scalar_sint16;
    break;
  case tweak_pb_value_scalar_uint16_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT16;
    result.value.uint16 = src->values.scalar_uint16;
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
    TWEAK_LOG_WARN("tweak_pickle_from_pb_variant: Unknown tag %d", src->which_values);
    break;
  }
  return result;
}

tweak_pb_value tweak_pickle_to_pb_variant(const tweak_variant *src) {
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
  pb_ostream_t sizestream = { 0 };
  size_t datagram_size;
  pb_ostream_t stream;

  if (!pb_encode(&sizestream, fields, src_struct)) {
    goto error;
  }

  datagram_size = sizestream.bytes_written;

  /* Avoid heap traffic if possible, but airbag is included */
  buffer = datagram_size < sizeof(inline_buffer) ? inline_buffer
                                                 : calloc(1, datagram_size);

  if (!buffer) { /* heap alloc failed */
    goto error;
  }

  stream = pb_ostream_from_buffer(buffer, datagram_size);
  if (!pb_encode(&stream, fields, src_struct)) {
    goto error;
  }

  if (tweak_wire_transmit(wire_connection, buffer, datagram_size) !=
      TWEAK_WIRE_SUCCESS) {
    goto error;
  }

  result = TWEAK_PICKLE_SUCCESS;
error:
  if (buffer != inline_buffer) {
    free(buffer);
  }
  return result;
}
