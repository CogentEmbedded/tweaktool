/**
 * @file tweakwire_rpmsg_transport.h
 * @ingroup tweak-internal
 *
 * @brief RPMessage transport API for tweak wire.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

/**
 * @defgroup tweak-internal Tweak Internals
 * Internal details of Tweak Tool implementation. May be useful for debugging,
 * extending and hacking.
 */

#ifndef TWEAKWIRE_RPMSG_TRANSPORT_H_INCLUDED
#define TWEAKWIRE_RPMSG_TRANSPORT_H_INCLUDED

#include <tweak2/wire.h>

#include <stdbool.h>

#if defined(WIRE_RPMSG_BACKEND_TI_API)
#include <ti/drv/ipc/ipc.h>
#else
#include <ti_rpmsg_char.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TWEAK_WIRE_FINALIZING 100

/**
 * @brief Timeout on rpmsg read and write operations in ms.
 */
#define TWEAK_WIRE_TIMEOUT 500

enum
{
    /**
     * @brief Maximum rpmsg message size in bytes.
     * Fixed value is used for IPC buffer allocation and also to save one call to malloc on message receive.
     */
    tweak_wire_rpmsg_max_message_size = 512,

    /**
     * @brief Maximum amount of data that could be sent.
     * Maximum value supported by the protocol is 65535 packets with 252 bytes payload each.
     */
    tweak_wire_rpmsg_max_packet_size = 256 * 1024,

    /**
     * @brief RPMSG seem to reject datagrams larger that this.
     * Thus, larger arrays should be split on chunks with this size.
     */
    tweak_wire_rpmsg_max_chunk_size = 256,

    /**
     * @brief Length of the message queue in maximum-sized messages.
     * RPMsg transport will block after this many maximum-sized messages are in the queue.
     * This will vary depending on the size of messages.
     */
#if defined(__QNX__)
    /* WA: QNX does not block queue but just drop messages */
    tweak_wire_rpmsg_num_buffers_in_flight = 2048,
#else
    tweak_wire_rpmsg_num_buffers_in_flight = 256,
#endif
};

/**
 * @brief IPC message scratch buffer type declaration.
 *
 * 256 bytes are needed for internal structures of RPMsg.
 * @note This typedef is needed to align data on MCU memory page boundary.
 */
typedef uint8_t tweak_wire_rpmsg_buf_t[tweak_wire_rpmsg_max_message_size * tweak_wire_rpmsg_num_buffers_in_flight * 2 + 256] __attribute__((aligned(1024)));

/**
 * @brief RPMsg endpoint context.
 *
 * This structure contains all context data required to maintain a single endpoint in RPMessage protocol.
 */
struct tweak_wire_rpmsg_transport
{
    /**
     * @brief Remote side endpoint number.
     */
    uint32_t remote_endpoint;

    /**
     * @brief Processor id for remote side.
     */
    uint32_t remote_proc;

    /**
     * @brief Local endpoint number.
     */
    uint32_t local_endpoint;

    uint8_t recv_buff[tweak_wire_rpmsg_max_message_size];

    uint8_t send_buff[tweak_wire_rpmsg_max_message_size];

#if defined(WIRE_RPMSG_BACKEND_TI_API)

    /**> RPMSG handle on SYSBIOS */
    RPMessage_Handle rpmsg;

    /**
     * @brief RPMessage scratch buffer for MCU.
     */
    tweak_wire_rpmsg_buf_t mcu_rpmsg_buf;

#elif defined(WIRE_RPMSG_BACKEND_CHRDEV)

    /**
     * @brief File descriptor for communication with rpmsg_chardev.
     * Valid values are >= 0 per POSIX definition.
     */
    int fd;

    /**
     * @brief Event file descriptor for unblocking the receive thread.
     * Valid values are >= 0 per POSIX definition.
     */
    int eventfd_unblock;

    rpmsg_char_dev_t *device;
#else
#error Backend type must be defined
#endif
};

/**
 * @brief Initialize RPMessage transport.
 *
 * @param transport RPMessage transport descriptor.
 * @param endpoint_name Name of endpoint.
 * @param endpoint Endpoint to use.
 * @param params Determines if transport is initialized as server (announces an endpoint)
 *                                                    or client (waits for announce from remote side).
 * @return tweak_wire_error_code
 *       @ref TWEAK_WIRE_SUCCESS The transport was created.
 *       @ref TWEAK_WIRE_ERROR The transport cannot be created (permanent failure).
 */
tweak_wire_error_code tweak_wire_rpmsg_init_transport(
    struct tweak_wire_rpmsg_transport *transport, const char *endpoint_name,
    uint32_t endpoint, const char *params);

/**
 * @brief Transmit data over RPMessage endpoint.
 *
 * @param transport RPMessage transport descriptor
 * @param buffer Buffer to transmit.
 * @param len Size of @p buffer in bytes.
 * @return tweak_wire_error_code
 *          @ref TWEAK_WIRE_SUCCESS Transmission was completed successfully.
 *          @ref TWEAK_WIRE_ERROR_TIMEOUT Timeout happened.
 */
tweak_wire_error_code tweak_wire_rpmsg_transport_send(struct tweak_wire_rpmsg_transport *transport,
                                                      const uint8_t *buffer, uint16_t len);

/**
 * @brief Receive data from RPMessage endpoint.
 *
 * @param transport RPMessage transport descriptor.
 * @param buffer Received buffer to transmit.
 * @param len Size of @p buffer in bytes.
 * @return tweak_wire_error_code
 *          @ref TWEAK_WIRE_SUCCESS Some data wes received.
 *          @ref TWEAK_WIRE_ERROR_TIMEOUT Timeout happened.
 */
tweak_wire_error_code tweak_wire_rpmsg_transport_receive(struct tweak_wire_rpmsg_transport *transport,
                                                         uint8_t *buffer, uint16_t *len);

/**
 * @brief Unblock a pending receive operation before the timeout occurs.
 *
 * @param transport RPMessage transport descriptor.
 */
void tweak_wire_rpmsg_transport_abort(struct tweak_wire_rpmsg_transport *transport);

/**
 * @brief Close from RPMessage transport.
 *
 * @param transport RPMessage transport descriptor.
 */
void tweak_wire_rpmsg_transport_close(struct tweak_wire_rpmsg_transport *transport);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKWIRE_RPMSG_TRANSPORT_H_INCLUDED */
