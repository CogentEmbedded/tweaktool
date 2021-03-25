/**
 * @file tweakpickle_client_pb.c
 * @ingroup tweak-internal
 *
 * @brief RPC implementation over transport layer provided by
 * weak2::wire library, client endpoint.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>
#include <tweak2/pickle.h>
#include <tweak2/pickle_client.h>
#include <tweak2/wire.h>

#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "tweak.pb.h"
#include "tweakpickle_pb_util.h"

#include <stdlib.h>
#include <inttypes.h>

struct tweak_pickle_endpoint_client_impl {
  struct tweak_pickle_endpoint_base base;
  tweak_wire_connection wire_connection;
  tweak_pickle_client_skeleton skeleton;
};

static void client_connection_state_listener(tweak_wire_connection connection,
  tweak_wire_connection_state connection_state, void* arg);

static void client_receive_listener(const uint8_t* buffer, size_t size, void* arg);

tweak_pickle_client_endpoint tweak_pickle_create_client_endpoint(
  const tweak_pickle_client_descriptor* client_descriptor)
{
  if (!client_descriptor) {
    TWEAK_LOG_ERROR("client_descriptor is NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  TWEAK_LOG_TRACE_ENTRY("client_descriptor={.connection_type=\"%s\", .params=\"%s\", .uri=\"%s\","
    " .skeleton={ ... }}",
    client_descriptor->context_type, client_descriptor->params, client_descriptor->uri);

  if (!client_descriptor->skeleton.add_item_listener.callback) {
    TWEAK_LOG_ERROR("client_descriptor->skeleton.add_item_listener.callback is NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  if (!client_descriptor->skeleton.change_item_listener.callback) {
    TWEAK_LOG_ERROR("client_descriptor->skeleton.change_item_listener.callback is NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  if (!client_descriptor->skeleton.remove_item_listener.callback) {
    TWEAK_LOG_ERROR("client_descriptor->skeleton.remove_item_listener.callback is NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  if (!client_descriptor->skeleton.connection_state_listener.callback) {
    TWEAK_LOG_ERROR("client_descriptor->skeleton.connection_state_listener.callback is NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  struct tweak_pickle_endpoint_client_impl* endpoint_client_impl =
    calloc(1, sizeof(*endpoint_client_impl));
  if (!endpoint_client_impl) {
    TWEAK_LOG_ERROR("calloc() returned NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  endpoint_client_impl->skeleton = client_descriptor->skeleton;

  endpoint_client_impl->wire_connection =
    tweak_wire_create_connection(client_descriptor->context_type,
    client_descriptor->params, client_descriptor->uri,
    &client_connection_state_listener, endpoint_client_impl,
    &client_receive_listener, endpoint_client_impl);

  if (endpoint_client_impl->wire_connection == TWEAK_WIRE_INVALID_CONNECTION) {
    free(endpoint_client_impl);
    TWEAK_LOG_ERROR("tweak_wire_create_connection() returned NULL");
    return TWEAK_PICKLE_INVALID_ENDPOINT;
  }

  return &endpoint_client_impl->base;
}

static void client_connection_state_listener(tweak_wire_connection connection,
  tweak_wire_connection_state connection_state, void* arg)
{
  TWEAK_LOG_TRACE_ENTRY("connection = %p, connection_state = %d, arg=%p",
    connection, connection_state, arg);
  (void)connection;
  struct tweak_pickle_endpoint_client_impl* endpoint =
    (struct tweak_pickle_endpoint_client_impl*)arg;

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

struct decoded_server_node_message {
  pb_size_t tag;

  union {
    tweak_pickle_add_item add_item;

    tweak_pickle_change_item change_item;

    tweak_pickle_remove_item remove_item;
  };
};

static bool decode_server_request(pb_istream_t *stream, const pb_field_t *field,
  void **arg);

static void client_receive_listener(const uint8_t* buffer,
  size_t size, void* arg)
{
  TWEAK_LOG_TRACE_ENTRY("buffer = %p, size = %u, arg=%p",
    buffer, size, arg);
  struct tweak_pickle_endpoint_client_impl* endpoint =
    (struct tweak_pickle_endpoint_client_impl*)arg;

  pb_istream_t stream = pb_istream_from_buffer(buffer, size);

  struct decoded_server_node_message decoded_server_node_message = {};

  tweak_pb_server_node_message message = {
    .cb_request = {
      .funcs = {
        .decode = &decode_server_request
      },
      .arg = &decoded_server_node_message
    }
  };

  if (pb_decode(&stream, tweak_pb_server_node_message_fields, &message)) {
    switch (decoded_server_node_message.tag) {
    case tweak_pb_server_node_message_add_item_tag:
      trace_add_item_req("Inbound", &decoded_server_node_message.add_item);
      TRIGGER_EVENT(endpoint->skeleton.add_item_listener, &decoded_server_node_message.add_item);
      tweak_variant_destroy_string(&decoded_server_node_message.add_item.uri);
      tweak_variant_destroy_string(&decoded_server_node_message.add_item.meta);
      tweak_variant_destroy_string(&decoded_server_node_message.add_item.description);
      tweak_variant_destroy(&decoded_server_node_message.add_item.default_value);
      tweak_variant_destroy(&decoded_server_node_message.add_item.current_value);
      break;
    case tweak_pb_server_node_message_change_item_tag:
      trace_change_item_req("Inbound", &decoded_server_node_message.change_item);
      TRIGGER_EVENT(endpoint->skeleton.change_item_listener, &decoded_server_node_message.change_item);
      tweak_variant_destroy(&decoded_server_node_message.change_item.value);
      break;
    case tweak_pb_server_node_message_remove_item_tag:
      trace_remove_item_req("Inbound", &decoded_server_node_message.remove_item);
      TRIGGER_EVENT(endpoint->skeleton.remove_item_listener, &decoded_server_node_message.remove_item);
      break;
    default:
      TWEAK_LOG_WARN("Should never happen.\n"
                     "Unrecognized tags should cause error in pb_decode()\n"
                     "If it hasn't, then error is in nanopb");
    }
  } else {
    TWEAK_LOG_WARN("Unrecognized datagram in client_receive_listener");
  }
}

static bool decode_server_add_item_request(pb_istream_t *stream,
  tweak_pb_add_item *add_item,
  struct decoded_server_node_message *decoded_server_node_message)
{
  TWEAK_LOG_TRACE_ENTRY("stream = %p, add_item = %p, decoded_server_node_message = %p",
    stream, add_item, decoded_server_node_message);
  add_item->uri = make_decode_callback_for_tweak_variant_string(
    &decoded_server_node_message->add_item.uri
  );
  add_item->description = make_decode_callback_for_tweak_variant_string(
    &decoded_server_node_message->add_item.description
  );
  add_item->meta = make_decode_callback_for_tweak_variant_string(
    &decoded_server_node_message->add_item.meta
  );
  if (!pb_decode(stream, tweak_pb_add_item_fields, add_item)) {
    TWEAK_LOG_WARN("Can't decode add_item request");
    tweak_variant_destroy_string(&decoded_server_node_message->add_item.uri);
    tweak_variant_destroy_string(&decoded_server_node_message->add_item.meta);
    tweak_variant_destroy_string(&decoded_server_node_message->add_item.description);
    tweak_variant_destroy(&decoded_server_node_message->add_item.default_value);
    tweak_variant_destroy(&decoded_server_node_message->add_item.current_value);
    return false;
  }

  decoded_server_node_message->add_item.tweak_id = add_item->tweak_id;
  decoded_server_node_message->add_item.current_value = tweak_pickle_from_pb_variant(&add_item->current_value);
  decoded_server_node_message->add_item.default_value = tweak_pickle_from_pb_variant(&add_item->default_value);
  return true;
}

static bool decode_server_change_item_request(pb_istream_t *stream,
  tweak_pb_change_item *change_item,
  struct decoded_server_node_message *decoded_server_node_message)
{
  TWEAK_LOG_TRACE_ENTRY("stream = %p, add_item = %p, decoded_server_node_message = %p",
    stream, change_item, decoded_server_node_message);
  if (!pb_decode(stream, tweak_pb_change_item_fields, change_item)) {
    TWEAK_LOG_WARN("Can't decode change_item request");
    return false;
  }

  decoded_server_node_message->change_item.tweak_id = change_item->tweak_id;
  decoded_server_node_message->change_item.value = tweak_pickle_from_pb_variant(&change_item->value);
  return true;
}

static bool decode_server_remove_item_request(pb_istream_t *stream,
  tweak_pb_remove_item *remove_item,
  struct decoded_server_node_message *decoded_server_node_message)
{
  TWEAK_LOG_TRACE_ENTRY("stream = %p, remove_item = %p, decoded_server_node_message = %p",
    stream, remove_item, decoded_server_node_message);
  if (!pb_decode(stream, tweak_pb_remove_item_fields, remove_item)) {
    TWEAK_LOG_WARN("Can't decode remove_item request");
    return false;
  }

  decoded_server_node_message->remove_item.tweak_id = remove_item->tweak_id;
  return true;
}

static bool decode_server_request(pb_istream_t *stream, const pb_field_t *field,
  void **arg)
{
  TWEAK_LOG_TRACE_ENTRY("stream = %p, field = %p, arg = %p", stream, field, arg);
  struct decoded_server_node_message *decoded_server_node_message =
    *(struct decoded_server_node_message **)arg;

  decoded_server_node_message->tag = field->tag;
  bool rv;
  switch (decoded_server_node_message->tag) {
  case tweak_pb_server_node_message_add_item_tag:
    TWEAK_LOG_TRACE("Invoking decode_server_add_item_request");
    rv = decode_server_add_item_request(stream,
      (tweak_pb_add_item *)field->pData, decoded_server_node_message);
    break;
  case tweak_pb_server_node_message_change_item_tag:
    TWEAK_LOG_TRACE("Invoking decode_server_change_item_request");
    rv = decode_server_change_item_request(stream,
      (tweak_pb_change_item *)field->pData, decoded_server_node_message);
    break;
  case tweak_pb_server_node_message_remove_item_tag:
    TWEAK_LOG_TRACE("Invoking decode_server_remove_item_request");
    rv = decode_server_remove_item_request(stream,
      (tweak_pb_remove_item *)field->pData, decoded_server_node_message);
    break;
  default:
    TWEAK_LOG_WARN("Unknown message tag : %d", decoded_server_node_message->tag);
    rv = false;
    break;
  }
  return rv;
}

static tweak_pickle_call_result
  encode_and_transmit_client_message(tweak_pickle_client_endpoint client_endpoint,
    const tweak_pb_client_node_message* message)
{
  TWEAK_LOG_TRACE_ENTRY("client_endpoint = %p, message = %p", client_endpoint, message);
  struct tweak_pickle_endpoint_client_impl* endpoint_client_impl =
    (struct tweak_pickle_endpoint_client_impl*)client_endpoint;

  tweak_pickle_call_result result = tweak_pickle_send_message(endpoint_client_impl->wire_connection,
    tweak_pb_client_node_message_fields, message);

  TWEAK_LOG_TRACE("tweak_pickle_send_message returned %d", result);

  return result;
}

tweak_pickle_call_result
  tweak_pickle_client_subscribe(tweak_pickle_client_endpoint client_endpoint,
    const tweak_pickle_subscribe* subscribe)
{
  TWEAK_LOG_TRACE_ENTRY("client_endpoint = %p, subscribe = %p", client_endpoint, subscribe);

  tweak_variant_string uri_patterns = {};
  tweak_variant_assign_string(&uri_patterns,
    subscribe
      ? tweak_variant_string_c_str(&subscribe->uri_patterns)
      : "*");

  tweak_pb_client_node_message message = {
    .which_request = tweak_pb_client_node_message_subscribe_tag,
    .request = {
      .subscribe = {
        .uri_patterns = make_encode_callback_for_tweak_variant_string(&uri_patterns)
      }
    }
  };
  trace_subscribe_req("Outbound", subscribe);

  tweak_pickle_call_result result = encode_and_transmit_client_message(client_endpoint, &message);
  tweak_variant_destroy_string(&uri_patterns);

  if (result == TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_TRACE("Change request has been encoded & transmitted");
  } else {
    TWEAK_LOG_TRACE("Change request hasn't been sent. Error code: %d", result);
  }

  return result;
}

tweak_pickle_call_result
  tweak_pickle_client_change_item(tweak_pickle_client_endpoint client_endpoint,
                                  const tweak_pickle_change_item *change)
{
  TWEAK_LOG_TRACE_ENTRY("client_endpoint = %p, subscribe = %p", client_endpoint, change);
  if (!change) {
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  if (change->value.type == TWEAK_VARIANT_TYPE_IS_NULL) {
    return TWEAK_PICKLE_BAD_PARAMETER;
  }

  tweak_pb_client_node_message message = {
    .which_request = tweak_pb_client_node_message_change_item_tag,
    .request = {
      .change_item = {
        .tweak_id = change->tweak_id,
        .has_value = true,
        .value = tweak_pickle_to_pb_variant(&change->value)
      }
    }
  };

  trace_change_item_req("Outbound", change);

  tweak_pickle_call_result result =
    encode_and_transmit_client_message(client_endpoint, &message);

  if (result == TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_TRACE("Change request has been encoded & transmitted");
  } else {
    TWEAK_LOG_TRACE("Change request hasn't been sent. Error code: %d", result);
  }

  return result;
}

void tweak_pickle_destroy_client_endpoint(
  tweak_pickle_client_endpoint client_endpoint)
{
  TWEAK_LOG_TRACE_ENTRY("client_endpoint = %p", client_endpoint);
  struct tweak_pickle_endpoint_client_impl* client_endpoint_impl =
    (struct tweak_pickle_endpoint_client_impl*)client_endpoint;

  TRIGGER_EVENT(client_endpoint_impl->skeleton.connection_state_listener, TWEAK_PICKLE_DISCONNECTED);

  tweak_wire_destroy_connection(client_endpoint_impl->wire_connection);
  client_endpoint_impl->wire_connection = NULL;

  free(client_endpoint);
}
