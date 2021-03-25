/**
 * @file tweakmodel_uri_to_tweak_id_index.h
 * @ingroup tweak-api
 *
 * @brief part of tweak2 application implementation.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
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

struct tweak_model_uri_to_tweak_id_index_base {};

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
