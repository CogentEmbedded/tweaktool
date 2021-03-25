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

tweak_variant tweak_pickle_from_pb_variant(const tweak_pb_value *src) {
  assert(src);
  tweak_variant result = {};
  switch (src->which_values) {
  case tweak_pb_value_is_null_tag:
    result.type = TWEAK_VARIANT_TYPE_IS_NULL;
    break;
  case tweak_pb_value_scalar_bool_tag:
    result.type = TWEAK_VARIANT_TYPE_BOOL;
    result.value_bool = src->values.scalar_bool;
    break;
  case tweak_pb_value_scalar_sint8_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT8;
    result.sint8 = src->values.scalar_sint8;
    break;
  case tweak_pb_value_scalar_sint16_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT16;
    result.sint16 = src->values.scalar_sint16;
    break;
  case tweak_pb_value_scalar_sint32_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT32;
    result.sint32 = src->values.scalar_sint32;
    break;
  case tweak_pb_value_scalar_sint64_tag:
    result.type = TWEAK_VARIANT_TYPE_SINT64;
    result.sint64 = src->values.scalar_sint64;
    break;
  case tweak_pb_value_scalar_uint8_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT8;
    result.uint8 = src->values.scalar_sint16;
    break;
  case tweak_pb_value_scalar_uint16_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT16;
    result.uint16 = src->values.scalar_uint16;
    break;
  case tweak_pb_value_scalar_uint32_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT32;
    result.uint32 = src->values.scalar_uint32;
    break;
  case tweak_pb_value_scalar_uint64_tag:
    result.type = TWEAK_VARIANT_TYPE_UINT32;
    result.uint32 = src->values.scalar_uint32;
    break;
  case tweak_pb_value_scalar_float_tag:
    result.type = TWEAK_VARIANT_TYPE_FLOAT;
    result.fp32 = src->values.scalar_float;
    break;
  case tweak_pb_value_scalar_double_tag:
    result.type = TWEAK_VARIANT_TYPE_DOUBLE;
    result.fp64 = src->values.scalar_double;
    break;
  default:
    TWEAK_LOG_WARN("tweak_pickle_from_pb_variant: Unknown tag %d", src->which_values);
    break;
  }
  return result;
}

tweak_pb_value tweak_pickle_to_pb_variant(const tweak_variant *src) {
  assert(src);
  tweak_pb_value result = {};
  switch (src->type) {
  case TWEAK_VARIANT_TYPE_IS_NULL:
    result.which_values = tweak_pb_value_is_null_tag;
    result.values.is_null = true;
    break;
  case TWEAK_VARIANT_TYPE_BOOL:
    result.which_values = tweak_pb_value_scalar_bool_tag;
    result.values.scalar_bool = src->value_bool;
    break;
  case TWEAK_VARIANT_TYPE_SINT8:
    result.which_values = tweak_pb_value_scalar_sint32_tag;
    result.values.scalar_sint8 = src->sint8;
    break;
  case TWEAK_VARIANT_TYPE_SINT16:
    result.which_values = tweak_pb_value_scalar_sint16_tag;
    result.values.scalar_sint16 = src->sint16;
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    result.which_values = tweak_pb_value_scalar_sint32_tag;
    result.values.scalar_sint32 = src->sint32;
    break;
  case TWEAK_VARIANT_TYPE_SINT64:
    result.which_values = tweak_pb_value_scalar_sint64_tag;
    result.values.scalar_sint64 = src->sint64;
    break;
  case TWEAK_VARIANT_TYPE_UINT8:
    result.which_values = tweak_pb_value_scalar_uint8_tag;
    result.values.scalar_uint8 = src->uint8;
    break;
  case TWEAK_VARIANT_TYPE_UINT16:
    result.which_values = tweak_pb_value_scalar_uint16_tag;
    result.values.scalar_uint16 = src->uint16;
    break;
  case TWEAK_VARIANT_TYPE_UINT32:
    result.which_values = tweak_pb_value_scalar_uint32_tag;
    result.values.scalar_uint32 = src->uint32;
    break;
  case TWEAK_VARIANT_TYPE_UINT64:
    result.which_values = tweak_pb_value_scalar_uint64_tag;
    result.values.scalar_uint64 = src->uint64;
    break;
  case TWEAK_VARIANT_TYPE_FLOAT:
    result.which_values = tweak_pb_value_scalar_float_tag;
    result.values.scalar_float = src->fp32;
    break;
  case TWEAK_VARIANT_TYPE_DOUBLE:
    result.which_values = tweak_pb_value_scalar_double_tag;
    result.values.scalar_double = src->fp64;
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
  pb_ostream_t sizestream = {};
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
