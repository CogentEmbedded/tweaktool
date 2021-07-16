/**
 * @file tweakwire_rpmsg_transport_linux.c
 * @ingroup tweak-internal
 *
 * @brief Linux version of RPMessage transport.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>

#include "tweakwire_rpmsg_transport.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/ipc/include/app_ipc.h>

tweak_wire_error_code tweak_wire_rpmsg_init_transport(
    struct tweak_wire_rpmsg_transport *transport, uint32_t endpoint)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, endpoint = %u", transport, endpoint);
    /*.. Create an eventfd for interrupting receive operation */
    transport->eventfd_unblock = eventfd(0, 0);
    if (transport->eventfd_unblock < 0)
    {
        TWEAK_LOG_ERROR("eventfd failed: %m");
        return TWEAK_WIRE_ERROR;
    }

    /* TODO: Remove dependency on CPU Id */
    transport->remote_proc = appIpcGetAppCpuId("mcu2_0");
    transport->remote_endpoint = endpoint;

    /* ...make sure IPC is enabled for remote partner cpu (s) */
    if (appIpcIsCpuEnabled(transport->remote_proc) == false)
    {
        TWEAK_LOG_ERROR("IPC is not enabled for cpu #%u", transport->remote_proc);
        return TWEAK_WIRE_ERROR;
    }

    /* .. Open RPMSG character device */
    transport->fd = appIpcCreateTxCh(transport->remote_proc, transport->remote_endpoint, &transport->local_endpoint);
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
    TWEAK_LOG_TRACE_ENTRY("transport = %p, buffer = %p, len = %d", transport, buffer, len);

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

    appIpcDeleteCh(transport->remote_proc, transport->remote_endpoint, transport->local_endpoint, transport->fd);
    transport->fd = -1;
    transport->remote_endpoint = 0;
    transport->local_endpoint = 0;

    close(transport->eventfd_unblock);
    transport->eventfd_unblock = -1;
}
