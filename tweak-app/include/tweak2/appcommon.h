/**
 * @file appcommon.h
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

#ifndef TWEAK2_APP_COMMON_INCLUDED
#define TWEAK2_APP_COMMON_INCLUDED

#include <tweak2/string.h>
#include <tweak2/types.h>
#include <tweak2/variant.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes for tweak app calls.
 */
typedef enum {
 /**
  * @brief Expected result of an operation.
  */
  TWEAK_APP_SUCCESS = 0,
 /**
  * @brief Context is disconnected from the peer.
  * Returning last known value.
  */
  TWEAK_APP_SUCCESS_LAST_KNOWN_VALUE,
 /**
  * @brief requested item id not found.
  */
  TWEAK_APP_ITEM_NOT_FOUND,
 /**
  * @brief attempt to assign value to an item
  * that differs from type of an initial value.
  *
  * @note if it's really needed, remove item and
  * introduce a new one with the same uri.
  */
  TWEAK_APP_TYPE_MISMATCH,
  /**
   * @brief Provided argument doesn't meet certain requirements.
   */
  TWEAK_APP_INVALID_ARGUMENT,
  /**
   * @brief Context can't propagate updated value since connection
   * is no longer available.
   */
  TWEAK_APP_PEER_DISCONNECTED
} tweak_app_error_code;

/**
 * @brief A container for all fields in the model.
 */
typedef struct {
  /**
   * @brief tweak id. Generated by server as result in one of tweak_add_* calls.
   */
  tweak_id id;
  /**
   * @brief Unique uri. Provided by server in one of tweak_add_* calls.
   *
   * Client is supposed to understand format of this string and use it
   * in control placement in conjunction with meta.
   */
  tweak_variant_string uri;
  /**
   * @brief Description. Provided by server in one of tweak_add_* calls.
   *
   * Textual description of an item. Supposedly contain a hint for the GUI.
   */
  tweak_variant_string description;
  /**
   * @brief Meta. Provided by server in one of tweak_add_* calls.
   *
   * Client is supposed to understand format of this string and use it
   * in control placement in conjunction with uri.
   *
   * @note uri is unique whereas meta is not.
   */
  tweak_variant_string meta;
  /**
   * @brief default_value. Provided by server in one of tweak_add_* calls.
   *
   * Client could implement "reset" use-case by referring to these values.
   */
  tweak_variant default_value;
  /**
   * @brief current_value. Provided by server in one of tweak_add_* calls.
   *
   * Client could implement "reset" use-case by referring to these values.
   */
  tweak_variant current_value;
} tweak_app_item_snapshot;

/**
 * @brief Forward declaration of base class for all context types.
 */
struct tweak_app_context_base;

/**
 * @brief Prototype for a callback to capture items enumerated by by tweak_app_traverse_items method.
 *
 * @param snapshot structure capturing all item's fields.
 * @param cookie opaque pointer provided by user.
 *
 * @return true to continue iteration, false to abort.
 */
typedef bool (*tweak_app_traverse_items_callback)(const tweak_app_item_snapshot* snapshot,
  void* cookie);

/**
 * @brief Instance encapsulating an abstract application context.
 */
typedef struct tweak_app_context_base *tweak_app_context;

 /**
  * @brief Callback for tracking changes of existing items' current_value field.
  *
  * @param context context on which this event had occurred.
  * @param id id of item to remove.
  * @param value new value. This instance is movable. User could get ownership
  * by using tweak_variant_swap with some other value.
  *
  * @param cookie arbitrary pointer provided by user.
  */
typedef void (*tweak_app_on_current_value_changed_callback)(tweak_app_context context,
  tweak_id id, tweak_variant* value, void *cookie);

/**
 * @brief Get tweak id by its uri.
 *
 * @param context an application context.
 * @param uri uri to find.
 *
 * @return tweak id or INVALID_TWEAK_ID if the key isn't found.
 */
tweak_id tweak_app_find_id(tweak_app_context context, const char* uri);

/**
 * @brief Enumerate all items in this context.
 *
 * @param context an application context.
 * @param callback callbeck to capture items being enumerated.
 * @param cookie opaque pointer provided by user.
 */
bool tweak_app_traverse_items(tweak_app_context context,
  tweak_app_traverse_items_callback callback, void* cookie);

/**
 * @brief Get a snapshot of an item's state at the current moment.
 *
 * @note User is responsible to release the snapshot using
 * tweak_app_release_attributes call, otherwise this will
 * introduce a memory leak.
 *
 * @param context an application context.
 * @param id tweak id.
 *
 * @return snapshot instance or NULL if tweak id is invalid.
 */
tweak_app_item_snapshot* tweak_app_item_get_snapshot(tweak_app_context context,
  tweak_id id);

/**
 * @brief Get an item's type.
 *
 * @param context an application context.
 * @param id tweak id.
 *
 * @return type of a given item or TWEAK_VARIANT_TYPE_IS_NULL if there's no such item.
 *
 * @note It is assumed that items having NULL value are useless, thus user isn't
 * allowed to create them.
 */
tweak_variant_type tweak_app_item_get_type(tweak_app_context context, tweak_id id);

/**
 * @brief Release a snapshot issued by tweak_app_item_get_snapshot.
 *
 * @param context an application context.
 * @param snapshot instance created by tweak_app_item_get_snapshot method.
 */
void tweak_app_release_snapshot(tweak_app_context context,
  tweak_app_item_snapshot* snapshot);

/**
 * @brief Clone item's value to designated tweak_variant instance.
 *
 * @note this method is more selective than tweak_app_item_get_attributes.
 *
 * @param context an application context.
 * @param id tweak id.
 * @param value a pointer to instance. Preexisting data will be released with
 * tweak_variant_destroy() call.
 *
 * @return TWEAK_APP_SUCCESS if there wasn't any errors.
 */
tweak_app_error_code tweak_app_item_clone_current_value(tweak_app_context context,
  tweak_id id, tweak_variant* value);

/**
 * @brief Swap item's value with provided @p value and propagate new item's value
 * to the connected peer, it there's one.
 *
 * @param context an application context.
 * @param id tweak id.
 * @param value a pointer to new value. After the call its contents will be replaced
 * by previous value of item. A user could repurpose the storage associated with
 * this tweak_variant instance or release it immediately with tweak_variant_destroy
 * call.
 *
 * @return TWEAK_APP_SUCCESS if there wasn't any errors.
 */
tweak_app_error_code tweak_app_item_replace_current_value(tweak_app_context context,
  tweak_id id, tweak_variant* value);

/**
 * @brief a virtual destructor for application context objects.
 */
void tweak_app_destroy_context(tweak_app_context context);

#ifdef __cplusplus
};
#endif

#endif //TWEAK2_APP_COMMON_H_INCLUDED
