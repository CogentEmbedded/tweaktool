/**
 * @file tweakwire_rpmsg_transport_sysbios.c
 * @ingroup tweak-internal
 *
 * @brief SYSBIOS version of RPMessage transport.
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

tweak_wire_error_code tweak_wire_rpmsg_init_transport(
    struct tweak_wire_rpmsg_transport *transport, uint32_t endpoint)
{
    RPMessage_Params params;
    int32_t status;

    /* TODD: Use cache-coherent well-aligned buffer for IPC */
    RPMessageParams_init(&params);
    params.requestedEndpt = endpoint;
    params.buf = &transport->mcu_rpmsg_buf;
    params.bufSize = sizeof(transport->mcu_rpmsg_buf);
    params.numBufs = tweak_wire_rpmsg_num_buffers_in_flight;

    if ((transport->rpmsg = RPMessage_create(&params, &transport->local_endpoint)) == NULL)
    {
        TWEAK_LOG_ERROR("failed to create RPMSG link");
        return TWEAK_WIRE_ERROR;
    }

    /* ...announce service name to remote processor */
    status = RPMessage_announce(RPMESSAGE_ALL, transport->local_endpoint, ENDPOINT_NAME);
    if (status != IPC_SOK)
    {
        TWEAK_LOG_ERROR("RPMessage_announce failed: %x", status);
        return TWEAK_WIRE_ERROR;
    }

    TWEAK_LOG_DEBUG("announced service '%s', endpoint %d", ENDPOINT_NAME, transport->local_endpoint);

    return TWEAK_WIRE_SUCCESS;
}

tweak_wire_error_code tweak_wire_rpmsg_transport_send(struct tweak_wire_rpmsg_transport *transport,
                                                      const uint8_t *buffer, uint16_t len)
{
    int32_t status;

    status = RPMessage_send(transport->rpmsg,
                            transport->remote_proc, transport->remote_endpoint,
                            transport->local_endpoint,
                            (void *)buffer, len);

    switch (status)
    {
    case IPC_SOK:
        break;

    case IPC_E_UNBLOCKED:
        return (tweak_wire_error_code)TWEAK_WIRE_FINALIZING;

    case IPC_ETIMEOUT:
        TWEAK_LOG_DEBUG("timeout sending a message");
        return TWEAK_WIRE_ERROR_TIMEOUT;

    default:
        TWEAK_LOG_ERROR("RPMessage_send failed: %d", status);
        return TWEAK_WIRE_ERROR;
    }

    TWEAK_LOG_DEBUG("sent message: %d bytes", len);

    return TWEAK_WIRE_SUCCESS;
}

tweak_wire_error_code tweak_wire_rpmsg_transport_receive(struct tweak_wire_rpmsg_transport *transport,
                                                         uint8_t *buffer, uint16_t *len)
{
    int32_t status;

    /* ...wait for input message */
    status = RPMessage_recv(transport->rpmsg, (void *)buffer, len,
                            &transport->remote_endpoint, &transport->remote_proc,
                            TWEAK_WIRE_TIMEOUT * 1000u /* ms -> us */);

    switch (status)
    {
    case IPC_SOK:
        break;

    case IPC_E_UNBLOCKED:
        return (tweak_wire_error_code)TWEAK_WIRE_FINALIZING;

    case IPC_ETIMEOUT:
        TWEAK_LOG_DEBUG("timeout receiving a message");
        return TWEAK_WIRE_ERROR_TIMEOUT;

    default:
        TWEAK_LOG_ERROR("RPMessage_recv failed: %d", status);
        return TWEAK_WIRE_ERROR;
    }

    TWEAK_LOG_DEBUG("msg received from %u:%u: length=%u",
              transport->remote_proc, transport->remote_endpoint, *len);

    return TWEAK_WIRE_SUCCESS;
}

void tweak_wire_rpmsg_transport_abort(struct tweak_wire_rpmsg_transport *transport)
{
    RPMessage_unblock(transport->rpmsg);
}

void tweak_wire_rpmsg_transport_close(struct tweak_wire_rpmsg_transport *transport)
{
    int32_t status;

    status = RPMessage_delete(&transport->rpmsg);
    transport->rpmsg = NULL;

    if (status != IPC_SOK)
    {
        TWEAK_LOG_ERROR("RPMessage_delete failed: %x", status);
    }
}
