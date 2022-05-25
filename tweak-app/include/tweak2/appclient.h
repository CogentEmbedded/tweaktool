/**
 * @file appclient.h
 * @ingroup tweak-api
 *
 * @brief part of tweak2 application interface.
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
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_APP_CLIENT_INCLUDED
#define TWEAK_APP_CLIENT_INCLUDED

#include <tweak2/appcommon.h>
#include <tweak2/thread.h> /* Timespan definitions */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Client context instance. Can be upcasted to tweak_app_context.
 *
 * @see tweak_app_context.
 */
typedef tweak_app_context tweak_app_client_context;

/**
 * @brief Callback for tracking additions of new items.
 *
 * @param context context on which this event had occurred.
 * @param id id of new item.
 * @param cookie arbitrary pointer provided by user.
 */
typedef void (*tweak_app_on_new_item_callback)(tweak_app_context context,
  tweak_id id, void *cookie);

 /**
  * @brief Callback for tracking removal of existing items.
  *
  * @param context context on which this event had occurred.
  * @param id id of item to remove.
  * @param cookie arbitrary pointer provided by user.
  */
typedef void (*tweak_app_on_item_removed_callback)(tweak_app_context context,
  tweak_id id, void *cookie);

 /**
  * @brief Callback for tracking connection state of a client.
  *
  * When server is disconnected, user won't be able to change item's
  * values. An attempt to access item's values shall issue
  * last known values of these items.
  *
  * @param context context on which this event had occurred.
  * @param is_connected true is server is online and accept values being updated
  * on client's side and notify client's side on updates happed on server side.
  * @param cookie arbitrary pointer provided by user.
  */
typedef void (*tweak_app_on_connection_status_changed_callback)(tweak_app_context context,
  bool is_connected, void *cookie);

/**
 * @brief Collection of listeners needed to initialize client context.
 */
typedef struct {
  /**
   * @brief Opaque pointer to pass into callbacks declared below.
   */
  void* cookie;
  /**
   * @see tweak_app_on_connection_status_changed_callback.
   */
  tweak_app_on_connection_status_changed_callback on_connection_status_changed;
  /**
   * @see tweak_app_on_new_item_callback.
   */
  tweak_app_on_new_item_callback on_new_item;
  /**
   * @see tweak_app_on_current_value_changed_callback.
   */
  tweak_app_on_current_value_changed_callback on_current_value_changed;
  /**
   * @see tweak_app_on_item_removed_callback.
   */
  tweak_app_on_item_removed_callback on_item_removed;
} tweak_app_client_callbacks;

/**
 * @brief Spawn a client context instance using provided parameters.
 *
 * @param connection_type One of "nng", "serial". Type is case-sensitive.
 * @param params Additional params for backend seperated by semicolon ';'.
 * Only mutually exclusive "role=server" and "role=client"
 * are currently recognized for IP-based connections.
 * @param uri address to start server on. Format is defined by backend.
 * @param client_listeners A structure containing all listeners.
 *
 * @return a valid client context or NULL.
 *
 * @see tweak_wire_create_connection
 */
tweak_app_client_context tweak_app_create_client_context(const char *context_type, const char *params,
  const char *uri, const tweak_app_client_callbacks* client_listeners);

/**
 * @brief Waits until client connection could be used to access all @p uris from given list
 * for given @p timeout.
 *
 * @param client_context tweak client context created @see by tweak_app_create_client_context.
 * @param uris list of uris to wait for.
 * @param uris_size of uris array.
 * @param tweak_ids Optional output list of tweak ids, must have size equal to size of @p uris array.
 * Can be NULL if not needed.
 * @param timeout timespan in milliseconds.
 *
 * @return TWEAK_APP_SUCCESS if all @p uris are available or TWEAK_APP_TIMEOUT if some of
 * @p uris are still inaccessible after @p timeout milliseconds.
 */
tweak_app_error_code tweak_app_client_wait_uris(tweak_app_client_context client_context,
  const char** uris, size_t uris_size, tweak_id* tweak_ids, tweak_common_milliseconds timeout_millis);

#ifdef __cplusplus
}
#endif

#endif // TWEAK_APP_CLIENT_INCLUDED
