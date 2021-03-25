/**
 * @file tweakwire_nng.c
 * @ingroup tweak-internal
 *
 * @brief Tweak wire transport layer implementation, NNG backend.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>

#include "tweakwire_nng.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>

#include <inttypes.h>

#define TWEAK_WIRE_DATAGRAM_HEADER_SIZE 2

#define TWEAK_WIRE_TIMEOUT 500

#define TWEAK_WIRE_FINALIZING 100

/*
 * This structure contains all
 * context needed to maintain a single
 * node in point to point NNG library connection.
 */
struct transport
{
  /*
   * nng socket for both client and server instances.
   */
  nng_socket socket;
  /*
   * nng async interface for transmit queue.
   */
  nng_aio *transmit_aio;
  /*
   * nng async interface for receive queue.
   */
  nng_aio *receive_aio;
  /*
   * Receive queue shouldn't submit new io requests if this flag is true.
   */
  volatile bool is_finalizing;
};

/*
 * Subclass of tweak_wire_connection_base class.
 * This particular class uses nng point to point
 * connection as backend.
 */
struct tweak_wire_connection_nng
{
    /*
   * Instance of base class.
   */
    struct tweak_wire_connection_base base;
    /*
   * Common context data for both client
   * and server endpoints.
   */
    struct transport transport;
    /*
   * Pointer to connection state listener callback.
   */
    tweak_wire_connection_state_listener state_listener;
    /*
   * Opaque pointer that is being passed to state listener.
   */
    void *state_listener_cookie;
    /*
   * Pointer to receive listener callback.
   */
    tweak_wire_receive_listener receive_listener;
    /*
   * Opaque pointer that is being passed to receive listener.
   */
    void *receive_listener_cookie;
    /*
   * Discriminator for the following union.
   */
    bool is_server;
    /*
   * This union contains data
   * specific for client
   * and server nodes.
   */
    union
    {
        /* Nng acceptor object. */
        nng_listener listener;
        /* Nng connector object. */
        nng_dialer dialer;
    };
};

/**
 * @brief transmit bytes over the nng connection.
 */
tweak_wire_error_code tweak_wire_nng_transmit(tweak_wire_connection connection,
                                              const uint8_t *buffer, size_t size);

/**
 * @brief Destroy connection with nng backend.
 */
void tweak_wire_destroy_nng_connection(tweak_wire_connection connection);

static bool check_packet_header(uint8_t *buffer_with_header,
                                size_t size_with_header);

static void on_new_connection(nng_pipe pipe, nng_pipe_ev pipe_event, void* arg);

static void on_connection_lost(nng_pipe pipe, nng_pipe_ev pipe_event, void* arg);

static void recv_async_callback(void* arg);

static void
transmit_async_callback(void* arg);

static tweak_wire_error_code start_dialer(struct tweak_wire_connection_nng *connection_nng,
  const char *uri);

static tweak_wire_error_code start_listener(struct tweak_wire_connection_nng *connection_nng,
  const char *uri);

tweak_wire_connection tweak_wire_create_nng_connection(
    const char *connection_type, const char *params, const char *uri,
    tweak_wire_connection_state_listener connection_state_listener,
    void *connection_state_cookie, tweak_wire_receive_listener receive_listener,
    void *receive_listener_cookie)
{
  TWEAK_LOG_TRACE_ENTRY("connection_type=\"%s\", params=\"%s\", uri=\"%s\","
    " connection_state_listener=%p, connection_state_cookie=%p,"
    " receive_listener=%p, receive_listener_cookie=%p",
    connection_type, params, uri, 
    connection_state_listener, connection_state_cookie,
    receive_listener, receive_listener_cookie);

  if (strcmp("nng", connection_type) != 0) {
    TWEAK_LOG_ERROR("Connection type must be '%s' for this backend but '%s' was provided.", "nng", connection_type);
    return TWEAK_WIRE_INVALID_CONNECTION;
  }

  bool server_role;

  if (strcmp("role=server", params) == 0) {
    server_role = true;
  } else if (strcmp("role=client", params) == 0) {
    server_role = false;
  } else {
    TWEAK_LOG_ERROR("Can't parse connection params: \"%s\"", params);
    return TWEAK_WIRE_INVALID_CONNECTION;
  }

  if (!receive_listener) {
    TWEAK_LOG_ERROR("Mandatory parameter receive_listener is NULL");
    return TWEAK_WIRE_INVALID_CONNECTION;
  }

  struct tweak_wire_connection_nng *connection = calloc(1, sizeof(*connection));

  connection->state_listener = connection_state_listener;
  connection->state_listener_cookie = connection_state_cookie;

  connection->receive_listener = receive_listener;
  connection->receive_listener_cookie = receive_listener_cookie;

 if (server_role) {
    if (start_listener(connection, uri) != TWEAK_WIRE_SUCCESS) {
      free(connection);
      return TWEAK_WIRE_INVALID_CONNECTION;
    }
  } else {
    if (start_dialer(connection, uri) != TWEAK_WIRE_SUCCESS) {
      free(connection);
      return TWEAK_WIRE_INVALID_CONNECTION;
    }
  }
  
  connection->base.transmit_proc = &tweak_wire_nng_transmit;
  connection->base.destroy_proc = &tweak_wire_destroy_nng_connection;

  return &connection->base;
}

