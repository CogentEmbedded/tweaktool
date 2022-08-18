/**
 * @file tweakwire.c
 * @ingroup tweak-internal
 *
 * @brief Tweak wire transport layer implementation, abstract connection factory.
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

#include "tweak2/wire.h"

#if defined(WITH_WIRE_NNG)
#include "tweakwire_nng.h"
#endif

#if defined(WITH_WIRE_RPMSG)
#include "tweakwire_rpmsg.h"
#endif

#include <string.h>

tweak_wire_connection tweak_wire_create_connection(
    const char *connection_type, const char *params, const char *uri,
    tweak_wire_connection_state_listener connection_state_listener,
    void *connection_state_cookie, tweak_wire_receive_listener receive_listener,
    void *receive_listener_cookie)
{
   if (!connection_type) {
      return TWEAK_WIRE_INVALID_CONNECTION;
   }

#if defined(WITH_WIRE_NNG)
   if (strcmp(connection_type, "nng") == 0) {
      return tweak_wire_create_nng_connection(connection_type, params, uri,
            connection_state_listener, connection_state_cookie, receive_listener,
            receive_listener_cookie);
   }
#endif
#if defined(WITH_WIRE_RPMSG)
   if (strcmp(connection_type, "rpmsg") == 0) {
      return tweak_wire_create_rpmsg_connection(connection_type, params, uri,
            connection_state_listener, connection_state_cookie, receive_listener,
            receive_listener_cookie);
   }
#endif

   return TWEAK_WIRE_INVALID_CONNECTION;
}

tweak_wire_error_code tweak_wire_transmit(tweak_wire_connection connection,
  const uint8_t *buffer, size_t size)
{
  return connection->transmit_proc(connection, buffer, size);
}

void tweak_wire_destroy_connection(tweak_wire_connection connection) {
  if (connection) {
    connection->destroy_proc(connection);
  }
}
