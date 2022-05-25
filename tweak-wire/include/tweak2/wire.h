/**
 * @file wire.h
 * @ingroup tweak-internal
 *
 * @brief Tweak wire protocol definition.
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

/**
 * @defgroup tweak-internal Tweak Internals
 * Internal details of Tweak Tool implementation. May be useful for debugging,
 * extending and hacking.
 */

#ifndef TWEAKWIRE_H_INCLUDED
#define TWEAKWIRE_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tweak_wire_connection_base;

/**
 * @ingroup tweak-internal
 *
 * @brief Error codes for tweak-wire calls.
 */
typedef enum {
  /**
   * @brief Operation completed successfully.
   */
  TWEAK_WIRE_SUCCESS = 0,
  /**
   * @brief Unrecoverable protocol level error.
   */
  TWEAK_WIRE_ERROR,
  /**
   * @brief Operation was not completed in time.
   * Caller may repeat operation.
   */
  TWEAK_WIRE_ERROR_TIMEOUT,
} tweak_wire_error_code;

/**
 * @brief virtual method definition for transmit procedure.
 *
 * Each backend should implement its own.
 *
 * @param[in] connection Instance associated with connection.
 * @param[in] buffer An array of bytes to transmit.
 * @param[in] size Number of bytes in the buffer referenced by @p buffer parameter.
 * @return
 * - TWEAK_WIRE_SUCCESS If the datagram is sent,
 * - TWEAK_WIRE_ERROR_TIMEOUT Operation hasn't been completed in statically configured time.
 * - TWEAK_WIRE_ERROR If there was unrecoverable library error.
 */
typedef tweak_wire_error_code (*tweak_wire_transmit_proc)(
  struct tweak_wire_connection_base* connection,
  const uint8_t *buffer, size_t size);

/**
 * @brief virtual destrictor for wire connection.
 *
 * Each backend should implement its own.
 * @param[in] connection Connection to destroy.
 */
typedef void (*tweak_wire_destroy_proc)(
  struct tweak_wire_connection_base* connection);

/**
 * @brief base class for all backends.
 *
 * Each backend should provide concrete implementations
 * for these virtual functions in its respective
 * instance constructor.
 */
struct tweak_wire_connection_base {
  /**
   * @brief transmit datagram.
   *
   * @see tweak_wire_transmit_proc.
   */
  tweak_wire_transmit_proc transmit_proc;
  /**
   * @brief Destroy backend instance and free all memory.
   *
   * @see tweak_wire_destroy_proc.
   */
  tweak_wire_destroy_proc destroy_proc;
};

/**
 * @ingroup tweak-internal
 *
 * @brief Opaque tweak wire protocol connection.
 *
 * @note The user shall not access nor manipulate the contents
 *       of the structure because it can change across versions
 *       without warning.
 */
typedef struct tweak_wire_connection_base *tweak_wire_connection;

/**
 * @ingroup tweak-internal
 *
 * @brief Invalid connection.
 */
#define TWEAK_WIRE_INVALID_CONNECTION (NULL)

/**
 * @ingroup tweak-internal
 *
 * @brief Tweak wire protocol connection status.
 */
typedef enum {
  /**
   * @brief There's no active connection.
   * tweak_wire_receive_listener will not register
   * inbound datagrams and tweak_wire_transmit() call
   * is expected to return TWEAK_WIRE_ERROR_TIMEOUT
   * after statically configured delay.
   */
  TWEAK_WIRE_DISCONNECTED = 0,
  /**
   * @brief Connection is established.
   */
  TWEAK_WIRE_CONNECTED
} tweak_wire_connection_state;

/**
 * @ingroup tweak-internal
 *
 * @brief Function signature for registering as
 * a connection state listener.
 *
 * @param[in] connection Connection instance.
 * @param[in] connection_state New connection state.
 * @param[in] cookie Value passed as connection_state_cookie parameter
 * to tweak_wire_create_connection() call.
 */
typedef void (*tweak_wire_connection_state_listener)(
    tweak_wire_connection connection,
    tweak_wire_connection_state connection_state,
    void* cookie);

/**
 * @ingroup tweak-internal
 *
 * @brief Function signature for registering as
 * receive inbound packets listener.
 *
 * @param[in] buffer Pointer to data received.
 * @param[in] size Size of data pinted by @p buffer.
 * @param[in] cookie value passed as receive_listener_cookie parameter
 * to tweak_wire_create_connection() call.
 */
typedef void (*tweak_wire_receive_listener)(const uint8_t *buffer, size_t size, void *cookie);

/**
 * @ingroup tweak-internal
 *
 * @brief Creates new instance of the connection.
 *
 * @param[in] connection_type One of "nng", "serial". Type is case-sensitive.
 * @param[in] params Additional params for backend seperated by semicolon ';'.
 * Only mutually exclusive "role=server" and "role=client"
 * are currently recognized for IP-based connections.
 * @param[in] uri Connection URI for the given network backend. NULL means default..
 * @param[in] connection_state_listener Application provided callback
 * that is called when connection state is changed.
 * This listener is optional, so it can be NULL if notification
 * about state change is not needed.
 * @param[in] connection_state_cookie Arbitrary pointer being passed to
 * @p connection_state_listener.
 * @param[in] receive_listener Listener for application provided callback
 * for accepting inbound datagrams. This is a mandatory listener,
 * and therefore NULL isn't accepted for this parameter.
 * @param[in] receive_listener_cookie Arbitrary pointer being passed to
 * @p receive_listener.
 * @return connection handle or TWEAK_WIRE_INVALID_CONNECTION.
 */
tweak_wire_connection tweak_wire_create_connection(
    const char *connection_type, const char *params, const char *uri,
    tweak_wire_connection_state_listener connection_state_listener,
    void *connection_state_cookie, tweak_wire_receive_listener receive_listener,
    void *receive_listener_cookie);

/**
 * @ingroup tweak-internal
 *
 * @brief Transmits chunk of bytes.
 *
 * @details This function will block until datagram is sent or there's timeout
 * due to broken connection. We assume that datagram being sent successfully
 * is a state when write() call on a TCP socket handle reported success status.
 * It doesn't necessarily mean that datagram was successfully parsed by
 * presentation layer and handled by application layer.
 *
 * @note this call basically re-route request to concrete implemeation
 * of backend specific transmit procedure. Each backend might have its
 * own set of quirks.
 *
 * @param[in] connection Instance associated with connection.
 * @param[in] buffer An array of bytes to transmit.
 * @param[in] size Number of bytes in the buffer referenced by @p buffer parameter.
 * @return
 * - TWEAK_WIRE_SUCCESS If the datagram is sent,
 * - TWEAK_WIRE_ERROR_TIMEOUT Operation hasn't been completed in statically configured time.
 * - TWEAK_WIRE_ERROR If there was unrecoverable library error.
 */
tweak_wire_error_code tweak_wire_transmit(tweak_wire_connection connection,
                                          const uint8_t *buffer, size_t size);

/**
 * @ingroup tweak-internal
 *
 * @brief Stops connect/accept loops and deallocates
 * library resources bound to given connection.
 *
 * All pending transmit operations will be interrupted
 * and return TWEAK_WIRE_ERROR_TIMEOUT code.
 * @param[in] connection Connection to destroy.
 */
void tweak_wire_destroy_connection(tweak_wire_connection connection);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKWIRE_H_INCLUDED */