static void finalize_transport(struct transport *transport);

static tweak_wire_error_code initialize_transport(struct transport* transport,
  const char *uri, void* aio_cookie)
{
  TWEAK_LOG_TRACE_ENTRY("transport = %p, uri = %s, aio_cookie = %p",
    transport, uri, aio_cookie);

  int rv;
  rv = nng_pair0_open(&transport->socket);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_pair0_open returned %d", rv);
    return TWEAK_WIRE_ERROR;
  }

  rv = nng_aio_alloc(&transport->receive_aio, &recv_async_callback, aio_cookie);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_aio_alloc returned %d", rv);
    finalize_transport(transport);
    return TWEAK_WIRE_ERROR;
  }

  rv = nng_aio_alloc(&transport->transmit_aio, &transmit_async_callback, aio_cookie);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_aio_alloc returned %d", rv);
    finalize_transport(transport);
    return TWEAK_WIRE_ERROR;
  }

  nng_aio_set_timeout(transport->transmit_aio, TWEAK_WIRE_TIMEOUT);
  rv = nng_pipe_notify(transport->socket, NNG_PIPE_EV_ADD_POST,
    &on_new_connection, aio_cookie);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_pipe_notify returned %d", rv);
    finalize_transport(transport);
    return TWEAK_WIRE_ERROR;
  }

  rv = nng_pipe_notify(transport->socket, NNG_PIPE_EV_REM_POST,
    &on_connection_lost, aio_cookie);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_pipe_notify returned %d", rv);
    finalize_transport(transport);
    return TWEAK_WIRE_ERROR;
  }

  return TWEAK_WIRE_SUCCESS;
}

/*
 * Initiates socket read operation on given transport instance
 * and exits.
 * Calling from socket receive callback
 * won't cause infinite recursion.
 */
static void submit_receive_task(struct transport *transport) {
  TWEAK_LOG_TRACE_ENTRY("transport = %p", transport);
  nng_recv_aio(transport->socket, transport->receive_aio);
}

static tweak_wire_error_code
  start_dialer(struct tweak_wire_connection_nng *connection_nng,
              const char *uri)
{
  TWEAK_LOG_TRACE_ENTRY("connection_nng = %p, uri = %s",
    connection_nng, uri);
  connection_nng->is_server = false;

  int rv;
  rv = initialize_transport(&connection_nng->transport,
    uri, connection_nng);
  if (rv != TWEAK_WIRE_SUCCESS) {
    return rv;
  }

  rv = nng_dial(connection_nng->transport.socket, uri,
                &connection_nng->dialer, NNG_FLAG_NONBLOCK);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_dial returned %d", rv);
    finalize_transport(&connection_nng->transport);
    return TWEAK_WIRE_ERROR;
  }

  submit_receive_task(&connection_nng->transport);
  return TWEAK_WIRE_SUCCESS;
}

static tweak_wire_error_code
  start_listener(struct tweak_wire_connection_nng *connection_nng,
                const char *uri)
{
  TWEAK_LOG_TRACE_ENTRY("connection_nng = %p, uri = %s",
    connection_nng, uri);
  connection_nng->is_server = true;

  int rv;
  rv = initialize_transport(&connection_nng->transport,
    uri, connection_nng);

  if (rv != TWEAK_WIRE_SUCCESS) {
    return rv;
  }
  rv = nng_listen(connection_nng->transport.socket, uri,
                  &connection_nng->listener, 0);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_listen returned %d", rv);
    finalize_transport(&connection_nng->transport);
    return TWEAK_WIRE_ERROR;
  }

  submit_receive_task(&connection_nng->transport);
  return TWEAK_WIRE_SUCCESS;
}

