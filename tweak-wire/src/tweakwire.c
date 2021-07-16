/**
 * @file tweakwire.c
 * @ingroup tweak-internal
 *
 * @brief Tweak wire transport layer implementation, abstract connection factory.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include "tweak2/wire.h"

#ifdef WITH_WIRE_NNG
#include "tweakwire_nng.h"
#endif

#ifdef WITH_WIRE_RPMSG
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

#ifdef WITH_WIRE_NNG
   if (strcmp(connection_type, "nng") == 0) {
      return tweak_wire_create_nng_connection(connection_type, params, uri,
            connection_state_listener, connection_state_cookie, receive_listener,
            receive_listener_cookie);
   }
#endif
#ifdef WITH_WIRE_RPMSG
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
