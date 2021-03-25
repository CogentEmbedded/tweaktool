/**
 * @file tweakwire_rpmsg.c
 * @ingroup tweak-internal
 *
 * @brief Implementation of tweak wire API interface for TI RP Messaging.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>
#include <tweak2/wire.h>

#include "tweakwire_rpmsg.h"
#include "tweakwire_rpmsg_transport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
     * @brief Receiving processing thread
     */
    pthread_t receive_thread;

    /**
     * @brief Statically-allocated receive buffer.
     * @note Static allocation is needed to minimize stack pressure. Stack space is limited on SYSBIOS.
     */
    uint8_t receive_buffer[tweak_wire_rpmsg_max_message_size];

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

tweak_wire_connection tweak_wire_create_rpmsg_connection(
    const char *connection_type, const char *params, const char *uri,
    tweak_wire_connection_state_listener connection_state_listener,
    void *connection_state_cookie, tweak_wire_receive_listener receive_listener,
    void *receive_listener_cookie)
{
    if (strcmp("rpmsg", connection_type) != 0)
    {
        TWEAK_LOG_ERROR("Connection type must be '%s' for this backend but '%s' was provided.", "rpmsg", connection_type);
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    /*.. Client / server role does not matter for rpmsg,
         the connection is always established by the host microcontroller. */
    uint32_t endpoint = 0;
    if (uri == NULL || sscanf(uri, "rpmsg://%u", &endpoint) != 1 || endpoint == 0)
    {
        /*.. failure is silent: use default */
        endpoint = 17;
        TWEAK_LOG_WARN("Cannot parse uri parameter: '%s', using default: 'rpmsg://%u'", params, endpoint);
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

    if (tweak_wire_rpmsg_init_transport(&connection->transport, endpoint) != TWEAK_WIRE_SUCCESS)
    {
        free(connection);
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    int32_t status = pthread_create(&connection->receive_thread, NULL,
                                    tweak_wire_rpmsg_receive_thread,
                                    (void *)connection);

    if (status != 0)
    {
        TWEAK_LOG_ERROR("pthread_create failed: %d", status);
        free(connection);
        return TWEAK_WIRE_INVALID_CONNECTION;
    }

    connection->base.transmit_proc = &tweak_wire_rpmsg_transmit;
    connection->base.destroy_proc = &tweak_wire_rpmsg_destroy_connection;
    connection->connection_state = TWEAK_WIRE_DISCONNECTED;
    return &connection->base;
}

static void *tweak_wire_rpmsg_receive_thread(void *arg)
{
    struct tweak_wire_connection_rpmsg *connection = (struct tweak_wire_connection_rpmsg *)arg;
    tweak_wire_error_code status;
    do
    {
        uint16_t len = (uint16_t)sizeof(connection->receive_buffer);
        status = tweak_wire_rpmsg_transport_receive(&connection->transport,
            connection->receive_buffer, &len);
        if (status == TWEAK_WIRE_SUCCESS)
        {
            bool has_escape = len >= 1 && (connection->receive_buffer[0] == TWEAKWIRE_RPMSG_ESCAPE[0]);
            bool shielded_escape = has_escape && len >= 2 && (connection->receive_buffer[1] == TWEAKWIRE_RPMSG_ESCAPE[0]);
            if (!has_escape || shielded_escape) {
                if (connection->connection_state == TWEAK_WIRE_DISCONNECTED) {
                    connection->state_listener(&connection->base,
                        TWEAK_WIRE_CONNECTED, connection->state_listener_cookie);
                    connection->connection_state = TWEAK_WIRE_CONNECTED;
                }
                if (!shielded_escape) {
                    connection->receive_listener(connection->receive_buffer, len, connection->receive_listener_cookie);
                } else {
                    connection->receive_listener(connection->receive_buffer + 1, len - 1, connection->receive_listener_cookie);
                }
            } else {
                if (connection->state_listener != NULL
                    && memcmp(TWEAKWIRE_RPMSG_SEND_DISCONNECTED,
                              connection->receive_buffer,
                              sizeof(TWEAKWIRE_RPMSG_SEND_DISCONNECTED)) == 0)
                {
                    connection->state_listener(&connection->base,
                        TWEAK_WIRE_DISCONNECTED, connection->state_listener_cookie);

                    connection->connection_state = TWEAK_WIRE_DISCONNECTED;
                }
            }
        }

        /* Errors have been printed already by tweak_wire_rpmsg_transport_receive */

    } while (status != TWEAK_WIRE_FINALIZING);

    char disconnect_datagram[] = TWEAKWIRE_RPMSG_SEND_DISCONNECTED;
    tweak_wire_rpmsg_transport_send(&connection->transport, (uint8_t *)disconnect_datagram, sizeof(disconnect_datagram));

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
    struct tweak_wire_connection_rpmsg *connection =
        (struct tweak_wire_connection_rpmsg *)connection_base;

    /*.. TODO: Make some sanity checks on connection, buffer and size */

    bool have_to_shield_escape = size >= 1 && (buffer[0] == TWEAKWIRE_RPMSG_ESCAPE[0]);
    if (!have_to_shield_escape) {
        return tweak_wire_rpmsg_transport_send(&connection->transport, buffer, size);
    } else {
        uint8_t *tmp_buffer = malloc(size + 1);
        if (tmp_buffer) {
            TWEAK_FATAL("Can't allocate memory for temp buffer");
        }
        memcpy(tmp_buffer + 1, buffer, size);
        tmp_buffer[0] = TWEAKWIRE_RPMSG_ESCAPE[0];
        tweak_wire_error_code error_code = tweak_wire_rpmsg_transport_send(&connection->transport, (uint8_t *)tmp_buffer, size + 1);
        free(tmp_buffer);
        return error_code;
    }
}

static void tweak_wire_rpmsg_destroy_connection(tweak_wire_connection connection_base)
{
    struct tweak_wire_connection_rpmsg *connection =
        (struct tweak_wire_connection_rpmsg *)connection_base;

    /*.. TODO: Make some sanity checks on connection */

    tweak_wire_rpmsg_transport_abort(&connection->transport);
    pthread_join(connection->receive_thread, NULL);

    tweak_wire_rpmsg_transport_close(&connection->transport);

    /*.. TODO: Introduce locks */
    free(connection);
}
