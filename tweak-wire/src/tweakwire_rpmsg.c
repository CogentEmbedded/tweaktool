/**
 * @file tweakwire_rpmsg.c
 * @ingroup tweak-internal
 *
 * @brief Implementation of tweak wire API interface for TI RP Messaging.
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

#include <tweak2/log.h>
#include <tweak2/thread.h>
#include <tweak2/wire.h>

#include "tweakwire_rpmsg.h"
#include "tweakwire_rpmsg_transport.h"

#include <common/app.h> // from vision_apps

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/**
 * @brief Subclass of tweak_wire_connection_base class.
 *
 * This particular class uses rpmsg point to point connection as backend.
 */
struct tweak_wire_connection_rpmsg
{
    /**
     * @brief Instance of the base class.
     */
    struct tweak_wire_connection_base base;

    /**
     * @brief Pointer to connection state listener callback.
     */
    tweak_wire_connection_state_listener state_listener;

    /**
     * @brief Opaque pointer that is being passed to state listener.
     */
    void *state_listener_cookie;

    /**
     * @brief Pointer to receive listener callback.
     */
    tweak_wire_receive_listener receive_listener;

    /**
     * @brief Opaque pointer that is being passed to receive listener.
     */
    void *receive_listener_cookie;

    /**
     * @brief Receiving processing thread.
     */
    tweak_common_thread receive_thread;

    /**
     * @brief Statically-allocated receive buffer.
     * @note Static allocation is needed to minimize stack pressure. Stack space is limited on SYSBIOS.
     */
    uint8_t receive_buffer[tweak_wire_rpmsg_max_packet_size];

    /**
     * @brief Common context data for both client and server endpoints.
     */
    struct tweak_wire_rpmsg_transport transport;

    /**
     * @brief Flag indicating whether remote RPMSG peer considered disconnected.
     */
    tweak_wire_connection_state connection_state;
};

static tweak_wire_error_code tweak_wire_rpmsg_transmit(tweak_wire_connection connection_base,
                                                       const uint8_t *buffer, size_t size);
static void tweak_wire_rpmsg_destroy_connection(tweak_wire_connection connection_base);
static void *tweak_wire_rpmsg_receive_thread(void *arg);

static bool rpmsg_uri_parse(const char* param,
  char* endpoint_name, size_t endpoint_name_size,
  uint32_t* endpoint)
{
  const char prefix[] = "rpmsg://";
  bool rv = false;
  char* param0 = strdup(param);
  char* p0 = NULL;
  char* p1 = NULL;
  char* p2 = NULL;

  uint32_t endpoint0;

  if (!param0) {
    TWEAK_LOG_ERROR("strdup returned NULL");
    goto exit;
  }

  if (strncmp(prefix, param0, sizeof(prefix) - 1) != 0) {
    goto exit;
  }

  p0 = param0 + sizeof(prefix) - 1;
  p1 = strchr(p0, '/');

  if (p1) {
    *p1 = '\0';
    strncpy(endpoint_name, p0, endpoint_name_size - 1);
    endpoint_name[endpoint_name_size - 1] = '\0';
    ++p1;
  } else {
        /**
         * @details Linux side of the RPMsg channel uses standard rpmsg_chrdev device driver provided,
         *          @see https://github.com/torvalds/linux/tree/master/drivers/rpmsg
         *
         *          It abstracts all implementation details and allows the user to simply read and write
         *          arbitrary-sized messages from / to the character device.
         *
         *          On QNX, a custom endpoint name 'tweak' is used.
         */
        const char *default_endpoint_name;

#if defined(TI_ARM_R5F)
    uint32_t host_os = appGetHostOSType();
    if (host_os == APP_HOST_TYPE_LINUX)
    {
        default_endpoint_name = "rpmsg_chrdev";
    }
    else
    {
        default_endpoint_name = "tweak";
    }
#elif defined(__QNX__)
        default_endpoint_name = "tweak";
#elif defined(__linux__)
        default_endpoint_name = "rpmsg_chrdev";
#else
        default_endpoint_name = "tweak";
#endif

        strncpy(endpoint_name, default_endpoint_name, endpoint_name_size - 1);
        endpoint_name[endpoint_name_size - 1] = '\0';
        p1 = p0;
    }

    if (strlen(p1) < 1)
    {
        TWEAK_LOG_ERROR("uri lacks endpoint number");
        goto exit;
    }

    endpoint0 = strtol(p1, &p2, 10);
    if (*p2 != '\0')
    {
        TWEAK_LOG_ERROR("uri lacks endpoint number");
        goto exit;
    }

    *endpoint = endpoint0;
    rv = true;

exit:
  free(param0);
  return rv;
}


