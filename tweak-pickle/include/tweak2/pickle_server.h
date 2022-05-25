/**
 * @file pickle_server.h
 * @ingroup tweak-internal
 *
 * @brief Remote procedure call implementation over transport layer provided by
 * weak2::wire library.
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
 * @ingroup tweak-internal Tweak Internals
 * Internal details of Tweak Tool implementation. May be useful for debugging,
 * extending and hacking.
 */

#ifndef TWEAK_PICKLE_SERVER_H_INCLUDED
#define TWEAK_PICKLE_SERVER_H_INCLUDED

#include <tweak2/pickle.h>
#include <tweak2/string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Server endpoint instance.
 */
typedef struct tweak_pickle_endpoint_base *tweak_pickle_server_endpoint;

/**
 * @brief Signature of a callback to invoke user implementation of
 * @p tweak_pickle_client_subscribe call.
 *
 * @param[in] subscribe Contains set of uri predicates
 * to create a subscription scope on server side.
 * Assumed to always contain universal quantifier "*" in this release.
 * @param[in] cookie cookie an opaque pointer to user context.
 */
typedef void (*tweak_pickle_subscribe_callback)(tweak_pickle_subscribe* subscribe,
  void *cookie);

/**
 * @brief Listener for spawining of client's subscription
 * scope event.
 *
 * @details A valid implementation of @p tweak-pickle server
 * is expected to keep client side in sync by providing
 * updates to changes of items from subscription scope
 * selected by client. Initially, when connection state
 * tracked by @p tweak_pickle_connection_state_listener
 * is changed from @p TWEAK_PICKLE_DISCONNECTED to
 * @p TWEAK_PICKLE_CONNECTED, this scope is assumed
 * to be empty. User can expand it by invoking
 * @p tweak_pickle_client_subscribe with uri predicates.
 *
 * @note _Subscription scope_ is a set of items having
 * @p uri fields that match predicates provided to
 * @p tweak_pickle_client_subscribe call.
 *
 * A valid server implementation shall
 *
 * - Start enumerating all items in the new subscription
 * scope by invoking  sequence of @p tweak_pickle_server_add_item
 * calls within a reasonably short timeframe following
 * activation of this listener.
 *
 * - Notify connected client about all events associated with
 * lifecycle of items within subscription scope with
 * @p tweak_pickle_server_add_item, @p tweak_pickle_server_change_item,
 * and @p tweak_pickle_server_remove_item calls while
 * underlying @p tweak-wire connection is alive,
 * as registered by @p tweak_pickle_connection_state_listener.
 */
typedef struct {
  /**
   * @brief User provided callback for selection of items
   * into a session-wide subscription scope.
   */
  tweak_pickle_subscribe_callback callback;

  /**
   * @brief Pointer to a user defined data structure
   * encapsulating context for the @p callback.
   */
  void *cookie;
} tweak_pickle_subscribe_listener;

/**
 * @brief Skeleton of the server endpoint implementation.
 *
 * @details User implementation of this skeleton's
 * methods is bound to an endpoint dynamically using
 * listeners referenced here.
 */
typedef struct {
  /**
   * @brief Listener for features sets.
   *
   * @see tweak_pickle_features_listener.
   */
  tweak_pickle_features_listener announce_features_listener;
  /**
   * @brief Listener for subscribe event.
   *
   * @see tweak_pickle_subscribe_listener.
   */
  tweak_pickle_subscribe_listener subscribe_listener;

 /**
   * @brief Listener for change current item value event.
   *
   * @see tweak_pickle_change_item_listener.
   */
  tweak_pickle_change_item_listener change_item_listener;

  /**
   * @brief Connection state listener.
   *
   * @see tweak_pickle_connection_state_listener.
   */
  tweak_pickle_connection_state_listener connection_state_listener;
} tweak_pickle_server_skeleton;


/**
 * @brief Parameters for spawning of @p tweak_pickle_server_endpoint instance.
 */
typedef struct {
  /**
   * @brief @p context_type connection parameter for @p tweak-wire.
   * This parameter is forwarded to the underlying layer as is.
   */
  const char* context_type;

  /**
   * @brief @p params connection parameter for @p tweak-wire.
   * This parameter is forwarded to the underlying layer as is.
   */
  const char* params;

  /**
   * @brief @p uri connection parameter for @p tweak-wire.
   * This parameter is forwarded to the underlying layer as is.
   */
  const char* uri;

  /**
   * @brief Skeleton of the server endpoint implementation.
   *
   * @see tweak_pickle_server_skeleton_descriptor.
   */
  tweak_pickle_server_skeleton skeleton;
} tweak_pickle_server_descriptor;


