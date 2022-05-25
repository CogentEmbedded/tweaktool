/**
 * @file pickle_client.h
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

#ifndef TWEAK_PICKLE_CLIENT_H_INCLUDED
#define TWEAK_PICKLE_CLIENT_H_INCLUDED

#include <tweak2/pickle.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Client endpoint instance.
 */
typedef struct tweak_pickle_endpoint_base *tweak_pickle_client_endpoint;

/**
 * @brief Signature of callback to invoke user implementation of
 * @p tweak_pickle_server_add_item call.
 *
 * @param[in] add_item pointer to structure containing all fields of the
 * item being created.
 * @param[in] cookie an opaque pointer to user context.
 */
typedef void (*tweak_pickle_add_item_callback)(
  tweak_pickle_add_item *add_item, void *cookie);

/**
 * @brief Listener for tracking addition
 * of new items to subscription scope of the current
 * tweak-pickle connection.
 *
 * @details After subscription scope is created
 * by successful invocation of @p tweak_pickle_client_subscribe,
 * a valid implementation of @p tweak-pickle client is expected
 * to maintain its own replica of the subscription scope
 * by tracking addition of items with a user defined
 * callback procedure.
 *
 * Typically, the pointer to the data replica could be passed
 * as a @p cookie pointer, and @p callback could reference
 * a procedure that creates an items's representation within
 * the replica with callbacks's @p add_item parameter.
 */
typedef struct {

  /**
   * @brief User provided callback for addition of new item
   * to current subscription scope.
   */
  tweak_pickle_add_item_callback callback;

  /**
   * @brief a pointer to a user defined data structure
   * encapsulating context for the @p callback.
   */
  void *cookie;
} tweak_pickle_add_item_listener;

/**
 * @brief Signature of callback to invoke user implementation of
 * @p tweak_pickle_server_remove_item call.
 *
 * @param[in] remove_item a structure containing identity of
 * an item being removed.
 * @param[in] cookie an opaque pointer to user context.
 */
typedef void (*tweak_pickle_remove_item_callback)(tweak_pickle_remove_item* remove_item,
  void *cookie);

/**
 * @brief Listener for tracking removal
 * of items from subscription scope of the current
 * tweak-pickle connection.
 *
 * @details A valid implementation of @p tweak-pickle client
 * is expected to maintain its own replica of subscription
 * scope by tracking removal of items with a user defined
 * callback procedure. Typically, the pointer to the
 * data replica could be passed as a @p cookie pointer,
 * and @p callback could reference a procedure that
 * searches in active subscription scope for an item
 * referenced by its @p id encapsuled in callback's
 * @p remove_item parameter and removes it from scope,
 * or simply marks it as unavailable, it's up to user.
 *
 * It's up to server implementation to decide does
 * it allow spawning of a new item identified by
 * previously used @p id or not.
 */
typedef struct {
  /**
   * @brief User provided callback for addition of new item
   * to current subscription scope.
   */
  tweak_pickle_remove_item_callback callback;

  /**
   * @brief a pointer to a user defined data structure
   * encapsulating context for the @p callback.
   */
  void *cookie;
} tweak_pickle_remove_item_listener;

/**
 * @brief Skeleton of the client endpoint implementation.
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
   * @brief Listener for add item event.
   *
   * @see tweak_pickle_add_item_listener.
   */
  tweak_pickle_add_item_listener add_item_listener;

  /**
   * @brief Listener for change current item value event.
   *
   * @see tweak_pickle_change_item_listener.
   */
  tweak_pickle_change_item_listener change_item_listener;

  /**
   * @brief Listener for remove item event.
   *
   * @see tweak_pickle_remove_item_listener.
   */
  tweak_pickle_remove_item_listener remove_item_listener;

  /**
   * @brief Optional connection state listener.
   *
   * @see tweak_pickle_connection_state_listener.
   */
  tweak_pickle_connection_state_listener connection_state_listener;
} tweak_pickle_client_skeleton;

/**
 * @brief Parameters for spawning of @p tweak_pickle_client_endpoint instance.
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
   * @brief Skeleton of the client endpoint implementation.
   *
   * @see tweak_pickle_client_skeleton_descriptor.
   */
  tweak_pickle_client_skeleton skeleton;
} tweak_pickle_client_descriptor;

