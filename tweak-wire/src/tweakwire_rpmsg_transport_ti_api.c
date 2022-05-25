/**
 * @file tweakwire_rpmsg_transport_ti_api.c
 * @ingroup tweak-internal
 *
 * @brief RPMessage transport interfacing low-level TI API.
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

tweak_wire_error_code tweak_wire_rpmsg_init_transport(
    struct tweak_wire_rpmsg_transport *transport, const char* endpoint_name, uint32_t endpoint,
    const char *params)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, endpoint_name = %s, endpoint = %u, role=%s", transport,
        endpoint_name, endpoint, params);

    bool server_role;
    if (params) {
        if (strcmp("role=server", params) == 0) {
            TWEAK_LOG_TRACE("server role is selected, announcing endpoint");
            server_role = true;
        } else if (strcmp("role=client", params) == 0) {
            TWEAK_LOG_TRACE("client role is selected, waiting for endpoint to be announced");
            server_role = false;
        } else {
            TWEAK_LOG_ERROR("Can't parse connection params: \"%s\"", params);
            return TWEAK_WIRE_ERROR;
        }
    } else {
        TWEAK_LOG_ERROR("Connection params is NULL");
        return TWEAK_WIRE_ERROR;
    }

    RPMessage_Params rpmsg_params;
    int32_t status;

    /* TODD: Use cache-coherent well-aligned buffer for IPC */
    RPMessageParams_init(&rpmsg_params);
    rpmsg_params.requestedEndpt = endpoint;
    rpmsg_params.buf = &transport->mcu_rpmsg_buf;
    rpmsg_params.bufSize = sizeof(transport->mcu_rpmsg_buf);
    rpmsg_params.numBufs = tweak_wire_rpmsg_num_buffers_in_flight;

    if ((transport->rpmsg = RPMessage_create(&rpmsg_params, &transport->local_endpoint)) != NULL) {
        TWEAK_LOG_DEBUG("RPMessage_create success: local_endpoint=%u",
            transport->local_endpoint);
    } else  {
        TWEAK_LOG_ERROR("RPMessage_create failed %d", endpoint);
        return TWEAK_WIRE_ERROR;
    }

    /* ...announce service name to remote processor */
    status = RPMessage_announce(RPMESSAGE_ALL, transport->local_endpoint, endpoint_name);
    if (status == IPC_SOK) {
        TWEAK_LOG_DEBUG("RPMessage_announce success");
    } else {
        TWEAK_LOG_ERROR("RPMessage_announce failed, status: %u", status);
        return TWEAK_WIRE_ERROR;
    }

    /* ...wait for announce from remote side */
    if (!server_role)
    {
        status = RPMessage_getRemoteEndPt(IPC_MCU2_0, endpoint_name, &transport->remote_proc,
            &transport->remote_endpoint, UINT32_MAX);
        if (status == IPC_SOK) {
            TWEAK_LOG_DEBUG("RPMessage_getRemoteEndPt success: remote_proc=%u, remote_endpoint=%u",
                transport->remote_proc, transport->remote_endpoint);
        } else {
            TWEAK_LOG_ERROR("RPMessage_getRemoteEndPt failed, status: %u", status);
            return TWEAK_WIRE_ERROR;
        }
    }

    return TWEAK_WIRE_SUCCESS;
}

tweak_wire_error_code tweak_wire_rpmsg_transport_send(struct tweak_wire_rpmsg_transport *transport,
                                                      const uint8_t *buffer, uint16_t len)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, buffer = %p, len = %d", transport, buffer, len);

    int32_t status;

    status = RPMessage_send(transport->rpmsg,
                            transport->remote_proc, transport->remote_endpoint,
                            transport->local_endpoint,
                            (void *)buffer, len);

    switch (status)
    {
    case IPC_SOK:
        TWEAK_LOG_TRACE("IPC_SOK");
        break;

    case IPC_E_UNBLOCKED:
        TWEAK_LOG_TRACE("IPC_E_UNBLOCKED");
        return (tweak_wire_error_code)TWEAK_WIRE_FINALIZING;

    case IPC_ETIMEOUT:
        TWEAK_LOG_DEBUG("timeout sending a message");
        return TWEAK_WIRE_ERROR_TIMEOUT;

    default:
        TWEAK_LOG_ERROR("RPMessage_send failed: %d", status);
        return TWEAK_WIRE_ERROR;
    }

    TWEAK_LOG_DEBUG("msg sent to %u:%u: length=%u",
      transport->remote_proc, transport->remote_endpoint, len);

    return TWEAK_WIRE_SUCCESS;
}

tweak_wire_error_code tweak_wire_rpmsg_transport_receive(struct tweak_wire_rpmsg_transport *transport,
                                                         uint8_t *buffer, uint16_t *len)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p, buffer = %p, *len = %d", transport, buffer, *len);

    int32_t status;

    /* ...wait for input message */
    status = RPMessage_recv(transport->rpmsg, (void *)buffer, len,
                            &transport->remote_endpoint, &transport->remote_proc,
                            TWEAK_WIRE_TIMEOUT * 1000u /* ms -> us */);

    switch (status)
    {
    case IPC_SOK:
        TWEAK_LOG_TRACE("IPC_SOK");
        break;

    case IPC_E_UNBLOCKED:
        TWEAK_LOG_TRACE("IPC_E_UNBLOCKED");
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
    TWEAK_LOG_TRACE_ENTRY("transport = %p", transport);

    RPMessage_unblock(transport->rpmsg);
}

void tweak_wire_rpmsg_transport_close(struct tweak_wire_rpmsg_transport *transport)
{
    TWEAK_LOG_TRACE_ENTRY("transport = %p", transport);

    int32_t status;

    status = RPMessage_delete(&transport->rpmsg);
    transport->rpmsg = NULL;

    if (status != IPC_SOK)
    {
        TWEAK_LOG_ERROR("RPMessage_delete failed: %x", status);
    }
}