static void on_new_connection(nng_pipe pipe, nng_pipe_ev ev, void* arg) {
  TWEAK_LOG_TRACE_ENTRY("pipe = 0x%X, ev = 0x%X, arg = %p",
    pipe.id, ev, arg);
  struct tweak_wire_connection_nng *connection_nng = arg;
  void *cookie = connection_nng->state_listener_cookie;
  tweak_wire_connection_state_listener listener;
  listener = connection_nng->state_listener;
  if (listener != NULL) {
    listener(&connection_nng->base, TWEAK_WIRE_CONNECTED, cookie);
  }
}

static void on_connection_lost(nng_pipe pipe, nng_pipe_ev ev, void* arg) {
  TWEAK_LOG_TRACE_ENTRY("pipe = 0x%X, ev = 0x%X, arg = %p",
    pipe.id, ev, arg);
  struct tweak_wire_connection_nng *connection_nng = arg;
  void *cookie = connection_nng->state_listener_cookie;
  tweak_wire_connection_state_listener listener;
  listener = connection_nng->state_listener;
  if (listener != NULL) {
    TWEAK_LOG_TRACE("Invoking user connection state listener %p,"
      " state = TWEAK_WIRE_DISCONNECTED, cookie = %p", listener, cookie);
    listener(&connection_nng->base, TWEAK_WIRE_DISCONNECTED, cookie);
  } else {
    TWEAK_LOG_TRACE("User hasn't provided connection state listener");
  }
}

static void transmit_async_callback(void* arg) {
  TWEAK_LOG_TRACE_ENTRY("arg = %p", arg);
}

static void recv_async_callback(void* arg) {
  TWEAK_LOG_TRACE_ENTRY("arg = %p", arg);
  struct tweak_wire_connection_nng *connection_nng =
    (struct tweak_wire_connection_nng *)arg;
  int32_t aio_result =
      nng_aio_result(connection_nng->transport.receive_aio);

  if (aio_result == TWEAK_WIRE_FINALIZING) {
    TWEAK_LOG_TRACE("aio_result == TWEAK_WIRE_FINALIZING, leaving async receive loop");
    return;
  }

  if (aio_result == 0) {
    nng_msg *inbound_message =
        nng_aio_get_msg(connection_nng->transport.receive_aio);
    uint8_t *buffer_with_header = nng_msg_body(inbound_message);
    size_t size_with_header = nng_msg_len(inbound_message);
    TWEAK_LOG_TRACE_HEXDUMP("nng receive", buffer_with_header, size_with_header);
    if (check_packet_header(buffer_with_header, size_with_header)) {
      uint8_t *payload = buffer_with_header + TWEAK_WIRE_DATAGRAM_HEADER_SIZE;
      size_t payload_size = size_with_header - TWEAK_WIRE_DATAGRAM_HEADER_SIZE;
      TWEAK_LOG_TRACE("Invoking user receive listener %p, cookie = %p",
        connection_nng->receive_listener, connection_nng->receive_listener_cookie);
      connection_nng->receive_listener(payload, payload_size,
        connection_nng->receive_listener_cookie);
    } else {
      TWEAK_LOG_WARN("Invalid datagram header");
    }

    nng_msg_free(inbound_message);
  }

  if (!connection_nng->transport.is_finalizing) {
    submit_receive_task(&connection_nng->transport);
  }
}