tweak_wire_connection tweak_wire_create_rpmsg_connection(
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

    if (strcmp("rpmsg", connection_type) != 0)
    {
        TWEAK_LOG_ERROR("Connection type must be '%s' for this backend but '%s' was provided.", "rpmsg", connection_type);
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    /*.. Client / server role does not matter for rpmsg,
         the connection is always established by the host microcontroller. */
    char endpoint_name[128];
    uint32_t endpoint = 0;
    if (!rpmsg_uri_parse(uri, endpoint_name, sizeof(endpoint_name), &endpoint))
    {
        TWEAK_LOG_ERROR("Cannot parse uri parameter: '%s'", uri);
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    if (receive_listener == NULL)
    {
        TWEAK_LOG_ERROR("Mandatory parameter receive_listener is NULL");
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    struct tweak_wire_connection_rpmsg *connection = malloc(sizeof(struct tweak_wire_connection_rpmsg));
    if (connection == NULL)
    {
        TWEAK_LOG_ERROR("Not enough memory to allocate struct tweak_wire_connection_nng");
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    connection->state_listener = connection_state_listener;
    connection->state_listener_cookie = connection_state_cookie;

    connection->receive_listener = receive_listener;
    connection->receive_listener_cookie = receive_listener_cookie;

    if (tweak_wire_rpmsg_init_transport(&connection->transport, endpoint_name, endpoint, params)
         != TWEAK_WIRE_SUCCESS)
    {
        free(connection);
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    tweak_common_thread_error status = tweak_common_thread_create(&connection->receive_thread,
                                                                  tweak_wire_rpmsg_receive_thread,
                                                                  (void *)connection);

    if (status != TWEAK_COMMON_THREAD_SUCCESS)
    {
        TWEAK_LOG_ERROR("tweak_common_thread_create failed: %d", status);
        free(connection);
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    connection->base.transmit_proc = &tweak_wire_rpmsg_transmit;
    connection->base.destroy_proc = &tweak_wire_rpmsg_destroy_connection;
    connection->connection_state = TWEAK_WIRE_DISCONNECTED;
    return &connection->base;
}

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
enum { CHUNK_NUMBER_OFFSET = 0, CHUNK_COUNT_OFFSET = 1, PAYLOAD_OFFSET = 2};

static tweak_wire_error_code send_partitioned(struct tweak_wire_rpmsg_transport *transport,
    const uint8_t *buffer, uint16_t len)
{
    const uint32_t max_chunk_payload_size = tweak_wire_rpmsg_max_chunk_size - PAYLOAD_OFFSET;
    uint32_t n_chunks = len / max_chunk_payload_size;
    if ((len % max_chunk_payload_size) != 0) {
        ++n_chunks;
    }
    if (n_chunks >= UINT8_MAX) {
        return TWEAK_WIRE_ERROR;
    }
    for (uint32_t n_chunk = 0; n_chunk < n_chunks; n_chunk++) {
        transport->send_buff[CHUNK_NUMBER_OFFSET] = (uint8_t)(n_chunk + 1); /* start from 1 */
        transport->send_buff[CHUNK_COUNT_OFFSET] = (uint8_t)n_chunks;

        uint32_t from = n_chunk * max_chunk_payload_size;
        uint32_t to = MIN(len, (n_chunk + 1) * max_chunk_payload_size);
        uint32_t chunk_payload_size = to - from;

        memcpy(&transport->send_buff[PAYLOAD_OFFSET], &buffer[from], chunk_payload_size);
        tweak_wire_error_code error_code  = tweak_wire_rpmsg_transport_send(transport,
            transport->send_buff, PAYLOAD_OFFSET + chunk_payload_size);

        if (error_code != TWEAK_WIRE_SUCCESS) {
            TWEAK_LOG_ERROR("tweak_wire_rpmsg_transport_send failed: %d", error_code);
            return error_code;
        }
        TWEAK_LOG_TRACE("%u of %u chunks sent",
            n_chunk + 1, n_chunks);
    }
    TWEAK_LOG_DEBUG("sent message: %d bytes", len);
    return TWEAK_WIRE_SUCCESS;
}

tweak_wire_error_code receive_partitioned(struct tweak_wire_rpmsg_transport *transport,
    uint8_t *buffer, uint16_t *len)
{
    uint32_t bytes_received = 0;
    uint32_t n_prev_chunk = 0;
    uint32_t n_chunk = 0;
    uint32_t n_chunks = 0;
    do {
        uint16_t chunk_size = sizeof(transport->recv_buff);
        tweak_wire_error_code error_code =
            tweak_wire_rpmsg_transport_receive(transport,
                transport->recv_buff, &chunk_size);

        if (error_code == TWEAK_WIRE_ERROR_TIMEOUT) {
            if (n_chunk == 0) {
                TWEAK_LOG_TRACE("tweak_wire_rpmsg_transport_receive() timeout");
            } else {
                TWEAK_LOG_WARN("Chunk sequence was abrupted prematurely on chunk %d of %d", n_chunk, n_chunks);
            }
            return error_code;
        }

        if (error_code != TWEAK_WIRE_SUCCESS) {
            TWEAK_LOG_ERROR("tweak_wire_rpmsg_transport_receive: %d", error_code);
            return error_code;
        }

        uint16_t chunk_payload_size = chunk_size - PAYLOAD_OFFSET;
        n_chunk = transport->recv_buff[CHUNK_NUMBER_OFFSET];
        n_chunks = transport->recv_buff[CHUNK_COUNT_OFFSET];
        if (n_prev_chunk != (n_chunk - 1)) {
            TWEAK_LOG_WARN("Chunk sequence was corrupted: chunk sequence number %u, prev chunk sequence number %u",
                n_chunk, n_prev_chunk);
            return TWEAK_WIRE_ERROR;
        }
        TWEAK_LOG_TRACE("tweak_wire_rpmsg_transport_receive() ok, %u of %u chunks received", n_chunk, n_chunks);
        n_prev_chunk = n_chunk;

        memcpy(&buffer[bytes_received], &transport->recv_buff[PAYLOAD_OFFSET], chunk_payload_size);
        bytes_received += chunk_payload_size;

        if (bytes_received > *len) {
            TWEAK_LOG_ERROR("Buffer is too short, needed at least %u bytes, given %u", *len, bytes_received);
            return TWEAK_WIRE_ERROR;
        }
    } while (n_chunk != n_chunks);

    /* ...wait for input message */
    *len = bytes_received;

    TWEAK_LOG_DEBUG("msg received from %u:%u: length=%u",
        transport->remote_proc, transport->remote_endpoint, *len);

    return TWEAK_WIRE_SUCCESS;
}

static void *tweak_wire_rpmsg_receive_thread(void *arg)
{
    TWEAK_LOG_TRACE_ENTRY("arg = %p", arg);
    uint16_t len;
    struct tweak_wire_connection_rpmsg *connection = (struct tweak_wire_connection_rpmsg *)arg;
    tweak_wire_error_code status;
    do {
        len = (uint16_t)sizeof(connection->receive_buffer);
        status = receive_partitioned(&connection->transport,
            connection->receive_buffer, &len);
        if (status == TWEAK_WIRE_SUCCESS)
        {
            bool has_escape = len >= 1 && (connection->receive_buffer[0] == TWEAKWIRE_RPMSG_ESCAPE[0]);
            bool shielded_escape = has_escape && len >= 2 && (connection->receive_buffer[1] == TWEAKWIRE_RPMSG_ESCAPE[0]);
            if (!has_escape || shielded_escape) {
                if (connection->connection_state == TWEAK_WIRE_DISCONNECTED) {
                    TWEAK_LOG_TRACE("Got first datagram, change state to TWEAK_WIRE_CONNECTED");
                    connection->state_listener(&connection->base,
                        TWEAK_WIRE_CONNECTED, connection->state_listener_cookie);
                    connection->connection_state = TWEAK_WIRE_CONNECTED;
                }
                if (!shielded_escape) {
                    TWEAK_LOG_TRACE("Invoking receive listener");
                    connection->receive_listener(connection->receive_buffer, len, connection->receive_listener_cookie);
                } else {
                    TWEAK_LOG_TRACE("Invoking receive listener, omitting escape sequence");
                    connection->receive_listener(connection->receive_buffer + 1, len - 1, connection->receive_listener_cookie);
                }
            } else {
                if (connection->state_listener != NULL
                    && memcmp(TWEAKWIRE_RPMSG_SEND_DISCONNECTED,
                              connection->receive_buffer,
                              sizeof(TWEAKWIRE_RPMSG_SEND_DISCONNECTED)) == 0)
                {
                    TWEAK_LOG_TRACE("Got service datagram, switching state to TWEAK_WIRE_DISCONNECTED");
                    connection->state_listener(&connection->base,
                        TWEAK_WIRE_DISCONNECTED, connection->state_listener_cookie);

                    connection->connection_state = TWEAK_WIRE_DISCONNECTED;
                } else {
                    TWEAK_LOG_WARN("Unrecognised service datagram, omitting");
                }
            }
        }

        /* Errors have been printed already by tweak_wire_rpmsg_transport_receive */

    } while (status != TWEAK_WIRE_FINALIZING);

    TWEAK_LOG_TRACE("Sending \"disconnect\" service datagram to remote peer");
    char disconnect_datagram[] = TWEAKWIRE_RPMSG_SEND_DISCONNECTED;
    send_partitioned(&connection->transport, (uint8_t *)disconnect_datagram, sizeof(disconnect_datagram));

    /*.. Report that we are disconnected */
    if (connection->state_listener != NULL)
    {
        connection->state_listener(&connection->base,
                                   TWEAK_WIRE_DISCONNECTED,
                                   connection->state_listener_cookie);
    }

    return NULL;
}

static tweak_wire_error_code tweak_wire_rpmsg_transmit(tweak_wire_connection connection_base,
                                                       const uint8_t *buffer, size_t size)
{
    TWEAK_LOG_TRACE_ENTRY("connection = %p, buffer = %p, size = %" PRId64 "",
                          connection_base, buffer, size);

    struct tweak_wire_connection_rpmsg *connection =
        (struct tweak_wire_connection_rpmsg *)connection_base;

    /*.. TODO: Make some sanity checks on connection, buffer and size */

    bool have_to_shield_escape = size >= 1 && (buffer[0] == TWEAKWIRE_RPMSG_ESCAPE[0]);
    TWEAK_LOG_TRACE("Sending datagram to remote peer");
    if (!have_to_shield_escape) {
        return send_partitioned(&connection->transport, buffer, size);
    } else {
        TWEAK_LOG_TRACE("Datagram starts with escape byte, prepending it with prefix");
        uint8_t *tmp_buffer = malloc(size + 1);
        if (tmp_buffer) {
            TWEAK_FATAL("Can't allocate memory for temp buffer");
        }
        memcpy(tmp_buffer + 1, buffer, size);
        tmp_buffer[0] = TWEAKWIRE_RPMSG_ESCAPE[0];
        tweak_wire_error_code error_code = send_partitioned(&connection->transport, (uint8_t *)tmp_buffer, size + 1);
        free(tmp_buffer);
        return error_code;
    }
}

static void tweak_wire_rpmsg_destroy_connection(tweak_wire_connection connection_base)
{
    TWEAK_LOG_TRACE_ENTRY("connection_base = %p", connection_base);
    struct tweak_wire_connection_rpmsg *connection =
        (struct tweak_wire_connection_rpmsg *)connection_base;

    /*.. TODO: Make some sanity checks on connection */

    tweak_wire_rpmsg_transport_abort(&connection->transport);
    tweak_common_thread_join(connection->receive_thread, NULL);

    tweak_wire_rpmsg_transport_close(&connection->transport);

    /*.. TODO: Introduce locks */
    free(connection);
}
