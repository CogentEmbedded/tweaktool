/**
 * @file tweakwire_nng.h
 * @ingroup tweak-internal
 *
 * @brief Tweak wire transport layer implementation, NNG backendd.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#ifndef TWEAK_WIRE_NNG_H_INCLUDED
#define TWEAK_WIRE_NNG_H_INCLUDED

#include <tweak2/wire.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create connection using nng as a backend.
 */
tweak_wire_connection tweak_wire_create_nng_connection(
    const char *connection_type, const char *params, const char *uri,
    tweak_wire_connection_state_listener connection_state_listener,
    void *connection_state_cookie, tweak_wire_receive_listener receive_listener,
    void *receive_listener_cookie);

#ifdef __cplusplus
}
#endif

#endif /* TWEAK_WIRE_NNG_H_INCLUDED */

