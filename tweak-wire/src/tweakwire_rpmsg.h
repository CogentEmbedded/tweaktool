/**
 * @file tweakwire_rpmsg.h
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

#ifndef TWEAKWIRE_RPMSG_H_INCLUDED
#define TWEAKWIRE_RPMSG_H_INCLUDED

#include <tweak2/wire.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TWEAKWIRE_RPMSG_ESCAPE "\033"

#define TWEAKWIRE_RPMSG_SEND_DISCONNECTED TWEAKWIRE_RPMSG_ESCAPE"disconnect"

/**
 * @brief Create connection using rpmsg as a backend.
 */
tweak_wire_connection
tweak_wire_create_rpmsg_connection(
    const char *connection_type, const char *params, const char *uri,
    tweak_wire_connection_state_listener connection_state_listener,
    void *connection_state_cookie, tweak_wire_receive_listener receive_listener,
    void *receive_listener_cookie);

#ifdef __cplusplus
}
#endif

#endif /* TWEAKWIRE_RPMSG_H_INCLUDED */
