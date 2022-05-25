/**
 * @file tweakuriutil.h
 * @ingroup tweak-api
 *
 * @brief populate/filter/sort utilities for collections of tweak uris.
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

#ifndef TWEAK_APP_CL_TWEAKURIUTIL_INCLUDED
#define TWEAK_APP_CL_TWEAKURIUTIL_INCLUDED

#include <stddef.h>
#include <tweak2/appclient.h>

/**
 * @brief All methods in this operate on this struct.
 */
struct tweak_app_cl_tweak_uris_list {
  /**
   * @brief list of string pointers. Terminated by NULL.
   */
  const char** uris;
  /**
   * @brief size of uris array.
   */
  size_t size;
};

/**
 * @brief Creates filtered and sorted list of available items at given moment.
 *
 * @note List is populated while internal context's mutex being acquired thus
 * making the item set being held in immutable state during the process.
 * However, item add/removal requests might land immediately after the moment
 * this list being created, this making this list obsolete prior to return from this function.
 * That's why tweakappclient.h doesn't provide such method forcing user to use a callback interface
 * in the first place. However, this is a non-issue for interactive client application.
 * User would just get an error on attempt to access an item that's no longer exists.
 *
 * @param context tweak-app context for which list is being populated.
 * @param filter substring to to look for in each uri. Can be NULL to capture all items.

 * @return filtered and sorted list of available items.
 */
struct tweak_app_cl_tweak_uris_list* tweak_app_cl_create_sorted_tweak_uris_list_strstr(tweak_app_client_context context,
                                                                                       const char* filter);

/**
 * @brief Creates filtered and sorted list of available items at given moment.
 *
 * @note List is populated while internal context's mutex being acquired thus
 * making the item set being held in immutable state during the process.
 * However, item add/removal requests might land immediately after the moment
 * this list being created, this making this list obsolete prior to return from this function.
 * That's why tweakappclient.h doesn't provide such method forcing user to use a callback interface
 * in the first place. However, this is a non-issue for interactive client application.
 * User would just get an error on attempt to access an item that's no longer exists.
 *
 * @param context tweak-app context for which list is being populated.
 * @param filter POSIX compatible regex to filter tweak uri's. Assumed to be .* when user provides NULL.

 * @return filtered and sorted list of available items.
 */
struct tweak_app_cl_tweak_uris_list* tweak_app_cl_create_sorted_tweak_uris_list_regex(tweak_app_client_context context,
                                                                                      const char* filter);

/**
 * @brief Helper method to find nth tweak URI having @p prefix. For use in automatic completion routeines.
 *
 * @details Suppose we have list such as this:
 * ```
 * /1/2/3
 * /1/2/4
 * /a/b/c  match 0
 * /a/b/d  ...
 * /a/b/e
 * /a/b/f  match 3
 * /a/c/f  no match for /a/b prefix
 * ```
 * then call to this method with parameters "/a/b" as prefix and 3 as n shall give /a/b/f as result.
 * when n is 4, it shall return NULL since /a/c/f isn't prefixed by /a/b.
 *
 * n could be as big as MAX_INT since array out of bounds condition shall result in NULL being returned.
 *
 * @param uris_list tweak_app_cl_tweak_uris_list instance.
 * @param prefix uri prefix. Cannot be NULL. Empty string mathes begin of the list.
 * @param n_match number of match.
 *
 * @return nth string in the sorted list having prefix equal to @p prefix.
 */
const char* tweak_app_cl_tweak_uris_list_pick_nth_match(struct tweak_app_cl_tweak_uris_list* uris_list,
                                                        const char* prefix,
                                                        int n_match);

/**
 * @brief Release all dynamic memory allocated by tweak_app_cl_create_sorted_tweak_uris_list_regex.
 *
 * @param tweak_uris_list tweak_app_cl_tweak_uris_list instance.
 */
void tweak_app_cl_destroy_sorted_tweak_uris_list(struct tweak_app_cl_tweak_uris_list* tweak_uris_list);

#endif
