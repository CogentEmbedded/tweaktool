/**
 * @file tweakwire_rpmsg_transport_linux.c
 * @ingroup tweak-internal
 *
 * @brief Linux version of RPMessage transport.
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

#include "tweakwire_rpmsg_transport.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <rproc_id.h>
#include <ti_rpmsg_char.h>

#define LINUX_ENDPOINT_NAME "rpmsg_chrdev"

static int32_t create_tx_channel(struct tweak_wire_rpmsg_transport *transport)
{
    transport->remote_proc = R5F_MAIN0_0;

    int32_t fd = -1;
    rpmsg_char_dev_t local_dev;
    char name[32] = { 0 };

    snprintf(name, sizeof(name) - 1, "rpmsg-char-%d-%d", transport->remote_proc, getpid());

    transport->device = rpmsg_char_open(transport->remote_proc,
                NULL,
                transport->remote_endpoint,
                name,
                0
    );

    if (transport->device != NULL)
    {
        local_dev = *transport->device;
        transport->local_endpoint = local_dev.endpt;

        fd = local_dev.fd;
    }

    return fd;
}

tweak_wire_error_code tweak_wire_rpmsg_init_transport(
    struct tweak_wire_rpmsg_transport *transport, const char* endpoint_name, uint32_t endpoint,
    const char *params)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, endpoint_name = %s, endpoint = %u, role=%s", transport,
        endpoint_name, endpoint, params);

    if (params) {
        if (strcmp("role=server", params) == 0) {
            TWEAK_LOG_ERROR("Server role is not supported on rpmsg chrdev");
            return TWEAK_WIRE_ERROR;
        } else if (strcmp("role=client", params) == 0) {
            TWEAK_LOG_TRACE("Client role is selected, opening chrdev");
        } else {
            TWEAK_LOG_ERROR("Can't parse connection params: \"%s\"", params);
            return TWEAK_WIRE_ERROR;
        }
    } else {
        TWEAK_LOG_ERROR("Connection params is NULL");
        return TWEAK_WIRE_ERROR;
    }

    /*.. Create an eventfd for interrupting receive operation */
    if (strcmp(endpoint_name, LINUX_ENDPOINT_NAME) != 0)
    {
        TWEAK_LOG_ERROR("Only endpoint name %s is supported on Linux", LINUX_ENDPOINT_NAME);
        return TWEAK_WIRE_ERROR;
    }

    transport->eventfd_unblock = eventfd(0, 0);
    if (transport->eventfd_unblock < 0)
    {
        TWEAK_LOG_ERROR("eventfd failed: %m");
        return TWEAK_WIRE_ERROR;
    }

    /* .. Open RPMSG character device */
    transport->remote_endpoint = endpoint;
    transport->fd = create_tx_channel(transport);
    if (transport->fd < 0)
    {
        TWEAK_LOG_ERROR("failed to create endpoint %d: %m", transport->local_endpoint);
        return TWEAK_WIRE_ERROR;
    }

    return TWEAK_WIRE_SUCCESS;
}

/**
 * @brief Checks whether an operation @p flags is possible on main fd with select().
 * This function also checks when an event is pushed through eventfd_unblock.
 * @param transport RPMessage transport descriptor.
 * @param flags Operation flags.
 * @return int TWEAK_WIRE_SUCCESS in case the operation is possible.
 */
static tweak_wire_error_code tweak_wire_rpmsg_check_op_possible(struct tweak_wire_rpmsg_transport *transport, int flags)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, endpoint = %d", transport, flags);

    struct pollfd fds[2];
    fds[0].fd = transport->fd;
    fds[0].events = flags;
    fds[1].fd = transport->eventfd_unblock;
    fds[1].events = POLLIN;

    int rv = poll(fds, 2, TWEAK_WIRE_TIMEOUT);
    if (rv == 0)
    {
        TWEAK_LOG_TRACE("poll timeout");
        return TWEAK_WIRE_ERROR_TIMEOUT;
    }
    else if (rv > 0 && (fds[1].revents & POLLIN) != 0)
    {
        TWEAK_LOG_TRACE("closing connection");
        return (tweak_wire_error_code)TWEAK_WIRE_FINALIZING;
    }
    else if (rv > 0 && (fds[0].revents & flags) != 0)
    {
        TWEAK_LOG_TRACE("poll success");
        return TWEAK_WIRE_SUCCESS;
    }

    TWEAK_LOG_ERROR("poll() failed: %m");
    return TWEAK_WIRE_ERROR;
}

tweak_wire_error_code
tweak_wire_rpmsg_transport_send(struct tweak_wire_rpmsg_transport *transport,
                                const uint8_t *buffer, uint16_t len)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, buffer = %p, len = %d", transport, buffer, len);

    /*.. Wait with timeout until we are ready to write. */
    /*.. TBD not sure if this works on current rpmsg_chardev actually
    tweak_wire_error_code status = tweak_wire_rpmsg_check_op_possible(transport, POLLOUT);
    if (status != TWEAK_WIRE_SUCCESS)
    {
        return status;
    }
    */

    int rv = write(transport->fd, buffer, len);
    if (rv == -EINTR)
    {
        TWEAK_LOG_TRACE("closing connection");
        return (tweak_wire_error_code)TWEAK_WIRE_FINALIZING;
    }
    else if (rv < 0)
    {
        TWEAK_LOG_ERROR("write() failed: %m");
        return TWEAK_WIRE_ERROR;
    }

    TWEAK_LOG_DEBUG("sent message: %d bytes", len);

    return TWEAK_WIRE_SUCCESS;
}

tweak_wire_error_code tweak_wire_rpmsg_transport_receive(struct tweak_wire_rpmsg_transport *transport,
                                                         uint8_t *buffer, uint16_t *len)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, buffer = %p, len = %d", transport, buffer, *len);

    tweak_wire_error_code status = tweak_wire_rpmsg_check_op_possible(transport, POLLIN);
    if (status != TWEAK_WIRE_SUCCESS)
    {
        TWEAK_LOG_TRACE("read() operation isn't possible at the moment");
        return status;
    }

    int rv = read(transport->fd, buffer, *len);
    if (rv < 0)
    {
        TWEAK_LOG_DEBUG("read() failed: %m");
        return TWEAK_WIRE_ERROR;
    } else {
        TWEAK_LOG_TRACE("read() success");
    }

    *len = rv;
    TWEAK_LOG_DEBUG("receive message: %d bytes", *len);

    return TWEAK_WIRE_SUCCESS;
}

void tweak_wire_rpmsg_transport_abort(struct tweak_wire_rpmsg_transport *transport)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p", transport);

    int rv = write(transport->eventfd_unblock, &(uint64_t){1}, sizeof(uint64_t));
    if (rv < 0)
    {
        TWEAK_LOG_DEBUG("failed to write to eventfd: %m");
    } else {
        TWEAK_LOG_TRACE("write() success");
    }
}

void tweak_wire_rpmsg_transport_close(struct tweak_wire_rpmsg_transport *transport)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p", transport);

    rpmsg_char_close(transport->device);
    transport->fd = -1;
    transport->remote_endpoint = 0;
    transport->local_endpoint = 0;

    close(transport->eventfd_unblock);
    transport->eventfd_unblock = -1;
}
