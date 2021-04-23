/**
 * @file tweakwire_rpmsg_transport.h
 * @ingroup tweak-internal
 *
 * @brief RPMessage transport API for tweak wire.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-internal Tweak Internals
 * Internal details of Tweak Tool implementation. May be useful for debugging,
 * extending and hacking.
 */

#ifndef TWEAKWIRE_RPMSG_TRANSPORT_H_INCLUDED
#define TWEAKWIRE_RPMSG_TRANSPORT_H_INCLUDED

#include <tweak2/wire.h>

#include <pthread.h>
#include <stdbool.h>

#if defined(CPU_mcu2_0)
#include <ti/drv/ipc/ipc.h>
#endif

/**
 * @details Linux side of the RPMsg channel uses standard rpmsg_chrdev device driver provided,
 *          @see https://github.com/torvalds/linux/tree/master/drivers/rpmsg
 *
 *          It abstracts all implementation details and allows the user to simply read and write
 *          arbitrary-sized messages from / to the character device.
 */
#define ENDPOINT_NAME "rpmsg_chrdev"

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
    tweak_wire_rpmsg_max_message_size = 1024,

    /**
     * @brief Length of the message queue in maximum-sized messages.
     * RPMsg transport will block after this many maximum-sized messages are in the queue.
     * This will vary depending on the size of messages.
     */
    tweak_wire_rpmsg_num_buffers_in_flight = 128,
};

/**
 * @brief IPC message scratch buffer type declaration.
 *
 * 256 bytes are needed for internal structures of RPMsg.
 * @note This typedef is needed to align data on MCU memory page boundary.
 * @todo Optimize message number and default size.
 */
typedef uint8_t tweak_wire_rpmsg_buf_t[tweak_wire_rpmsg_max_message_size * tweak_wire_rpmsg_num_buffers_in_flight + 256] __attribute__((aligned(1024)));

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

#if defined(CPU_mcu2_0)

    /**> RPMSG handle on SYSBIOS */
    RPMessage_Handle rpmsg;

    /**
     * @brief RPMessage scratch buffer for MCU.
     */
    tweak_wire_rpmsg_buf_t mcu_rpmsg_buf;

#elif defined(CPU_mpu1)

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

#else
#error CPU type must be defined
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize RPMessage transport.
 *
 * @param transport RPMessage transport descriptor.
 * @param endpoint Endpoint to use.
 * @return tweak_wire_error_code
 *       @ref TWEAK_WIRE_SUCCESS The transport was created.
 *       @ref TWEAK_WIRE_ERROR The transport cannot be created (permanent failure).
 */
tweak_wire_error_code tweak_wire_rpmsg_init_transport(
    struct tweak_wire_rpmsg_transport *transport, uint32_t endpoint);

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
