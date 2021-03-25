/**
 * @file tweakmodel.c
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

#include "tweakmodel.h"

#include <stdlib.h>
#include <uthash.h>

#define HASH_FIND_TWEAK_ID(head, findint, out) HASH_FIND(hh, head, findint, sizeof(tweak_id), out)

#define HASH_ADD_TWEAK_ID(head, intfield, add) HASH_ADD(hh, head, intfield, sizeof(tweak_id), add)

struct id_item_pair {
  tweak_id id;
  tweak_item* item;
  UT_hash_handle hh;
};

struct tweak_model_impl {
  struct tweak_model_base base;
  struct id_item_pair *pairs;
};


static tweak_model_error_code attach_item(tweak_model model, tweak_id id,
  tweak_item* item)
{
  struct tweak_model_impl* model_impl = (struct tweak_model_impl*)model;
  struct id_item_pair *pair = NULL;
  HASH_FIND_TWEAK_ID(model_impl->pairs, &id, pair);
  if (pair != NULL) {
    return TWEAK_MODEL_INDEX_ERROR;
  }
  pair = malloc(sizeof(*pair));
  if (pair == NULL) {
    return TWEAK_MODEL_BAD_ALLOC;
  }
  pair->id = id;
  pair->item = item;
  HASH_ADD_TWEAK_ID(model_impl->pairs, id, pair);  /* id: name of key field */
  return TWEAK_MODEL_SUCCESS;
}

tweak_model_error_code tweak_model_create_item(tweak_model model, tweak_id id,
  const tweak_variant_string* uri, const tweak_variant_string* description,
  const tweak_variant_string* meta, const tweak_variant* default_value,
  const tweak_variant* current_value, void* item_cookie)
{
  tweak_model_error_code model_error_code;
  tweak_item* item = calloc(1, sizeof(*item));
  if (item != NULL) {
    model_error_code = attach_item(model, id, item);
    if (model_error_code == TWEAK_MODEL_SUCCESS) {
      item->id = id;
      item->uri = tweak_variant_string_copy(uri);
      item->description = tweak_variant_string_copy(description);
      item->meta = tweak_variant_string_copy(meta);
      item->default_value = tweak_variant_copy(default_value);
      item->current_value = tweak_variant_copy(current_value);
      item->variant_type = current_value->type;
      item->item_cookie = item_cookie;
    } else {
      free(item);
    }
  } else {
    model_error_code = TWEAK_MODEL_BAD_ALLOC;
  }
  return model_error_code;
}

static void destroy_item(tweak_item* item) {
  tweak_variant_destroy_string(&item->uri);
  tweak_variant_destroy_string(&item->description);
  tweak_variant_destroy_string(&item->meta);
  tweak_variant_destroy(&item->default_value);
  tweak_variant_destroy(&item->current_value);
  free(item);
}

tweak_model tweak_model_create() {
  struct tweak_model_impl* model_impl = calloc(1, sizeof(*model_impl));
  return model_impl ? &model_impl->base : NULL;
}

tweak_item* tweak_model_find_item_by_id(tweak_model model, tweak_id id) {
  struct tweak_model_impl* model_impl = (struct tweak_model_impl*)model;
  struct id_item_pair *pair = NULL;
  HASH_FIND_TWEAK_ID(model_impl->pairs, &id, pair);
  if (!pair) {
    return NULL;
  }
  return pair->item;
}

static tweak_item* detach_item(tweak_model model, tweak_id id) {
  struct tweak_model_impl* model_impl = (struct tweak_model_impl*)model;
  struct id_item_pair *pair = NULL;
  tweak_item* tweak_item = NULL;
  HASH_FIND_TWEAK_ID(model_impl->pairs, &id, pair);
  if (!pair) {
    return NULL;
  }
  HASH_DEL(model_impl->pairs, pair);
  tweak_item = pair->item;
  free(pair);
  return tweak_item;
}

tweak_model_error_code tweak_model_remove_item(tweak_model model, tweak_id id) {
  tweak_item* item = detach_item(model, id);
  if (item != NULL) {
    destroy_item(item);
    return TWEAK_MODEL_SUCCESS;
  } else {
    return TWEAK_MODEL_ITEM_NOT_FOUND;
  }
}

void tweak_model_destroy(tweak_model model) {
  struct tweak_model_impl* model_impl = (struct tweak_model_impl*)model;
  struct id_item_pair *pair = NULL, *tmp = NULL;
  HASH_ITER(hh, model_impl->pairs, pair, tmp) {
    HASH_DEL(model_impl->pairs, pair);
    destroy_item(pair->item);
    free(pair);
  }
  free(model_impl);
}
