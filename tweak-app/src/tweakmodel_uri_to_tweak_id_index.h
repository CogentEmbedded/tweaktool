/**
 * @file tweakmodel_uri_to_tweak_id_index.h
 * @ingroup tweak-api
 *
 * @brief part of tweak2 application implementation.
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

#ifndef TWEAK_MODEL_URI_2_TWEAK_ID_INDEX_INCLUDED
#define TWEAK_MODEL_URI_2_TWEAK_ID_INDEX_INCLUDED

#include <tweak2/types.h>

/**
 * @brief Result of an index operation.
 */
typedef enum {
 /**
  * @brief Success.
  */
  TWEAK_MODEL_INDEX_SUCCESS = 0,
  /**
   * @brief Unique constraint violation.
   */
  TWEAK_MODEL_INDEX_KEY_ALREADY_EXISTS,
  /**
   * @brief Lookup operation error.
   */
  TWEAK_MODEL_INDEX_KEY_NOT_FOUND,
  /**
   * @brief Can't allocate memory for a new entry in the index.
   */
  TWEAK_MODEL_INDEX_NO_MEMORY,
} tweak_model_index_result;

struct tweak_model_uri_to_tweak_id_index_base {
  int dummy; /* Empty structures are implementation specific. */
             /* ANSI ISO C99 doesn't forbid nor explicitly allow them. */
             /* GreenHills C compiler, MSVC and GCC in pedantic mode */
             /* would issue a warning or an error */
             /* if this dummy placeholder is removed. */
};

typedef struct tweak_model_uri_to_tweak_id_index_base *tweak_model_uri_to_tweak_id_index;
/**
 * @brief Create new index instance.
 *
 * @return new index instance or NULL.
 */
tweak_model_uri_to_tweak_id_index tweak_model_uri_to_tweak_id_index_create();

/**
 * @brief Associates a unique string with given id.
 *
 * @param index index instance.
 * @param uri key to lookup.
 * @param id id to associate with uri.
 *
 * @return TWEAK_MODEL_INDEX_SUCCESS if operation was successful.
 */
tweak_model_index_result
tweak_model_uri_to_tweak_id_index_insert(tweak_model_uri_to_tweak_id_index index,
  const char *uri, tweak_id id);

/**
 * @brief Looks for a given uri and returns id it associated with.
 *
 * @param index index instance.
 * @param uri key to lookup.
 *
 * @return id or TWEAK_INVALID_ID if there wasn't such item.
 */
tweak_id tweak_model_uri_to_tweak_id_index_lookup(tweak_model_uri_to_tweak_id_index index,
  const char *uri);

/**
 * @brief Callback to capture items stored in index.
 *
 * @param uri key index key.
 * @param id id associated with @p uri.
 * @param cookie opaque pointer passed as is.
 *
 * @return true to continue iteration, false to abort.
 */
typedef bool (*tweak_model_uri_to_tweak_id_walk_proc)(const char *uri, tweak_id id, void* cookie);

/**
 * @brief Enumerate all items in index.
 *
 * @param index index instance.
 * @param walk_proc callback to capture index entries.
 * @param cookie opaque pointer to pass into @p walk_proc.
 *
 * @return true if iteration hasn't been aborted.
 */
bool tweak_model_uri_to_tweak_id_index_walk(tweak_model_uri_to_tweak_id_index index,
  tweak_model_uri_to_tweak_id_walk_proc walk_proc, void* cookie);

/**
 * @brief Remove an item from index.
 *
 * @param index index instance.
 * @param uri key.
 *
 * @return TWEAK_MODEL_INDEX_SUCCESS if operation was successful.
 */
tweak_model_index_result tweak_model_uri_to_tweak_id_index_remove(tweak_model_uri_to_tweak_id_index index,
  const char *uri);

/**
 * @brief Destroy index and deallocate all resources associated with it.
 *
 * @param index index instance.
 */
void tweak_model_uri_to_tweak_id_index_destroy(tweak_model_uri_to_tweak_id_index index);

#endif
