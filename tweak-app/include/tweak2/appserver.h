/**
 * @file appserver.h
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

#ifndef TWEAK_APP_SERVER_INCLUDED
#define TWEAK_APP_SERVER_INCLUDED

#include <tweak2/appcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  /**
   * @brief Opaque pointer to pass into callback declared below.
   */
  void* cookie;
  /**
   * @see tweak_app_on_current_value_changed_callback.
   */
  tweak_app_on_current_value_changed_callback on_current_value_changed;
} tweak_app_server_callbacks;

/**
 * @brief Server context instance. Can be upcasted to tweak_app_context with C-style cast.
 *
 * @see tweak_app_context.
 */
typedef tweak_app_context tweak_app_server_context;

/**
 * @brief Spawn a server context instance using provided parameters.
 *
 * @param connection_type One of "nng", "serial". Type is case-sensitive.
 * @param params Additional params for backend seperated by semicolon ';'.
 * Only mutually exclusive "role=server" and "role=client"
 * are currently recognized for IP-based connections.
 * @param uri address to start server on. Format is defined by backend.
 *
 * @param on_current_value_changed listener for on_current_value_changed events
 * initiated by connected client.
 *
 * @return a valid server context or NULL.
 */
tweak_app_server_context tweak_app_create_server_context(const char *context_type, const char *params,
  const char *uri, const tweak_app_server_callbacks* callbacks);

/**
 * @brief Adds new item into the set of available tweaks.
 *
 * @details Item is identified by fully-qualified name in form of URI.
 * URI shall be unique, attempt to add another item with same URI will fail.
 * The current_value field of an item could be observed and altered by both
 * client and server when connection is established. Item's metadata
 * fields such as @p uri, @p description, @p meta, @p type are
 * set during initialization and and are read-only.
 *
 * @param server_context server context to which item is being added.
 *
 * @param uri Unique uri of an item within tweak collection.
 * Typically, it is a freeform string. GUI shall decide where to display
 * controls altering tweak's value according to this URI.
 * Initial proposal is to use _/group/subgroup/item_name_ notation.
 * You can get id of a tweak by using tweak_find_id() call.
 *
 * @param description textual description of an item.
 * The intended application of this string is to be displayed
 * as a user hint in a GUI. Other applications could exist
 * assuming this string is a human readable description of
 * this item's purpose.
 *
 * @param meta freeform representation hint that should be understood
 * by the gui. For instance, "slider", "checkbox", "text_area". Along
 * with @p uri, this parameter should be used by GUI application to
 * construct GUI control to edit this value in most natural way.
 * Their principal difference is that @p uri is unique whereas
 * @p meta is not.
 *
 * @param initial_value item's value for current_value and
 * default_value fields. The current_value field could be
 * altered later on, whilst default_value is permanent. *
 * @return tweak_id of a newly created item or TWEAK_INVALID_ID if there was an error.
 * Debug build would issue an error log describing what's happened.
 */
tweak_id tweak_app_server_add_item(tweak_app_server_context server_context,
  const char* uri, const char* description, const char* meta,
  tweak_variant* initial_value, void* item_cookie);

/**
 * @brief Retrieving user value provided as item_cookie parameter to
 * @see tweak_app_server_add_item.
 *
 * @details Item is identified by fully-qualified name in form of URI.
 * URI shall be unique, attempt to add another item with same URI will fail.
 * The current_value field of an item could be observed and altered by both
 * client and server when connection is established. Item's metadata
 * fields such as @p uri, @p description, @p meta, @p type are
 * set during initialization and and are read-only.
 *
 * @param server_context server context in which item is being held.
 *
 * @param id id of an item being accessed.
 *
 * @return value of item_cookie parameter provided to
 * @see tweak_app_server_add_item.
 */
void* tweak_app_item_get_cookie(tweak_app_server_context server_context, tweak_id id);

/**
 * @brief remove an item from internal collection given its @p id.
 *
 * @details This method won't block unless there's an IO request overflow.
 * The client, if it's present, shall be notified about item removal
 * asynchronously.
 *
 * @note if there are active client's subscriptions, item with given id shall
 * be removed from underlying collection after sending remove_item
 * request to client. Otherwise, item shall be removed immediately.
 *
 * @param server_context server context from which item is being removed.
 *
 * @param id id of an item being removed.
 *
 * @return true if item has been removed, false if there's no item with given @p id.
 */
bool tweak_app_server_remove_item(tweak_app_server_context context, tweak_id id);

#ifdef __cplusplus
}
#endif

#endif // TWEAK_APP_SERVER_INCLUDED
