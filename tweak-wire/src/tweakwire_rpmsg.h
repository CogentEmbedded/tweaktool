/**
 * @file tweakwire_rpmsg.h
 * @ingroup tweak-internal
 *
 * @brief RPMessage transport API for tweak wire.
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
