/**
 * @file appclient.h
 * @ingroup tweak-api
 *
 * @brief part of tweak2 application interface.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

 /**
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_APP_CLIENT_INCLUDED
#define TWEAK_APP_CLIENT_INCLUDED

#include <tweak2/appcommon.h>

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

#ifdef __cplusplus
}
#endif

#endif // TWEAK_APP_CLIENT_INCLUDED
