/**
 * @file pickle.h
 * @ingroup tweak-internal
 *
 * @brief Remote procedure call implementation over transport layer provided by
 * weak2::wire library.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @ingroup tweak-internal Tweak Internals
 * Internal details of Tweak Tool implementation. May be useful for debugging,
 * extending and hacking.
 */

#ifndef TWEAK_PICKLE_H_INCLUDED
#define TWEAK_PICKLE_H_INCLUDED

#include <tweak2/string.h>
#include <tweak2/types.h>
#include <tweak2/variant.h>

/**
 * @ingroup tweak-internal
 *
 * @brief Invalid endpoint.
 */
#define TWEAK_PICKLE_INVALID_ENDPOINT (NULL)

/**
 * @brief A superclass for all endpoint types.
 */
struct tweak_pickle_endpoint_base {
  int dummy; /* Empty structures are implementation specific. */
             /* ANSI ISO C99 doesn't forbid nor explicitly allow them. */
             /* GreenHills C compiler, MSVC and GCC in pedantic mode */
             /* would issue a warning or an error */
             /* if this dummy placeholder is removed. */
};

/**
 * @brief This structure contains all fields for add tweak request.
 *
 * @details Add tweak request shall be implemented as stub on server side
 * and as skeleton/callback on client side.
 */
typedef struct {
  /**
   * @brief Internal tweak id.
   */
  tweak_id tweak_id;
  /**
   * @brief Fully qualified name.
   */
  tweak_variant_string uri;
  /**
   * @brief Detailed description.
   */
  tweak_variant_string description;
  /**
   * @brief Representation hints.
   */
  tweak_variant_string meta;
  /**
   * @brief Current value.
   */
  tweak_variant current_value;
  /**
   * @brief Default value.
   */
  tweak_variant default_value;
} tweak_pickle_add_item;

/**
 * @brief This structure contains all fields for change tweak request.
 *
 * @details Change tweak is a bidirectional request that should
 * be implemented as a stub and as a skeleton/callback on both
 * client and server sides.
 */
typedef struct {
  /**
   * @brief Internal tweak id.
   */
  tweak_id tweak_id;
  /**
   * @brief Updated value for tweak's @p current_value field.
   */
  tweak_variant value;
} tweak_pickle_change_item;

/**
 * @brief This structure contains all fields for remove tweak request.
 *
 * @details Remove tweak shall be implemented as a stub on server side
 * and as skeleton/callback on client side.
 */
typedef struct {
  /**
   * @brief Internal tweak id.
   */
  tweak_id tweak_id;
} tweak_pickle_remove_item;

/**
 * @brief This structure contains all fields for subscribe request.
 *
 * @details Subscribe shall be implemented as a stub on client side
 * and as skeleton/callback on server side.
 */
typedef struct {
  /**
   * @brief Uri patterns to subscribe separated by semicolon.
   *
   * @details Currently only "*" is supported.
   * Complex pattern collections such as "/aaaa/bbb*;/ccc/dddd*"
   * are planned for future releases.
   */
  tweak_variant_string uri_patterns;
} tweak_pickle_subscribe;

/**
 * @brief Error codes for tweak-pickle calls.
 */
typedef enum {
  /**
   * @brief Success.
   */
  TWEAK_PICKLE_SUCCESS = 0,
  /**
   * @brief Missing mandatory parameter or its
   * value is outside expected range.
   */
  TWEAK_PICKLE_BAD_PARAMETER,
  /**
   * @brief Transport layer error.
   */
  TWEAK_PICKLE_REMOTE_ERROR,
} tweak_pickle_call_result;

/**
 * @brief Tweak wire protocol connection status.
 */
typedef enum {
  /**
   * @brief Client is disconnected.
   */
  TWEAK_PICKLE_DISCONNECTED = 0,
  /**
   * @brief Client is connected.
   */
  TWEAK_PICKLE_CONNECTED
} tweak_pickle_connection_state;

/**
 * @brief Signature for a user defined callback
 * for tracking RPC connection state.
 *
 * @param[in] connection_state most recent connection state.
 * @param[in] cookie an opaque pointer to user context.
 */
typedef void (*tweak_pickle_connection_state_callback)(
    tweak_pickle_connection_state connection_state, void *cookie);

/**
 * @brief Connection state listener and its bound
 * @p cookie parameter.
 *
 * @details  If it's the last @p connection_state
 * registered with this listener has been equal to
 * @p TWEAK_PICKLE_CONNECTED then the next RPC call
 * should succeed assuming its parameters were valid
 * and underlying @p tweak-wire connection hasn't
 * been broken immediately after being established.
 */
typedef struct {
  /**
   * @brief User provided connection state listener, can be NULL.
   */
  tweak_pickle_connection_state_callback callback;
  /**
   * @brief a pointer to a user defined data structure
   * encapsulating context for the @p callback.
   */
  void *cookie;
} tweak_pickle_connection_state_listener;

/**
 * @brief Signature for a user defined callback for tracking current
 * value of items.
 *
 * @param[in] change @p tweak_id of an item and its new @p current_value.
 * @param[in] cookie an opaque pointer to user context.
 */
typedef void (*tweak_pickle_change_item_callback)(
  tweak_pickle_change_item *change, void *cookie);

/**
 * @brief Listener for tracking updates to @p current_value
 * field of items within established subscription scope.
 *
 * @details Listener for this event shall be provided
 * by implementations of both client and server endpoints
 * since both client and server could alter current value
 * of any item within share subscription scope.
 *
 * @note Server updates have priority.
 */
typedef struct {
  /**
   * @brief User provided item's @p current_value
   * change callback.
   */
  tweak_pickle_change_item_callback callback;

  /**
   * @brief a pointer to a user defined data structure
   * encapsulating context for the @p callback.
   */
  void *cookie;
} tweak_pickle_change_item_listener;


#endif