/**
 * @brief Create RPC endpoint allowing user to implement client side of
 * the tweak protocol.
 *
 * @details This implementation is using @p tweak-wire for transport for
 * establishing PRC connection.
 *
 * Owner of the client RPC endpoint can:
 *  - subscribe to item values with @p tweak_pickle_client_subscribe call.
 *  - push item value changes to server endpoint with
 * @p tweak_pickle_client_change_item calls.
 *
 * Owner of the client RPC endpoint can't spawn new items on server
 * or remove existing ones.
 *
 * Client is expected to maintain its own replica of
 * the server's set of subscribed items by providing implementations for
 * listeners from @p tweak_pickle_client_skeleton structure.
 *
 * @param[in] client_descriptor A structure containing all parameters
 * needed to spawn an instance of client endpoint.
 *
 * @return client endpoint instance or @p NULL. Debug builds will print
 * possible cause of an error to @p stderr.
 */
tweak_pickle_client_endpoint tweak_pickle_create_client_endpoint(
  const tweak_pickle_client_descriptor* client_descriptor);

/**
 * @brief Subscribe to track state of items uri of which matches to pattern.
 *
 * @details This call could be the first action of the client after
 * endpoint is created and connection if client implements mandatory features only.
 * If it implements any extended features, it is expected to enumerate
 * supported features by prior @see tweak_pickle_client_announce_features call.
 *
 * After client created subscriptions, the server will send updates of items
 * uris of which matches to patterns provided by uri_patterns parameter.
 *
 * @param[in] client_endpoint Endpoint instance created by
 * @p tweak_pickle_create_client_endpoint call.
 * @param[in] subscribe not implemented in this release.
 * Assumed to be NULL, will expand to universal quantifier "*".
 *
 * @return @p TWEAK_PICKLE_SUCCESS if there wasn't any errors.
 * @p TWEAK_PICKLE_BAD_PARAMETER if @p client_endpoint is @p NULL.
 * @p TWEAK_PICKLE_REMOTE_ERROR if disconnected.
 */
tweak_pickle_call_result
  tweak_pickle_client_subscribe(tweak_pickle_client_endpoint client_endpoint,
    const tweak_pickle_subscribe* subscribe);

/**
 * @brief Notify server about set of features supported by this client.
 *
 * @details This call could be the first action of the client after
 * endpoint is created connection. If client called @see tweak_pickle_client_subscribe
 * as its first action, server would assume that tweak_pickle_client_announce_features
 * supports minimal set of features and won't notify it about its own features.
 *
 * @param[in] client_endpoint Endpoint instance created by
 * @p tweak_pickle_create_client_endpoint call.
 * @param[in] features supported by this client endpoint as semicolon delimited list.
 *
 * @return @p TWEAK_PICKLE_SUCCESS if there wasn't any errors.
 * @p TWEAK_PICKLE_REMOTE_ERROR if disconnected.
 */
tweak_pickle_call_result
  tweak_pickle_client_announce_features(tweak_pickle_client_endpoint client_endpoint,
    const tweak_pickle_features* features);

/**
 * @brief Push item's current value update to server side.
 *
 * @details Owner of client endpoint is able to update
 * current value of any given item by invoking this method.
 *
 * @param[in] client_endpoint Endpoint instance created by
 * @p tweak_pickle_create_client_endpoint call.
 * @param[in] change pair of tweak id and value being sent to server.
 * @return @p TWEAK_PICKLE_SUCCESS if there wasn't any errors.
 * @p TWEAK_PICKLE_BAD_PARAMETER if @p client_endpoint is @p NULL
 * or @p change is @p NULL
 * or @p change->tweak_id is equal to @p TWEAK_INVALID_ID
 * or @p change->value is equivalent to @p NULL variant value.
 * @p TWEAK_PICKLE_REMOTE_ERROR if disconnected.
 */
tweak_pickle_call_result
  tweak_pickle_client_change_item(tweak_pickle_client_endpoint client_endpoint,
    const tweak_pickle_change_item *change);

/**
 * @brief Destroy an endpoint and deallocate all resources associated with it.
 *
 * @param[in] client_endpoint Endpoint instance created by
 * @p tweak_pickle_create_client_endpoint call.
 */
void tweak_pickle_destroy_client_endpoint(tweak_pickle_client_endpoint client_endpoint);

#ifdef __cplusplus
}
#endif

#endif
