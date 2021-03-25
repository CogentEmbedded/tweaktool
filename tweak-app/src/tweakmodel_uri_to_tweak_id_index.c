/**
 * @file tweakmodel_uri_to_tweak_id_index.c
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

#include <tweak2/string.h>
#include <tweak2/types.h>

#include "tweakmodel_uri_to_tweak_id_index.h"

#include <assert.h>
#include <limits.h>
#include <uthash.h>
struct uri_tweak_id_pair {
  char* key;
  tweak_id tweak_id;
  UT_hash_handle hh;
};

struct tweak_model_uri_to_tweak_id_index_impl {
  struct tweak_model_uri_to_tweak_id_index_base base;
  struct uri_tweak_id_pair* pairs;
};

tweak_model_uri_to_tweak_id_index tweak_model_uri_to_tweak_id_index_create() {
  struct tweak_model_uri_to_tweak_id_index_impl* index_impl = calloc(1, sizeof(*index_impl));
  return index_impl ? &index_impl->base : NULL;
}

tweak_model_index_result tweak_model_uri_to_tweak_id_index_insert(tweak_model_uri_to_tweak_id_index index,
  const char *uri, tweak_id tweak_id)
{
  struct tweak_model_uri_to_tweak_id_index_impl* index_impl = (struct tweak_model_uri_to_tweak_id_index_impl*)index;
  struct uri_tweak_id_pair* pair = NULL;
  HASH_FIND_STR(index_impl->pairs, uri, pair);
  if (pair != NULL) {
    return TWEAK_MODEL_INDEX_KEY_ALREADY_EXISTS;
  }
  pair = malloc(sizeof(*pair));
  if (pair == NULL) {
    return TWEAK_MODEL_INDEX_NO_MEMORY;
  }
  pair->tweak_id = tweak_id;
  size_t length = strlen(uri);
  pair->key = malloc(length + 1);
  if (pair->key == NULL) {
    free(pair);
    return TWEAK_MODEL_INDEX_NO_MEMORY;
  }
  strncpy(pair->key, uri, length + 1);
  HASH_ADD_KEYPTR(hh, index_impl->pairs, pair->key, length, pair);
  return TWEAK_MODEL_INDEX_SUCCESS;
}

bool tweak_model_uri_to_tweak_id_index_walk(tweak_model_uri_to_tweak_id_index index,
  tweak_model_uri_to_tweak_id_walk_proc walk_proc, void* cookie)
{
  bool result = true;
  struct tweak_model_uri_to_tweak_id_index_impl* index_impl = (struct tweak_model_uri_to_tweak_id_index_impl*)index;
  struct uri_tweak_id_pair *pair = NULL;
  struct uri_tweak_id_pair *tmp = NULL;
  HASH_ITER(hh, index_impl->pairs, pair, tmp) {
    if (!walk_proc(pair->key, pair->tweak_id, cookie)) {
      result = false;
      break;
    }
  }
  return result;
}

tweak_id tweak_model_uri_to_tweak_id_index_lookup(tweak_model_uri_to_tweak_id_index index,
  const char *uri)
{
  struct tweak_model_uri_to_tweak_id_index_impl* index_impl = (struct tweak_model_uri_to_tweak_id_index_impl*)index;
  struct uri_tweak_id_pair* pair = NULL;
  HASH_FIND_STR(index_impl->pairs, uri, pair);
  return pair ? pair->tweak_id : TWEAK_INVALID_ID;
}

tweak_model_index_result tweak_model_uri_to_tweak_id_index_remove(tweak_model_uri_to_tweak_id_index index,
  const char *uri)
{
  struct tweak_model_uri_to_tweak_id_index_impl* index_impl = (struct tweak_model_uri_to_tweak_id_index_impl*)index;
  struct uri_tweak_id_pair* pair = NULL;
  HASH_FIND_STR(index_impl->pairs, uri, pair);
  if (!pair) {
    return TWEAK_MODEL_INDEX_KEY_NOT_FOUND;
  }
  HASH_DEL(index_impl->pairs, pair);
  free(pair->key);
  free(pair);

  return TWEAK_MODEL_INDEX_SUCCESS;
}

void tweak_model_uri_to_tweak_id_index_destroy(tweak_model_uri_to_tweak_id_index index) {
  struct tweak_model_uri_to_tweak_id_index_impl* index_impl = (struct tweak_model_uri_to_tweak_id_index_impl*)index;
  struct uri_tweak_id_pair *pair = NULL;
  struct uri_tweak_id_pair *tmp = NULL;
  HASH_ITER(hh, index_impl->pairs, pair, tmp) {
    HASH_DEL(index_impl->pairs, pair);
    free(pair->key);
    free(pair);
  }
  free(index_impl);
}
