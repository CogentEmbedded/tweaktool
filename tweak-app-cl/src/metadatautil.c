/**
 * @file metadatautil.c
 * @ingroup tweak-api
 *
 * @brief formats and parses multidimensional json arrays with respect to metadata.
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

#include <stddef.h>
#include <utarray.h>

#include <tweak2/json.h>
#include <tweak2/metadata.h>
#include "metadatautil.h"

static const UT_icd ut_size_t_icd = {sizeof(size_t), NULL, NULL, NULL};

struct array_to_string_context;

typedef tweak_variant_string (*buffer_item_to_string)(struct array_to_string_context* context, size_t index);

struct matrix_geometry {
  tweak_metadata_layout layout;
  UT_array *indices;
  size_t cur_index;
};

struct array_to_string_context {
  struct matrix_geometry geometry;
  const void* buffer;
  size_t buffer_size;
  buffer_item_to_string convertor_proc;
};

#define IMPLEMENT_BUFFER_ITEM_TO_STRING(SUFFIX, C_TYPE, FORMAT)                                       \
  static tweak_variant_string SUFFIX##_buffer_item_to_string(struct array_to_string_context* context, \
    size_t index)                                                                                     \
  {                                                                                                   \
    const C_TYPE* array = (const C_TYPE*) context->buffer;                                            \
    tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;                                         \
    tweak_string_format(&result, FORMAT, array[index]);                                               \
    return result;                                                                                    \
  }

IMPLEMENT_BUFFER_ITEM_TO_STRING(sint8, int8_t, "%d")
IMPLEMENT_BUFFER_ITEM_TO_STRING(sint16, int16_t, "%d")
IMPLEMENT_BUFFER_ITEM_TO_STRING(sint32, int32_t, "%d")
IMPLEMENT_BUFFER_ITEM_TO_STRING(sint64, int64_t, "%" PRId64)
IMPLEMENT_BUFFER_ITEM_TO_STRING(uint8, uint8_t, "%u")
IMPLEMENT_BUFFER_ITEM_TO_STRING(uint16, uint16_t, "%u")
IMPLEMENT_BUFFER_ITEM_TO_STRING(uint32, uint32_t, "%u")
IMPLEMENT_BUFFER_ITEM_TO_STRING(uint64, uint64_t, "%" PRIu64)
IMPLEMENT_BUFFER_ITEM_TO_STRING(float, float, "%.17g")
IMPLEMENT_BUFFER_ITEM_TO_STRING(double, double, "%.17g")

static size_t array_geom_get_factor(struct matrix_geometry* geometry, size_t n_dim) {
  size_t result = SIZE_MAX;
  switch(tweak_metadata_layout_get_order(geometry->layout)) {
  case TWEAK_METADATA_LAYOUT_ORDER_ROW_MAJOR:
    result = 1;
    for (size_t n_dim0 = 0; n_dim0 < n_dim; ++n_dim0) {
      size_t factor = tweak_metadata_layout_get_dimension(geometry->layout, n_dim - n_dim0);
      result *= factor;
    }
    break;
  case TWEAK_METADATA_LAYOUT_ORDER_COLUMN_MAJOR:
    result = 1;
    for (size_t n_dim0 = 0; n_dim0 < n_dim; ++n_dim0) {
      size_t factor = tweak_metadata_layout_get_dimension(geometry->layout, n_dim0);
      result *= factor;
    }
    break;
  }
  assert(result != SIZE_MAX);
  return result;
}

static inline size_t get_index(struct matrix_geometry* geometry, size_t ix) {
  return *(size_t*)utarray_eltptr(geometry->indices, ix);
}

static inline void set_index(struct matrix_geometry* geometry, size_t ix, size_t i) {
  *(size_t *)utarray_eltptr(geometry->indices, ix) = i;
}

static size_t array_geom_calculate_array_index(struct matrix_geometry* geometry) {
  size_t index = SIZE_MAX;
  size_t dim_count = tweak_metadata_layout_get_number_of_dimensions(geometry->layout);
  switch(tweak_metadata_layout_get_order(geometry->layout)) {
  case TWEAK_METADATA_LAYOUT_ORDER_ROW_MAJOR:
    index = 0;
    for (size_t n_dim = 0; n_dim < dim_count; ++n_dim) {
      size_t factor = array_geom_get_factor(geometry, dim_count - n_dim - 1);
      index += factor * get_index(geometry, n_dim);
    }
    break;
  case TWEAK_METADATA_LAYOUT_ORDER_COLUMN_MAJOR:
    index = 0;
    for (size_t n_dim = 0; n_dim < dim_count; ++n_dim) {
      size_t factor = array_geom_get_factor(geometry, n_dim);
      index += factor * get_index(geometry, n_dim);
    }
    break;
  }
  assert(index != SIZE_MAX);
  return index;
}

static tweak_variant_string nested_array_to_string(struct array_to_string_context* context) {
  tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;
  size_t dim_count = tweak_metadata_layout_get_dimension(context->geometry.layout, context->geometry.cur_index);
  tweak_string_append(&result, "[");
  for (size_t i = 0; i < dim_count; i++) {
    if (i != 0) {
      tweak_string_append(&result, ", ");
    }
    set_index(&context->geometry, context->geometry.cur_index, i);
    tweak_variant_string item_str;
    if (context->geometry.cur_index == tweak_metadata_layout_get_number_of_dimensions(context->geometry.layout) - 1) {
      size_t index = array_geom_calculate_array_index(&context->geometry);
      item_str = context->convertor_proc(context, index);
    } else {
      ++context->geometry.cur_index;
      item_str = nested_array_to_string(context);
      --context->geometry.cur_index;
    }
    tweak_string_append(&result, tweak_variant_string_c_str(&item_str));
    tweak_variant_destroy_string(&item_str);
  }
  tweak_string_append(&result, "]");
  return result;
}

static tweak_variant_string array_to_string(const tweak_variant* value,
  size_t item_size, const tweak_variant_string* metadata_str, buffer_item_to_string convertor_proc)
{
  size_t num_items = value->value.buffer.size / item_size;
  tweak_metadata metadata = tweak_metadata_create(value->type,
    num_items, tweak_variant_string_c_str(metadata_str));

  if (!metadata)
    goto no_layout;

  tweak_metadata_layout layout = tweak_metadata_get_layout(metadata);

  struct array_to_string_context context = {
    .geometry = {
      .layout = layout,
      .indices = NULL,
      .cur_index = 0,
    },
    .buffer = tweak_buffer_get_data_const(&value->value.buffer),
    .buffer_size = num_items,
    .convertor_proc = convertor_proc
  };

  utarray_new(context.geometry.indices, &ut_size_t_icd);
  utarray_resize(context.geometry.indices, tweak_metadata_layout_get_number_of_dimensions(layout));
  tweak_variant_string result = nested_array_to_string(&context);
  utarray_free(context.geometry.indices);
  tweak_metadata_destroy(metadata);
  return result;

no_layout:
  return tweak_variant_to_string(value);
}

tweak_variant_string tweak_app_cl_metadata_aware_variant_to_string(const tweak_variant* value,
  const tweak_variant_string* metadata_str)
{
  switch (value->type) {
  case TWEAK_VARIANT_TYPE_NULL:
  case TWEAK_VARIANT_TYPE_BOOL:
  case TWEAK_VARIANT_TYPE_SINT8:
  case TWEAK_VARIANT_TYPE_SINT16:
  case TWEAK_VARIANT_TYPE_SINT32:
  case TWEAK_VARIANT_TYPE_SINT64:
  case TWEAK_VARIANT_TYPE_UINT8:
  case TWEAK_VARIANT_TYPE_UINT16:
  case TWEAK_VARIANT_TYPE_UINT32:
  case TWEAK_VARIANT_TYPE_UINT64:
  case TWEAK_VARIANT_TYPE_FLOAT:
  case TWEAK_VARIANT_TYPE_DOUBLE:
  case TWEAK_VARIANT_TYPE_STRING:
    return tweak_variant_to_string(value);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return array_to_string(value, sizeof(int8_t), metadata_str, sint8_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return array_to_string(value, sizeof(int16_t), metadata_str, sint16_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return array_to_string(value, sizeof(int32_t), metadata_str, sint32_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return array_to_string(value, sizeof(int64_t), metadata_str, sint64_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return array_to_string(value, sizeof(uint8_t), metadata_str, uint8_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return array_to_string(value, sizeof(uint16_t), metadata_str, uint16_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return array_to_string(value, sizeof(uint32_t), metadata_str, uint32_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return array_to_string(value, sizeof(uint64_t), metadata_str, uint64_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return array_to_string(value, sizeof(float), metadata_str, float_buffer_item_to_string);
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return array_to_string(value, sizeof(double), metadata_str, double_buffer_item_to_string);
  }
  TWEAK_FATAL("Unknown type: %d", value->type);
  return tweak_variant_to_string(value);
}

struct array_from_string_context;

typedef tweak_variant_type_conversion_result (*buffer_item_from_string_proc)(struct array_from_string_context* context,
  const char* string, size_t index);

struct array_from_string_context {
  struct matrix_geometry geometry;
  const struct tweak_json_node* cur_node;
  void* buffer;
  buffer_item_from_string_proc convertor_proc;
  tweak_variant_type_conversion_result conversion_result;
};

static tweak_variant_type_conversion_result nested_array_from_string(struct array_from_string_context* context);

#define IMPLEMENT_BUFFER_ITEM_FROM_STRING(SUFFIX, C_TYPE, TWEAK_TYPE, FIELD)                                         \
  static tweak_variant_type_conversion_result SUFFIX##_buffer_item_from_string(                                      \
    struct array_from_string_context* context, const char* string, size_t index)                                     \
  {                                                                                                                  \
    tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;                                                                    \
    C_TYPE* array = (C_TYPE*)context->buffer;                                                                        \
    tweak_variant_type_conversion_result conversion_result =                                                         \
      tweak_variant_from_string(string, TWEAK_TYPE, &tmp);                                                           \
    switch (conversion_result) {                                                                                     \
    case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS:                                                               \
    case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED:                                                       \
      array[index] = tmp.value.FIELD;                                                                                \
      break;                                                                                                         \
    case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL:                                                            \
      TWEAK_LOG_WARN("Can't parse string : %s", string);                                                             \
      break;                                                                                                         \
    }                                                                                                                \
    return conversion_result;                                                                                        \
  }                                                                                                                  \
                                                                                                                     \
  static tweak_variant_type_conversion_result SUFFIX##_buffer_from_string(struct array_from_string_context* context, \
    size_t num_items, tweak_variant* out)                                                                            \
  {                                                                                                                  \
    tweak_variant_type_conversion_result conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;        \
    context->buffer = malloc(num_items * sizeof(C_TYPE));                                                            \
    if (context->buffer != NULL) {                                                                                   \
      context->convertor_proc = SUFFIX##_buffer_item_from_string;                                                    \
      conversion_result = nested_array_from_string(context);                                                         \
      if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {                                       \
        tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;                                                                \
        tweak_variant_assign_##SUFFIX##_vector(&tmp, context->buffer, num_items);                                    \
        tweak_variant_swap(&tmp, out);                                                                               \
        tweak_variant_destroy(&tmp);                                                                                 \
      }                                                                                                              \
      free(context->buffer);                                                                                         \
    }                                                                                                                \
    return conversion_result;                                                                                        \
  }

IMPLEMENT_BUFFER_ITEM_FROM_STRING(sint8, int8_t, TWEAK_VARIANT_TYPE_SINT8, sint8)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(sint16, int16_t, TWEAK_VARIANT_TYPE_SINT16, sint16)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(sint32, int32_t, TWEAK_VARIANT_TYPE_SINT32, sint32)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(sint64, int64_t, TWEAK_VARIANT_TYPE_SINT64, sint64)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(uint8, uint8_t, TWEAK_VARIANT_TYPE_UINT8, uint8)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(uint16, uint16_t, TWEAK_VARIANT_TYPE_UINT16, uint16)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(uint32, uint32_t, TWEAK_VARIANT_TYPE_UINT32, uint32)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(uint64, uint64_t, TWEAK_VARIANT_TYPE_UINT64, uint64)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(float, float, TWEAK_VARIANT_TYPE_FLOAT, fp32)
IMPLEMENT_BUFFER_ITEM_FROM_STRING(double, double, TWEAK_VARIANT_TYPE_DOUBLE, fp64)

static tweak_variant_type_conversion_result nested_array_from_string(struct array_from_string_context* context) {
  size_t dim_count = tweak_metadata_layout_get_dimension(context->geometry.layout, context->geometry.cur_index);
  for (size_t i = 0; i < dim_count; i++) {
    set_index(&context->geometry, context->geometry.cur_index, i);
    const struct tweak_json_node* item_node;
    if (context->geometry.cur_index == tweak_metadata_layout_get_number_of_dimensions(context->geometry.layout) - 1) {
      item_node = tweak_json_get_array_item(context->cur_node, i, TWEAK_JSON_NODE_TYPE_NUMBER);
      if (!item_node) {
        return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
      }
      size_t index = array_geom_calculate_array_index(&context->geometry);
      if (context->convertor_proc(context, tweak_json_node_as_c_str(item_node), index)
            != TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS)
      {
        return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
      }
    } else {
      item_node = tweak_json_get_array_item(context->cur_node, i, TWEAK_JSON_NODE_TYPE_ARRAY);
      if (!item_node) {
        return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
      }

      const struct tweak_json_node* cur_node = context->cur_node;
      size_t array_size;
      tweak_json_get_array_size_status status = tweak_json_get_array_size(cur_node, &array_size);
      if (status != TWEAK_JSON_GET_SIZE_SUCCESS) {
        return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
      }

      if (array_size == tweak_metadata_layout_get_dimension(context->geometry.layout, context->geometry.cur_index)) {
        context->cur_node = item_node;
        ++context->geometry.cur_index;
        if (nested_array_from_string(context) != TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
          return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
        }
        --context->geometry.cur_index;
        context->cur_node = cur_node;
      } else {
        return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
      }
    }
  }
  return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
}

tweak_variant_type_conversion_result tweak_app_cl_metadata_aware_variant_from_string(const char* string,
  tweak_variant_type type, size_t item_count, const tweak_variant_string* metadata_str, tweak_variant* out)
{
  tweak_metadata metadata = NULL;
  struct tweak_json_node* root = NULL;

  root = tweak_json_parse(string);
  if (tweak_json_get_type(root) != TWEAK_JSON_NODE_TYPE_ARRAY)
    goto fallback;

  metadata = tweak_metadata_create(type, item_count, tweak_variant_string_c_str(metadata_str));
  if (!metadata)
    goto fallback;

  struct array_from_string_context context = {
    .geometry = {
      .layout = tweak_metadata_get_layout(metadata),
      .indices = NULL,
      .cur_index = 0
    },
    .cur_node = root
  };

  tweak_metadata_layout layout = tweak_metadata_get_layout(metadata);
  if (!layout)
    goto fallback;

  utarray_new(context.geometry.indices, &ut_size_t_icd);
  utarray_resize(context.geometry.indices, tweak_metadata_layout_get_number_of_dimensions(layout));

  tweak_variant_type_conversion_result conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  switch (type) {
  case TWEAK_VARIANT_TYPE_NULL:
  case TWEAK_VARIANT_TYPE_BOOL:
  case TWEAK_VARIANT_TYPE_SINT8:
  case TWEAK_VARIANT_TYPE_SINT16:
  case TWEAK_VARIANT_TYPE_SINT32:
  case TWEAK_VARIANT_TYPE_SINT64:
  case TWEAK_VARIANT_TYPE_UINT8:
  case TWEAK_VARIANT_TYPE_UINT16:
  case TWEAK_VARIANT_TYPE_UINT32:
  case TWEAK_VARIANT_TYPE_UINT64:
  case TWEAK_VARIANT_TYPE_FLOAT:
  case TWEAK_VARIANT_TYPE_DOUBLE:
  case TWEAK_VARIANT_TYPE_STRING:
    conversion_result = tweak_variant_from_string(string, type, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    conversion_result = sint8_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    conversion_result = sint16_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    conversion_result = sint32_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    conversion_result = sint64_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    conversion_result = uint8_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    conversion_result = uint16_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    conversion_result = uint32_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    conversion_result = uint64_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    conversion_result = float_buffer_from_string(&context, item_count, out);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    conversion_result = double_buffer_from_string(&context, item_count, out);
    break;
  }

  tweak_json_destroy(root);
  utarray_free(context.geometry.indices);
  tweak_metadata_destroy(metadata);
  return conversion_result;

fallback:
  if (root) {
    tweak_json_destroy(root);
  }

  if (metadata) {
    tweak_metadata_destroy(metadata);
  }

  return tweak_variant_from_string(string, type, out);
}