/**
 * @brief Create RPC endpoint allowing user to implement server side of
 * the tweak protocol.
 *
 * @details This implementation is using @p tweak-wire for transport for
 * establishing PRC connection.
 *
 * Owner of the server RPC endpoint is expected to:
 *
 *  - Respond to subscribe request with providing the full list
 * of items matching subscription patterns by
 * @p tweak_pickle_server_add_item calls.
 *
 *  - Keep client's subscription scope in sync by providing updates
 * to item list with @p tweak_pickle_server_add_item
 * @p tweak_pickle_server_remove_item, and
 * @p tweak_pickle_server_change_item calls.
 *
 *  - Accept client updates to @p current_value
 * fields of items by implementing @p change_tweak_proc callback.
 *
 * @param[in] server_descriptor A structure containing all parameters
 * needed to spawn an instance of server endpoint.
 *
 * @return valid endpoint instance or or @p NULL. Debug builds will print
 * possible cause of an error to @p stderr.
 */
tweak_pickle_server_endpoint tweak_pickle_create_server_endpoint(
    const tweak_pickle_server_descriptor* server_descriptor);


/**
 * @brief Notify client about set of features supported by this server.
 *
 * @details This call is expected to be the first server call if client
 * initiated the session by @see tweak_pickle_client_announce_features call.
 * It must be invoked prior to any @see tweak_pickle_server_add_item calls in this case.
 * features listed here must be the intersection of features supported by both sides.
 *
 * @param[in] server_endpoint Endpoint instance created by
 * @p tweak_pickle_create_server_endpoint call.
 *
 * @param[in] features supported by this client endpoint as semicolon delimited list.
 * NULL is equivalent to string "scalar".
 *
 * @return @p TWEAK_PICKLE_SUCCESS if there wasn't any errors.
 * @p TWEAK_PICKLE_REMOTE_ERROR if disconnected.
 */
tweak_pickle_call_result
  tweak_pickle_server_announce_features(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pickle_features* features);

/**
 * @brief Call this in order to forward item creation event to client endpoint.
 *
 * @param[in] server_endpoint Endpoint instance created by
 * @p tweak_pickle_create_server_endpoint call.
 * @param[in] add_item record containing all fields of the tweak.
 * @return @p TWEAK_PICKLE_SUCCESS if there wasn't any errors.
 * @p TWEAK_PICKLE_BAD_PARAMETER if @p server_endpoint is @p NULL
 * or @p add_item is @p NULL
 * or @p add_item->tweak_id is equal to @p TWEAK_INVALID_ID
 * or @p add_item->uri is equivalent to @p NULL variant value.
 * or @p add_item->current_value is equivalent to @p NULL variant value.
 * @p TWEAK_PICKLE_REMOTE_ERROR if disconnected.
 */
tweak_pickle_call_result
  tweak_pickle_server_add_item(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pickle_add_item *add_item);

/**
 * @brief Push tweak value update to client side.
 *
 * @details Owner of server endpoint is expected to keep current values in
 * client replica in sync by using this method.
 * @param[in] server_endpoint Endpoint instance created by
 * @p tweak_pickle_create_server_endpoint call.
 * @param[in] change pair of tweak id and value being sent to client.
 *
 * @return @p TWEAK_PICKLE_SUCCESS if there wasn't any errors.
 * @p TWEAK_PICKLE_BAD_PARAMETER if @p server_endpoint is @p NULL
 * or if @p change is @p NULL
 * or @p change->tweak_id is equal to @p TWEAK_INVALID_ID
 * or @p change->value is equivalent to @p NULL variant value.
 * @p TWEAK_PICKLE_REMOTE_ERROR if disconnected.
 */
tweak_pickle_call_result
  tweak_pickle_server_change_item(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pickle_change_item *change);

/**
 * @brief Call this in order to forward item removal event to client endpoint.
 *
 * @param[in] server_endpoint Endpoint instance created by
 * @p tweak_pickle_create_server_endpoint call.
 * @param[in] remove_item A container with id of item being removed.
 * @return @p TWEAK_PICKLE_SUCCESS if there wasn't any errors.
 * @p TWEAK_PICKLE_BAD_PARAMETER if @p server_endpoint is @p NULL
 * or @p remove_item->tweak_id is equal to @p TWEAK_INVALID_ID.
 * @p TWEAK_PICKLE_REMOTE_ERROR if disconnected.
 */
tweak_pickle_call_result
  tweak_pickle_server_remove_item(tweak_pickle_server_endpoint server_endpoint,
    const tweak_pickle_remove_item* remove_item);

/**
 * @brief Destroy an endpoint and deallocate all resources associated with it.
 *
 * @param[in] server_endpoint An endpoint instance created by
 * @p tweak_pickle_create_server_endpoint call.
 */
void tweak_pickle_destroy_server_endpoint(
    tweak_pickle_server_endpoint server_endpoint);

#ifdef __cplusplus
}
#endif

#endif
