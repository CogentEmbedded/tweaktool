/**
 * @file tweakpickle_server_pb.c
 * @ingroup tweak-internal
 *
 * @brief RPC implementation over transport layer provided by
 * weak2::wire library, server endpoint.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/pickle.h>
#include <tweak2/pickle_server.h>
#include <tweak2/wire.h>

#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "tweak.pb.h"
#include "tweakpickle_pb_util.h"

#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>

struct tweak_pickle_endpoint_server_impl {
  struct tweak_pickle_endpoint_base base;
  tweak_wire_connection wire_connection;
  tweak_pickle_server_skeleton skeleton;
};

static void server_connection_state_listener(tweak_wire_connection connection,
  tweak_wire_connection_state connection_state, void* arg);

static void server_receive_listener(const uint8_t *buffer, size_t size, void* arg);

tweak_pickle_server_endpoint tweak_pickle_create_server_endpoint(
  const tweak_pickle_server_descriptor* server_descriptor)
{
  if (!server_descriptor) {
    TWEAK_LOG_ERROR("server_descriptor is NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  TWEAK_LOG_TRACE_ENTRY("server_descriptor={.connection_type=\"%s\", .params=\"%s\", .uri=\"%s\","
    " .skeleton={ ... }}",
    server_descriptor->context_type, server_descriptor->params, server_descriptor->uri);

  if (!server_descriptor->skeleton.connection_state_listener.callback) {
    TWEAK_LOG_ERROR("server_descriptor->skeleton.connection_state_listener.callback");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  if (!server_descriptor->skeleton.subscribe_listener.callback) {
    TWEAK_LOG_ERROR("server_descriptor->skeleton.subscribe_listener.callback");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  struct tweak_pickle_endpoint_server_impl* endpoint_server_impl =
    calloc(1, sizeof(*endpoint_server_impl));

  if (!endpoint_server_impl) {
    TWEAK_LOG_ERROR("calloc() returned NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  endpoint_server_impl->skeleton = server_descriptor->skeleton;

  endpoint_server_impl->wire_connection =
    tweak_wire_create_connection(server_descriptor->context_type,
    server_descriptor->params, server_descriptor->uri,
    &server_connection_state_listener, endpoint_server_impl,
    &server_receive_listener, endpoint_server_impl);

  if (endpoint_server_impl->wire_connection == TWEAK_WIRE_INVALID_CONNECTION) {
    TWEAK_LOG_ERROR("tweak_wire_create_connection() returned TWEAK_WIRE_INVALID_CONNECTION");
    free(endpoint_server_impl);
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  return &endpoint_server_impl->base;
}

static void server_connection_state_listener(tweak_wire_connection connection,
  tweak_wire_connection_state connection_state, void* arg)
{
  TWEAK_LOG_TRACE_ENTRY("connection = %p, connection_state = 0x%X, arg = %p",
    connection, connection_state, arg);
  (void)connection;
  struct tweak_pickle_endpoint_server_impl* endpoint =
    (struct tweak_pickle_endpoint_server_impl*)arg;

  switch (connection_state) {
  case TWEAK_WIRE_DISCONNECTED:
    TRIGGER_EVENT(endpoint->skeleton.connection_state_listener, TWEAK_PICKLE_DISCONNECTED);
    break;
  case TWEAK_WIRE_CONNECTED:
    TRIGGER_EVENT(endpoint->skeleton.connection_state_listener, TWEAK_PICKLE_CONNECTED);
    break;
  default:
    TWEAK_FATAL("Inside client_connection_state_listener: Unknown enumeration value: %d",
      connection_state);
  }
}

struct decoded_client_node_message {
  pb_size_t tag;

  union {
    tweak_pickle_subscribe subscribe;

    tweak_pickle_change_item change_item;
  } body;
};

static bool decode_client_subscribe_request(pb_istream_t *stream, tweak_pb_subscribe *subscribe,
  struct decoded_client_node_message* decoded_client_node_message)
{
  TWEAK_LOG_TRACE_ENTRY("stream = %p, subscribe = %p, decoded_client_node_message = %p",
    stream, subscribe, decoded_client_node_message);
  subscribe->uri_patterns = make_decode_callback_for_tweak_variant_string(
    &decoded_client_node_message->body.subscribe.uri_patterns
  );
  if (!pb_decode(stream, tweak_pb_subscribe_fields, subscribe)) {
    tweak_variant_destroy_string(&decoded_client_node_message->body.subscribe.uri_patterns);
    TWEAK_LOG_WARN("Can't decode inbound subscribe request");
    return false;
  }
  return true;
}

static bool decode_client_change_item_request(pb_istream_t *stream, tweak_pb_change_item *change_item,
  struct decoded_client_node_message* decoded_client_node_message)
{
  TWEAK_LOG_TRACE_ENTRY("stream = %p, change_item = %p, decoded_client_node_message = %p",
    stream, change_item, decoded_client_node_message);
  if (!pb_decode(stream, tweak_pb_change_item_fields, change_item)) {
    TWEAK_LOG_WARN("Can't decode inbound change item request");
    return false;
  }

  decoded_client_node_message->body.change_item.tweak_id = change_item->tweak_id;
  decoded_client_node_message->body.change_item.value = tweak_pickle_from_pb_variant(&change_item->value);
  return true;
}

static bool decode_client_request(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  TWEAK_LOG_TRACE_ENTRY("stream = %p, change_item = %p, arg = %p",
    stream, field, arg);
  struct decoded_client_node_message* decoded_client_node_message =
    *(struct decoded_client_node_message **)arg;
  decoded_client_node_message->tag = field->tag;
  bool rv;
  switch (decoded_client_node_message->tag) {
  case tweak_pb_client_node_message_subscribe_tag:
    rv = decode_client_subscribe_request(stream, (tweak_pb_subscribe *)field->pData,
      decoded_client_node_message);
    break;
  case tweak_pb_client_node_message_change_item_tag:
    rv = decode_client_change_item_request(stream, (tweak_pb_change_item *)field->pData,
      decoded_client_node_message);
    break;
  default:
    TWEAK_LOG_WARN("Unknown tag: %d", decoded_client_node_message->tag);
    rv = false;
    break;
  }
  return rv;
}

static void server_receive_listener(const uint8_t *buffer, size_t size,
  void* arg)
{
  TWEAK_LOG_TRACE_ENTRY("buffer = %p, size = 0x%X, arg=%p",
    buffer, size, arg);
  struct tweak_pickle_endpoint_server_impl* endpoint =
    (struct tweak_pickle_endpoint_server_impl*)arg;

  pb_istream_t stream = pb_istream_from_buffer(buffer, size);
  struct decoded_client_node_message decoded_client_node_message = { 0 };

  tweak_pb_client_node_message message = {
    .cb_request = {
      .funcs = {
        .decode = &decode_client_request
      },
      .arg = &decoded_client_node_message
    }
  };

  if (pb_decode(&stream, tweak_pb_client_node_message_fields, &message)) {
    switch(decoded_client_node_message.tag) {
    case tweak_pb_client_node_message_subscribe_tag:
      tweak_pickle_trace_subscribe_req("Inbound", &decoded_client_node_message.body.subscribe);
      TRIGGER_EVENT(endpoint->skeleton.subscribe_listener, &decoded_client_node_message.body.subscribe);
      break;
    case tweak_pb_client_node_message_change_item_tag:
      tweak_pickle_trace_change_item_req("Inbound", &decoded_client_node_message.body.change_item);
      TRIGGER_EVENT(endpoint->skeleton.change_item_listener, &decoded_client_node_message.body.change_item);
      tweak_variant_destroy(&decoded_client_node_message.body.change_item.value);
      break;
    default:
      TWEAK_LOG_WARN("Should never happen.\n"
        "Unrecognized tags should cause error in pb_decode()\n"
        "If it hasn't, then error is in nanopb");
      break;
    }
  } else {
    TWEAK_LOG_ERROR("Unrecognized datagram in server_receive_listener");
  }
}

static tweak_pickle_call_result
  encode_and_transmit_server_message(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pb_server_node_message* message)
{
  TWEAK_LOG_TRACE_ENTRY("server_endpoint = %p, message=%p",
    server_endpoint, message);
  struct tweak_pickle_endpoint_server_impl* endpoint_server_impl =
    (struct tweak_pickle_endpoint_server_impl*)server_endpoint;

  tweak_pickle_call_result result =
    tweak_pickle_send_message(endpoint_server_impl->wire_connection,
    tweak_pb_server_node_message_fields, message);

  TWEAK_LOG_TRACE("tweak_pickle_send_message returned %d", result);
  return result;
}

tweak_pickle_call_result
  tweak_pickle_server_add_item(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pickle_add_item *add_item)
{
  TWEAK_LOG_TRACE_ENTRY("server_endpoint = %p, add_item=%p",
    server_endpoint, add_item);
  if (!add_item) {
    TWEAK_LOG_ERROR("add_item is NULL");
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  if (add_item->tweak_id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("add_item->tweak_id == TWEAK_INVALID_ID");
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  if (tweak_variant_string_is_empty(&add_item->uri)) {
    TWEAK_LOG_ERROR("add_item->uri is empty");
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  if (add_item->current_value.type == TWEAK_VARIANT_TYPE_NULL) {
    TWEAK_LOG_ERROR("add_item->current_value.type == TWEAK_VARIANT_TYPE_NULL");
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  bool has_default_value = add_item->default_value.type != TWEAK_VARIANT_TYPE_NULL;
  tweak_pb_value default_value = tweak_pb_value_init_default;
  if (has_default_value) {
    default_value = tweak_pickle_to_pb_variant(&add_item->default_value);
  }
  tweak_pb_server_node_message message = {
    .which_request = tweak_pb_server_node_message_add_item_tag,
    .request = {
      .add_item = {
        .tweak_id = add_item->tweak_id,
        .uri = make_encode_callback_for_tweak_variant_string(&add_item->uri),
        .description = make_encode_callback_for_tweak_variant_string(&add_item->description),
        .meta = make_encode_callback_for_tweak_variant_string(&add_item->meta),
        .has_default_value = has_default_value,
        .default_value = default_value,
        .has_current_value = true,
        .current_value = tweak_pickle_to_pb_variant(&add_item->current_value)
      }
    }
  };
  tweak_pickle_trace_add_item_req("Outbound", add_item);
  tweak_pickle_call_result result = encode_and_transmit_server_message(server_endpoint, &message);
  if (result == TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_TRACE("Server message has been encoded and transmitted");
  } else {
    TWEAK_LOG_TRACE("Server message hasn't been delivered");
  }
  return result;
}

tweak_pickle_call_result
  tweak_pickle_server_change_item(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pickle_change_item *change)
{
  TWEAK_LOG_TRACE_ENTRY("server_endpoint = %p, change = %p",
    server_endpoint, change);
  if (!change) {
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  if (change->value.type == TWEAK_VARIANT_TYPE_NULL) {
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  tweak_pb_server_node_message message = {
    .which_request = tweak_pb_server_node_message_change_item_tag,
    .request = {
      .change_item = {
        .tweak_id = change->tweak_id,
        .has_value = true,
        .value = tweak_pickle_to_pb_variant(&change->value)
      }
    }
  };
  tweak_pickle_trace_change_item_req("Outbound", change);
  tweak_pickle_call_result result = encode_and_transmit_server_message(server_endpoint, &message);
  if (result == TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_TRACE("Server message has been encoded and transmitted");
  } else {
    TWEAK_LOG_TRACE("Server message hasn't been delivered");
  }
  return result;
}

tweak_pickle_call_result
  tweak_pickle_server_remove_item(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pickle_remove_item* remove_item)
{
  TWEAK_LOG_TRACE("Enter");
  tweak_pb_server_node_message message = {
    .which_request = tweak_pb_server_node_message_remove_item_tag,
    .request = {
      .remove_item = {
        .tweak_id = remove_item->tweak_id
      }
    }
  };

  tweak_pickle_trace_remove_item_req("Outbound", remove_item);
  tweak_pickle_call_result result = encode_and_transmit_server_message(server_endpoint, &message);
  if (result == TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_TRACE("Server message has been encoded and transmitted");
  } else {
    TWEAK_LOG_TRACE("Server message hasn't been delivered");
  }
  return result;
}

void tweak_pickle_destroy_server_endpoint(
    tweak_pickle_server_endpoint server_endpoint)
{
  TWEAK_LOG_TRACE_ENTRY("server_endpoint = %p", server_endpoint);
  struct tweak_pickle_endpoint_server_impl* endpoint_server_impl =
    (struct tweak_pickle_endpoint_server_impl*) server_endpoint;

  TRIGGER_EVENT(endpoint_server_impl->skeleton.connection_state_listener, TWEAK_PICKLE_DISCONNECTED);

  tweak_wire_destroy_connection(endpoint_server_impl->wire_connection);
  endpoint_server_impl->wire_connection = NULL;
  free(server_endpoint);
}