tweak_wire_error_code tweak_wire_nng_transmit(tweak_wire_connection connection,
  const uint8_t *buffer, size_t size)
{
  TWEAK_LOG_TRACE_ENTRY("connection = %p, buffer = %p, size = %" PRId64 "",
    connection, buffer, size);
  struct tweak_wire_connection_nng *connection_nng =
      (struct tweak_wire_connection_nng *)connection;

  if (connection_nng->transport.is_finalizing) {
    TWEAK_LOG_TRACE("connection is finalizing, aborting transmit request");
    return TWEAK_WIRE_ERROR;
  }

  size_t size_with_header = size + TWEAK_WIRE_DATAGRAM_HEADER_SIZE;
  nng_msg *outbound_message;
  int rv;
  rv = nng_msg_alloc(&outbound_message, size_with_header);
  if (rv != 0) {
    TWEAK_LOG_ERROR("nng_msg_alloc returned %d", rv);
    return TWEAK_WIRE_ERROR;
  }

  uint8_t *buffer_with_header = nng_msg_body(outbound_message);
  buffer_with_header[0] = 'T';
  buffer_with_header[1] = 'W';
  memcpy(buffer_with_header + TWEAK_WIRE_DATAGRAM_HEADER_SIZE, buffer, size);
  TWEAK_LOG_TRACE_HEXDUMP("nng transmit", buffer_with_header, size_with_header);
  int32_t aio_result;
  nng_aio_set_msg(connection_nng->transport.transmit_aio,
                  outbound_message);
  nng_send_aio(connection_nng->transport.socket,
               connection_nng->transport.transmit_aio);
  TWEAK_LOG_TRACE("Before nng_aio_wait");
  nng_aio_wait(connection_nng->transport.transmit_aio);
  TWEAK_LOG_TRACE("After nng_aio_wait");
  aio_result = nng_aio_result(connection_nng->transport.transmit_aio);
  if (aio_result == 0) {
    return TWEAK_WIRE_SUCCESS;
  } else {
    TWEAK_LOG_TRACE("nng_aio_result returned %d", aio_result);
    nng_msg_free(outbound_message);
    return aio_result == NNG_ETIMEDOUT 
      ? TWEAK_WIRE_ERROR_TIMEOUT
      : TWEAK_WIRE_ERROR;
  }
}

static bool check_packet_header(uint8_t *buffer_with_header,
                                size_t size_with_header) {
  if (size_with_header < TWEAK_WIRE_DATAGRAM_HEADER_SIZE)
    return false;

  if (buffer_with_header[0] != 'T')
    return false;

  if (buffer_with_header[1] != 'W')
    return false;

  return true;
}

static void finalize_transport(struct transport* transport) {
  TWEAK_LOG_TRACE_ENTRY("transport = %p", transport);
  if (transport->receive_aio != NULL) {
    /* This abort operation calls receive callback with
     * TWEAK_WIRE_FINALIZING error code.
     * When recv_async_callback receives this error
     * code, it exits without submitting new receive task.
     * After that, respective aio queue becomes empty.
     * If aio queue isn't empty, nng_aio_free will block indefinitely.
     * This nng_aio_abort call ensure that it doesn't happen.
     */
    TWEAK_LOG_TRACE("Aborting pending IO operations on receive_aio");
    transport->is_finalizing = true;
    nng_aio_abort(transport->receive_aio, TWEAK_WIRE_FINALIZING);
    nng_aio_free(transport->receive_aio);
    transport->receive_aio = NULL;
  }
  if (transport->transmit_aio) {
    /* If transmit operation is in progress, nng_aio_free
     * could block until datagram is sent or configured
     * timeout is reached.
     * This nng_aio_abort call ensure that it doesn't happen.
     */
    TWEAK_LOG_TRACE("Aborting pending IO operations on transmit_aio");
    nng_aio_abort(transport->transmit_aio, TWEAK_WIRE_FINALIZING);
    nng_aio_free(transport->transmit_aio);
    transport->transmit_aio = NULL;
  }
  TWEAK_LOG_TRACE("Closing nng socket");
  nng_close(transport->socket);
  TWEAK_LOG_TRACE("Transport finalized");
}

void tweak_wire_destroy_nng_connection(tweak_wire_connection connection) {
  TWEAK_LOG_TRACE_ENTRY("connection = %p", connection);
  struct tweak_wire_connection_nng *connection_nng =
      (struct tweak_wire_connection_nng *)connection;

  finalize_transport(&connection_nng->transport);
  if (connection_nng->is_server) {
    TWEAK_LOG_TRACE("Before nng_listener_close");
    nng_listener_close(connection_nng->listener);
    TWEAK_LOG_TRACE("After nng_listener_close");
  } else {
    TWEAK_LOG_TRACE("Before nng_dialer_close");
    nng_dialer_close(connection_nng->dialer);
    TWEAK_LOG_TRACE("After nng_dialer_close");
  }
  free(connection_nng);
}
