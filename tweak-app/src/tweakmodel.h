/**
 * @file tweakmodel.h
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

#ifndef TWEAK_MODEL_H_INCLUDED
#define TWEAK_MODEL_H_INCLUDED

#include <tweak2/string.h>
#include <tweak2/types.h>
#include <tweak2/variant.h>
#include <tweak2/metadata.h>

/**
 * @brief Structure to encapsulate an item.
 */
typedef struct {
  /**
   * @brief id.
   */
  tweak_id id;
  /**
   * @brief uri.
   */
  tweak_variant_string uri;
  /**
   * @brief description.
   */
  tweak_variant_string description;
  /**
   * @brief meta.
   */
  tweak_variant_string meta;
  /**
   * @brief type.
   */
  tweak_variant_type variant_type;
  /**
   * @brief current value.
   */
  tweak_variant current_value;
  /**
   * @brief default value.
   */
  tweak_variant default_value;
  /**
   * @brief user might provide additional context on per-item basis.
   */
  void* item_cookie;
  /**
   * @brief Metadata was parsed. If this field is true and metadata
   * field is NULL then string contained in meta is invalid.
   */
  bool metadata_initialized;
  /**
   * @brief metadata instance.
   */
  tweak_metadata metadata;
} tweak_item;

/**
 * @brief Error code for model ops.
 */
typedef enum {
 /**
  * @brief Success.
  */
  TWEAK_MODEL_SUCCESS,
  /**
   * @brief Constraint violation.
   */
  TWEAK_MODEL_INDEX_ERROR,
  /**
   * @brief Memory allocation routine returned NULL.
   */
  TWEAK_MODEL_BAD_ALLOC,
  /**
   * @brief Item not found.
   */
  TWEAK_MODEL_ITEM_NOT_FOUND
} tweak_model_error_code;

/**
 * @brief Base class for model.
 */
struct tweak_model_base {
  int dummy; /* Empty structures are implementation specific. */
             /* ANSI ISO C99 doesn't forbid nor explicitly allow them. */
             /* GreenHills C compiler, MSVC and GCC in pedantic mode */
             /* would issue a warning or an error */
             /* if this dummy placeholder is removed. */
};

/**
 * @brief typedef for base class.
 */
typedef struct tweak_model_base *tweak_model;

/**
 * @brief create model instance.
 *
 * @return new instance or NULL if there was memory allocation error
 */
tweak_model tweak_model_create();

/**
 * @brief Create new item to store in model.
 *
 * @note this method uses move semantic and because of that
 * the item being created becomes the owner of all provided
 * tweak_variant_string and tweak_variant instances.
 *
 * @note This code is thread neutral, and user should provide synchronization when accessing model.
 *
 * @param model model instance to append new item onto.
 * @param id id
 * @param uri uri
 * @param description description
 * @param meta meta
 * @param default_value default_value
 * @param current_value current_value
 * @param metadata metadata
 * @param item_cookie additional context
 *
 * @return TWEAK_MODEL_SUCCESS if there weren't any errors, TWEAK_MODEL_INDEX_ERROR if
 * there was constraint violation, TWEAK_MODEL_BAD_ALLOC if there was memory allocation
 * error.
 */
tweak_model_error_code tweak_model_create_item(tweak_model model, tweak_id id,
  const tweak_variant_string* uri, const tweak_variant_string* description,
  const tweak_variant_string* meta, const tweak_variant* default_value,
  const tweak_variant* current_value, void* item_cookie);

/**
 * @brief Find an item in model given its id.
 *
 * @note This code is thread neutral, and user should provide synchronization when altering item's fields.
 *
 * @param model model instance.
 * @param id item's id.
 *
 * @return item instance or NULL if there's no item with provided id.
 */
tweak_item* tweak_model_find_item_by_id(tweak_model model, tweak_id id);

/**
 * @brief Remove an item from model given its id.
 *
 * @note This code is thread neutral, and user should provide synchronization when accessing model.
 *
 * @param model model instance.
 * @param id item's id.
 *
 * @return TWEAK_MODEL_SUCCESS if there weren't any errors.
 */
tweak_model_error_code tweak_model_remove_item(tweak_model model, tweak_id id);

/**
 * @brief Destroy model instance and deallocate all the resources.
 *
 * @param model model instance.
 */
void tweak_model_destroy(tweak_model model);

#endif /* TWEAK_MODEL_H_INCLUDED */
