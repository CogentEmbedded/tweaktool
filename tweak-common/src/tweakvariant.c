/**
 * @file tweakvariant.c
 * @ingroup tweak-api
 *
 * @brief Tweak variant type, data conversion helpers.
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
 * @defgroup tweak-app-implementation Implementation of tweak-app component.
 */

#include <tweak2/log.h>
#include <tweak2/variant.h>
#include <tweak2/json.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <ctype.h>

static tweak_variant_string null_to_string() {
  tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;
  tweak_assign_string(&result, "null");
  return result;
}

static tweak_variant_string bool_to_string(bool arg) {
  tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;
  tweak_assign_string(&result, arg ? "true" : "false");
  return result;
}

#define IMPLEMENT_SCALAR_TO_STRING(SUFFIX, C_TYPE, FORMAT)                                                   \
  static tweak_variant_string SUFFIX##_to_string(C_TYPE arg) {                                               \
    tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;                                                \
    tweak_string_format(&result, FORMAT, arg);                                                               \
    return result;                                                                                           \
  }

IMPLEMENT_SCALAR_TO_STRING(sint8, int8_t, "%d")
IMPLEMENT_SCALAR_TO_STRING(sint16, int16_t, "%d")
IMPLEMENT_SCALAR_TO_STRING(sint32, int32_t, "%d")
IMPLEMENT_SCALAR_TO_STRING(sint64, int64_t, "%" PRId64)
IMPLEMENT_SCALAR_TO_STRING(uint8, uint8_t, "%u")
IMPLEMENT_SCALAR_TO_STRING(uint16, uint16_t, "%u")
IMPLEMENT_SCALAR_TO_STRING(uint32, uint32_t, "%u")
IMPLEMENT_SCALAR_TO_STRING(uint64, uint64_t, "%" PRIu64)
IMPLEMENT_SCALAR_TO_STRING(float, float, "%.17g")
IMPLEMENT_SCALAR_TO_STRING(double, double, "%.17g")

#define IMPLEMENT_VECTOR_TO_STRING(SUFFIX, CTYPE)                                                            \
  static tweak_variant_string SUFFIX##_vector_to_string(const CTYPE* array, size_t count)                    \
  {                                                                                                          \
    tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;                                                \
    tweak_string_append(&result, "[");                                                                       \
    for (size_t ix = 0; ix < count; ++ix) {                                                                  \
      tweak_variant_string s = SUFFIX##_to_string(array[ix]);                                                \
      if (ix != 0) {                                                                                         \
        tweak_string_append(&result, ", ");                                                                  \
      }                                                                                                      \
      tweak_string_append(&result, tweak_variant_string_c_str(&s));                                          \
      tweak_variant_destroy_string(&s);                                                                      \
    }                                                                                                        \
    tweak_string_append(&result, "]");                                                                       \
    return result;                                                                                           \
  }

IMPLEMENT_VECTOR_TO_STRING(sint8, int8_t)
IMPLEMENT_VECTOR_TO_STRING(sint16, int16_t)
IMPLEMENT_VECTOR_TO_STRING(sint32, int32_t)
IMPLEMENT_VECTOR_TO_STRING(sint64, int64_t)
IMPLEMENT_VECTOR_TO_STRING(uint8, uint8_t)
IMPLEMENT_VECTOR_TO_STRING(uint16, uint16_t)
IMPLEMENT_VECTOR_TO_STRING(uint32, uint32_t)
IMPLEMENT_VECTOR_TO_STRING(uint64, uint64_t)
IMPLEMENT_VECTOR_TO_STRING(float, float)
IMPLEMENT_VECTOR_TO_STRING(double, double)

tweak_variant_string tweak_variant_to_string(const tweak_variant* arg) {
  assert(arg != NULL && tweak_variant_is_valid(arg));
  switch (arg->type) {
  case TWEAK_VARIANT_TYPE_NULL:
    return null_to_string();
  case TWEAK_VARIANT_TYPE_BOOL:
    return bool_to_string(arg->value.b);
  case TWEAK_VARIANT_TYPE_SINT8:
    return sint8_to_string(arg->value.sint8);
  case TWEAK_VARIANT_TYPE_SINT16:
    return sint16_to_string(arg->value.sint16);
  case TWEAK_VARIANT_TYPE_SINT32:
    return sint32_to_string(arg->value.sint32);
  case TWEAK_VARIANT_TYPE_SINT64:
    return sint64_to_string(arg->value.sint64);
  case TWEAK_VARIANT_TYPE_UINT8:
    return uint8_to_string(arg->value.uint8);
  case TWEAK_VARIANT_TYPE_UINT16:
    return uint16_to_string(arg->value.uint16);
  case TWEAK_VARIANT_TYPE_UINT32:
    return uint32_to_string(arg->value.uint32);
  case TWEAK_VARIANT_TYPE_UINT64:
    return uint64_to_string(arg->value.uint64);
  case TWEAK_VARIANT_TYPE_FLOAT:
    return float_to_string(arg->value.fp32);
  case TWEAK_VARIANT_TYPE_DOUBLE:
    return double_to_string(arg->value.fp64);
  case TWEAK_VARIANT_TYPE_STRING:
    return tweak_variant_string_copy(&arg->value.string);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return sint8_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(int8_t));
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return sint16_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(int16_t));
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return sint32_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(int32_t));
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return sint64_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(int64_t));
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return uint8_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(uint8_t));
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return uint16_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(uint16_t));
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return uint32_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(uint32_t));
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return uint64_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(uint64_t));
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return float_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(float));
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return double_vector_to_string(
      tweak_buffer_get_data_const(&arg->value.buffer),
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(double));
  }
  return null_to_string();
}

static tweak_variant_string json_escape_string(const tweak_variant_string *arg)
{
  tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;
  const char *p = tweak_variant_string_c_str(arg);
  while (*p) {
    switch (*p) {
    // quotation mark (0x22)
    case '"':
      tweak_string_append(&result, "\\\"");
      break;

    case '\\':
      tweak_string_append(&result, "\\\\");
      break;

    // backspace (0x08)
    case '\b':
      tweak_string_append(&result, "\\b");
      break;

    // formfeed (0x0c)
    case '\f':
      tweak_string_append(&result, "\\f");
      break;

    // newline (0x0a)
    case '\n':
      tweak_string_append(&result, "\\n");
      break;

    // carriage return (0x0d)
    case '\r':
      tweak_string_append(&result, "\\r");
      break;

    // horizontal tab (0x09)
    case '\t':
      tweak_string_append(&result, "\\t");
      break;

    default:
      if (iscntrl(*p)) {
        // print character c as \uxxxx
        tweak_variant_string tmp = TWEAK_VARIANT_STRING_EMPTY;
        tweak_string_format(&tmp, "\\u%04x", *p);
        tweak_string_append(&result, tweak_variant_string_c_str(&tmp));
        tweak_variant_destroy_string(&tmp);
      } else {
        // all other characters are added as-is
        const char tmp[] = {*p, '\0'};
        tweak_string_append(&result, tmp);
      }
      break;
    }
    ++p;
  }
  return result;
}

#define IMPLEMENT_SCALAR_ITEM_TO_JSON(C_TYPE, SUFFIX, FIELD)                                         \
  static tweak_variant_string SUFFIX##_to_json(const tweak_variant* arg) {                           \
    tweak_variant_string tmp = SUFFIX##_to_string(arg->value.FIELD);                                 \
    tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;                                        \
    tweak_string_format(&result, "{\"" #SUFFIX "\": %s}", tweak_variant_string_c_str(&tmp));         \
    tweak_variant_destroy_string(&tmp);                                                              \
    return result;                                                                                   \
  }

IMPLEMENT_SCALAR_ITEM_TO_JSON(bool, bool, b)
IMPLEMENT_SCALAR_ITEM_TO_JSON(int8_t, sint8, sint8)
IMPLEMENT_SCALAR_ITEM_TO_JSON(int16_t, sint16, sint16)
IMPLEMENT_SCALAR_ITEM_TO_JSON(int32_t, sint32, sint32)
IMPLEMENT_SCALAR_ITEM_TO_JSON(int64_t, sint64, sint64)
IMPLEMENT_SCALAR_ITEM_TO_JSON(uint8_t, uint8, uint8)
IMPLEMENT_SCALAR_ITEM_TO_JSON(uint16_t, uint16, uint16)
IMPLEMENT_SCALAR_ITEM_TO_JSON(uint32_t, uint32, uint32)
IMPLEMENT_SCALAR_ITEM_TO_JSON(uint64_t, uint64, uint64)
IMPLEMENT_SCALAR_ITEM_TO_JSON(float, float, fp32)
IMPLEMENT_SCALAR_ITEM_TO_JSON(double, double, fp64)

#define IMPLEMENT_VECTOR_ITEM_TO_JSON(C_TYPE, SUFFIX)                                                \
  static tweak_variant_string SUFFIX##_vector_to_json(const tweak_variant* arg) {                    \
    tweak_variant_string tmp = SUFFIX##_vector_to_string(                                            \
      tweak_buffer_get_data_const(&arg->value.buffer),                                               \
      tweak_buffer_get_size(&arg->value.buffer) / sizeof(C_TYPE));                                   \
    tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;                                        \
    tweak_string_format(&result,                                                                     \
      "{\"vector\": {\"item_type\": \"%s\", \"items\": %s}}", #SUFFIX,                               \
      tweak_variant_string_c_str(&tmp));                                                             \
    tweak_variant_destroy_string(&tmp);                                                              \
    return result;                                                                                   \
  }

IMPLEMENT_VECTOR_ITEM_TO_JSON(int8_t, sint8)
IMPLEMENT_VECTOR_ITEM_TO_JSON(int16_t, sint16)
IMPLEMENT_VECTOR_ITEM_TO_JSON(int32_t, sint32)
IMPLEMENT_VECTOR_ITEM_TO_JSON(int64_t, sint64)
IMPLEMENT_VECTOR_ITEM_TO_JSON(uint8_t, uint8)
IMPLEMENT_VECTOR_ITEM_TO_JSON(uint16_t, uint16)
IMPLEMENT_VECTOR_ITEM_TO_JSON(uint32_t, uint32)
IMPLEMENT_VECTOR_ITEM_TO_JSON(uint64_t, uint64)
IMPLEMENT_VECTOR_ITEM_TO_JSON(float, float)
IMPLEMENT_VECTOR_ITEM_TO_JSON(double, double)

static tweak_variant_string string_to_json(const tweak_variant* arg) {
  tweak_variant_string tmp = json_escape_string(&arg->value.string);
  tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;
  tweak_string_format(&result, "{\"string\": \"%s\"}",
    tweak_variant_string_c_str(&tmp));
  tweak_variant_destroy_string(&tmp);
  return result;
}

tweak_variant_string tweak_variant_to_json(const tweak_variant* arg) {
  assert(arg != NULL && tweak_variant_is_valid(arg));
  switch (arg->type) {
  case TWEAK_VARIANT_TYPE_NULL:
    return null_to_string();
  case TWEAK_VARIANT_TYPE_BOOL:
    return bool_to_json(arg);
  case TWEAK_VARIANT_TYPE_SINT8:
    return sint8_to_json(arg);
  case TWEAK_VARIANT_TYPE_SINT16:
    return sint16_to_json(arg);
  case TWEAK_VARIANT_TYPE_SINT32:
    return sint32_to_json(arg);
  case TWEAK_VARIANT_TYPE_SINT64:
    return sint64_to_json(arg);
  case TWEAK_VARIANT_TYPE_UINT8:
    return uint8_to_json(arg);
  case TWEAK_VARIANT_TYPE_UINT16:
    return uint16_to_json(arg);
  case TWEAK_VARIANT_TYPE_UINT32:
    return uint32_to_json(arg);
  case TWEAK_VARIANT_TYPE_UINT64:
    return uint64_to_json(arg);
  case TWEAK_VARIANT_TYPE_FLOAT:
    return float_to_json(arg);
  case TWEAK_VARIANT_TYPE_DOUBLE:
    return double_to_json(arg);
  case TWEAK_VARIANT_TYPE_STRING:
    return string_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return sint8_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return sint16_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return sint32_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return sint64_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return uint8_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return uint16_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return uint32_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return uint64_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return float_vector_to_json(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return double_vector_to_json(arg);
  }
  TWEAK_FATAL("Unknown type: %d", arg->type);
  return null_to_string();
}

static tweak_variant_type_conversion_result
  parse_bool(const char* arg, tweak_variant* out);

static tweak_variant_type_conversion_result
  parse_signed_int64(const char *s, int64_t* out);

static tweak_variant_type_conversion_result
  parse_generic_int( const char* arg,
                     tweak_variant_type target_type,
                     tweak_variant* out);

static tweak_variant_type_conversion_result
  parse_unsigned_int64(const char *s, uint64_t* out);

static tweak_variant_type_conversion_result
  parse_generic_uint(const char* arg,
                     tweak_variant_type target_type,
                     tweak_variant* out);

static tweak_variant_type_conversion_result
  parse_floating_point(const char *s, double* out);

static tweak_variant_type_conversion_result
  parse_generic_float(const char* arg,
                      tweak_variant_type target_type,
                      tweak_variant* out);

static tweak_variant_type_conversion_result
  parse_string(const char *s, tweak_variant* out);

#define IMPLEMENT_PARSE_VECTOR(SUFFIX, CTYPE, PARSE_SCALAR_FUNC, SCALAR_TWEAK_TYPE, SCALAR_FIELD, VECTOR_TWEAK_TYPE)    \
  static tweak_variant_type_conversion_result parse_vector_##SUFFIX(const char* arg, tweak_variant* out)                \
  {                                                                                                                     \
    struct tweak_json_node* root = NULL;                                                                                \
    tweak_variant_type_conversion_result conversion_result                                                              \
      = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;                                                                \
    tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;                                                                       \
    bool truncated = false;                                                                                             \
                                                                                                                        \
    size_t item_count;                                                                                                  \
    CTYPE* arr;                                                                                                         \
                                                                                                                        \
    root = tweak_json_parse(arg);                                                                                       \
                                                                                                                        \
    if (tweak_json_get_type(root) != TWEAK_JSON_NODE_TYPE_ARRAY) {                                                      \
      goto parse_error;                                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    if (tweak_json_get_array_size(root, &item_count) != TWEAK_JSON_GET_SIZE_SUCCESS) {                                  \
      goto parse_error;                                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    tmp.type = VECTOR_TWEAK_TYPE,                                                                                       \
    tmp.value.buffer = tweak_buffer_create(NULL, item_count * sizeof(CTYPE));                                           \
    arr = (CTYPE*)tweak_buffer_get_data(&tmp.value.buffer);                                                             \
                                                                                                                        \
    for (size_t ix = 0; ix < item_count; ix++) {                                                                        \
      const struct tweak_json_node* item_node =                                                                         \
        tweak_json_get_array_item(root, ix, TWEAK_JSON_NODE_TYPE_NUMBER);                                               \
                                                                                                                        \
      if (!item_node) {                                                                                                 \
        goto parse_error;                                                                                               \
      }                                                                                                                 \
                                                                                                                        \
      tweak_variant item_value = TWEAK_VARIANT_INIT_EMPTY;                                                              \
                                                                                                                        \
      tweak_variant_type_conversion_result item_cr =                                                                    \
        PARSE_SCALAR_FUNC(tweak_json_node_as_c_str(item_node), SCALAR_TWEAK_TYPE, &item_value);                         \
                                                                                                                        \
      switch (item_cr) {                                                                                                \
      case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED:                                                        \
        truncated = true;                                                                                               \
        break;                                                                                                          \
      case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS:                                                                \
        break;                                                                                                          \
      default:                                                                                                          \
        goto parse_error;                                                                                               \
      }                                                                                                                 \
                                                                                                                        \
      arr[ix] = item_value.value.SCALAR_FIELD;                                                                          \
    }                                                                                                                   \
                                                                                                                        \
    tweak_variant_swap(&tmp, out);                                                                                      \
    conversion_result = truncated                                                                                       \
      ? TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED                                                            \
      : TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;                                                                   \
                                                                                                                        \
  parse_error:                                                                                                          \
    tweak_variant_destroy(&tmp);                                                                                        \
    tweak_json_destroy(root);                                                                                           \
    return conversion_result;                                                                                           \
  }

IMPLEMENT_PARSE_VECTOR(sint8, int8_t, parse_generic_int, TWEAK_VARIANT_TYPE_SINT8, sint8, TWEAK_VARIANT_TYPE_VECTOR_SINT8)
IMPLEMENT_PARSE_VECTOR(sint16, int16_t, parse_generic_int, TWEAK_VARIANT_TYPE_SINT16, sint16, TWEAK_VARIANT_TYPE_VECTOR_SINT16)
IMPLEMENT_PARSE_VECTOR(sint32, int32_t, parse_generic_int, TWEAK_VARIANT_TYPE_SINT32, sint32, TWEAK_VARIANT_TYPE_VECTOR_SINT32)
IMPLEMENT_PARSE_VECTOR(sint64, int64_t, parse_generic_int, TWEAK_VARIANT_TYPE_SINT64, sint64, TWEAK_VARIANT_TYPE_VECTOR_SINT64)
IMPLEMENT_PARSE_VECTOR(uint8, uint8_t, parse_generic_uint, TWEAK_VARIANT_TYPE_UINT8, uint8, TWEAK_VARIANT_TYPE_VECTOR_UINT8)
IMPLEMENT_PARSE_VECTOR(uint16, uint16_t, parse_generic_uint, TWEAK_VARIANT_TYPE_UINT16, uint16, TWEAK_VARIANT_TYPE_VECTOR_UINT16)
IMPLEMENT_PARSE_VECTOR(uint32, uint32_t, parse_generic_uint, TWEAK_VARIANT_TYPE_UINT32, uint32, TWEAK_VARIANT_TYPE_VECTOR_UINT32)
IMPLEMENT_PARSE_VECTOR(uint64, uint64_t, parse_generic_uint, TWEAK_VARIANT_TYPE_UINT64, uint64, TWEAK_VARIANT_TYPE_VECTOR_UINT64)
IMPLEMENT_PARSE_VECTOR(float, float, parse_generic_float, TWEAK_VARIANT_TYPE_FLOAT, fp32, TWEAK_VARIANT_TYPE_VECTOR_FLOAT)
IMPLEMENT_PARSE_VECTOR(double, double, parse_generic_float, TWEAK_VARIANT_TYPE_DOUBLE, fp64, TWEAK_VARIANT_TYPE_VECTOR_DOUBLE)

tweak_variant_type_conversion_result tweak_variant_from_string(const char* arg,
  tweak_variant_type target_type, tweak_variant* out)
{
  assert(out != NULL && out->type == TWEAK_VARIANT_TYPE_NULL);
  switch(target_type) {
  case TWEAK_VARIANT_TYPE_NULL:
    TWEAK_FATAL("Can't parse into null type");
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  case TWEAK_VARIANT_TYPE_BOOL:
    return parse_bool(arg, out);
  case TWEAK_VARIANT_TYPE_SINT8:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT8, out);
  case TWEAK_VARIANT_TYPE_SINT16:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT16, out);
  case TWEAK_VARIANT_TYPE_SINT32:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT32, out);
  case TWEAK_VARIANT_TYPE_SINT64:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT64, out);
  case TWEAK_VARIANT_TYPE_UINT8:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT8, out);
  case TWEAK_VARIANT_TYPE_UINT16:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT16, out);
  case TWEAK_VARIANT_TYPE_UINT32:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT32, out);
  case TWEAK_VARIANT_TYPE_UINT64:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT64, out);
  case TWEAK_VARIANT_TYPE_FLOAT:
    return parse_generic_float(arg, TWEAK_VARIANT_TYPE_FLOAT, out);
  case TWEAK_VARIANT_TYPE_DOUBLE:
    return parse_generic_float(arg, TWEAK_VARIANT_TYPE_DOUBLE, out);
  case TWEAK_VARIANT_TYPE_STRING:
    return parse_string(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return parse_vector_sint8(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return parse_vector_sint16(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return parse_vector_sint32(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return parse_vector_sint64(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return parse_vector_uint8(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return parse_vector_uint16(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return parse_vector_uint32(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return parse_vector_uint64(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return parse_vector_float(arg, out);
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return parse_vector_double(arg, out);
  }
  return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
}

static tweak_variant_type_conversion_result
  parse_string(const char *arg, tweak_variant *out)
{
  struct tweak_json_node *root = tweak_json_parse(arg);
  if (tweak_json_get_type(root) == TWEAK_JSON_NODE_TYPE_STRING) {
    tweak_variant_assign_string(out, tweak_json_node_as_c_str(root));
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    tweak_variant_assign_string(out, arg);
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;
  }
}

static tweak_variant_type_conversion_result
  parse_bool(const char* arg, tweak_variant* out)
{
  if (strcmp(arg, "true") == 0 || strcmp(arg, "1") == 0) {
    out->type = TWEAK_VARIANT_TYPE_BOOL;
    out->value.b = true;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else if (strcmp(arg, "false") == 0 || strcmp(arg, "0") == 0) {
    out->type = TWEAK_VARIANT_TYPE_BOOL;
    out->value.b = false;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

static tweak_variant_type_conversion_result
  parse_signed_int64(const char *s, int64_t* out)
{
  int64_t i;
  char* endp;
  i = strtol(s, &endp, 10);
  if (endp != s && *endp == '\0') {
    *out = i;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

static inline bool can_handle_conversion_result(
  tweak_variant_type_conversion_result conversion_result)
{
  return conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS
      || conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;
}

#define COERCE_INT(VARIANT_FIELD, VARIANT_TYPE, VARIANT_TYPE_MIN, VARIANT_TYPE_MAX) \
  if (val >= VARIANT_TYPE_MIN && val <= VARIANT_TYPE_MAX) {                         \
    result.value.VARIANT_FIELD = (VARIANT_TYPE) val;                                \
  } else if (val < VARIANT_TYPE_MIN) {                                              \
    result.value.VARIANT_FIELD = VARIANT_TYPE_MIN;                                  \
    conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;       \
  } else if (val > VARIANT_TYPE_MAX) {                                              \
    result.value.VARIANT_FIELD = VARIANT_TYPE_MAX;                                  \
    conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;       \
  }

static tweak_variant_type_conversion_result
  parse_generic_int(const char* arg,
                    tweak_variant_type target_type,
                    tweak_variant* out)
{
  int64_t val = 0;
  tweak_variant_type_conversion_result conversion_result;
  tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
  conversion_result = parse_signed_int64(arg, &val);
  if (conversion_result != TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
    tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;
    conversion_result = parse_generic_float(arg, TWEAK_VARIANT_TYPE_DOUBLE, &tmp);
    if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
      conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;
      if (tmp.value.fp64 > (double)INT64_MAX) {
        val = INT64_MAX;
      } else if (tmp.value.fp64 < (double)INT64_MIN) {
        val = INT64_MIN;
      } else {
        val = (int64_t)round(tmp.value.fp64);
      }
    }
    tweak_variant_destroy(&tmp);
  }
  if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
    switch (target_type) {
    case TWEAK_VARIANT_TYPE_NULL:
    case TWEAK_VARIANT_TYPE_BOOL:
    case TWEAK_VARIANT_TYPE_UINT8:
    case TWEAK_VARIANT_TYPE_UINT16:
    case TWEAK_VARIANT_TYPE_UINT32:
    case TWEAK_VARIANT_TYPE_UINT64:
    case TWEAK_VARIANT_TYPE_FLOAT:
    case TWEAK_VARIANT_TYPE_DOUBLE:
      TWEAK_FATAL("Not a signed integer type: %d", target_type);
      break;
    case TWEAK_VARIANT_TYPE_SINT8:
      result.type = TWEAK_VARIANT_TYPE_SINT8;
      COERCE_INT(sint8, int8_t, INT8_MIN, INT8_MAX);
      break;
    case TWEAK_VARIANT_TYPE_SINT16:
      result.type = TWEAK_VARIANT_TYPE_SINT16;
      COERCE_INT(sint16, int16_t, INT16_MIN, INT16_MAX);
      break;
    case TWEAK_VARIANT_TYPE_SINT32:
      result.type = TWEAK_VARIANT_TYPE_SINT32;
      COERCE_INT(sint32, int32_t, INT32_MIN, INT32_MAX);
      break;
    case TWEAK_VARIANT_TYPE_SINT64:
      result.type = TWEAK_VARIANT_TYPE_SINT64;
      result.value.sint64 = val;
      break;
    default:
      TWEAK_FATAL("Unknown signed integer type: %d", target_type);
      break;
    }
  }
  if (can_handle_conversion_result(conversion_result)) {
    *out = result;
  }
  return conversion_result;
}

static tweak_variant_type_conversion_result
  parse_unsigned_int64(const char *s, uint64_t* out)
{
  uint64_t u;
  char* endp;
  u = strtoul(s, &endp, 10);
  if (endp != s && *endp == '\0') {
    *out = u;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

#define COERCE_UINT(VARIANT_FIELD, VARIANT_TYPE, VARIANT_TYPE_MAX)                  \
  if (val <= VARIANT_TYPE_MAX)  {                                                   \
    result.value.VARIANT_FIELD = (VARIANT_TYPE) val;                                \
  } else  {                                                                         \
    result.value.VARIANT_FIELD = VARIANT_TYPE_MAX;                                  \
    conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;       \
  }

static tweak_variant_type_conversion_result
  parse_generic_uint(const char* arg,
                     tweak_variant_type target_type,
                     tweak_variant* out)
{
  uint64_t val = 0;
  tweak_variant_type_conversion_result conversion_result;
  tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
  conversion_result = parse_unsigned_int64(arg, &val);
  if (conversion_result != TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
    tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;
    conversion_result = parse_generic_float(arg, TWEAK_VARIANT_TYPE_DOUBLE, &tmp);
    if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
      conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;
      if (tmp.value.fp64 > (double)UINT64_MAX) {
        val = UINT64_MAX;
      } else if (tmp.value.fp64 < 0.) {
        val = 0;
      } else {
        val = (uint64_t)round(tmp.value.fp64);
      }
    }
    tweak_variant_destroy(&tmp);
  }
  if (conversion_result != TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL) {
    switch (target_type) {
    case TWEAK_VARIANT_TYPE_NULL:
    case TWEAK_VARIANT_TYPE_BOOL:
    case TWEAK_VARIANT_TYPE_SINT8:
    case TWEAK_VARIANT_TYPE_SINT16:
    case TWEAK_VARIANT_TYPE_SINT32:
    case TWEAK_VARIANT_TYPE_SINT64:
    case TWEAK_VARIANT_TYPE_FLOAT:
    case TWEAK_VARIANT_TYPE_DOUBLE:
      TWEAK_FATAL("Not an unsigned integer type: %d", target_type);
      break;
    case TWEAK_VARIANT_TYPE_UINT8:
      result.type = TWEAK_VARIANT_TYPE_UINT8;
      COERCE_UINT(uint8, uint8_t, UINT8_MAX);
      break;
    case TWEAK_VARIANT_TYPE_UINT16:
      result.type = TWEAK_VARIANT_TYPE_UINT16;
      COERCE_UINT(uint16, uint16_t, UINT16_MAX);
      break;
    case TWEAK_VARIANT_TYPE_UINT32:
      result.type = TWEAK_VARIANT_TYPE_UINT32;
      COERCE_UINT(uint32, uint32_t, UINT32_MAX);
      break;
    case TWEAK_VARIANT_TYPE_UINT64:
      result.type = TWEAK_VARIANT_TYPE_UINT64;
      result.value.uint64 = val;
      break;
    default:
      TWEAK_FATAL("Unknown signed integer type: %d", target_type);
      break;
    }
  }
  if (can_handle_conversion_result(conversion_result)) {
    *out = result;
  }
  return conversion_result;
}

static tweak_variant_type_conversion_result
  parse_floating_point(const char *s, double* out)
{
  double d;
  char* endp;
  d = strtod(s, &endp);
  if (endp != s && *endp == '\0') {
    *out = d;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

static tweak_variant_type_conversion_result
  parse_generic_float(const char* arg,
                      tweak_variant_type target_type,
                      tweak_variant* out)
{
  double val;
  tweak_variant_type_conversion_result conversion_result;
  tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
  conversion_result = parse_floating_point(arg, &val);
  if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
    switch (target_type) {
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
        TWEAK_FATAL("Not a floating point type: %d", target_type);
      break;
    case TWEAK_VARIANT_TYPE_FLOAT:
      result.type = TWEAK_VARIANT_TYPE_FLOAT;
      result.value.fp32 = (float) val;
      break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
      result.type = TWEAK_VARIANT_TYPE_DOUBLE;
      result.value.fp64 = val;
      break;
    default:
      TWEAK_FATAL("Unknown floating point type: %d", target_type);
      break;
    }
  }
  if (can_handle_conversion_result(conversion_result)) {
    *out = result;
  }
  return conversion_result;
}

tweak_variant tweak_variant_copy(const tweak_variant* variant) {
  assert(variant != NULL && tweak_variant_is_valid(variant));
  switch (variant->type) {
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
    return *variant;
  case TWEAK_VARIANT_TYPE_STRING:
  {
    tweak_variant copy = {
        .type = TWEAK_VARIANT_TYPE_STRING,
        .value = {
          .string = tweak_variant_string_copy(&variant->value.string)
        }
    };
    return copy;
  }

  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
  {
    tweak_variant copy = {
        .type = variant->type,
        .value = {
          .buffer = tweak_buffer_clone(&variant->value.buffer)
        }
    };
    return copy;
  }
  }
  tweak_variant empty = TWEAK_VARIANT_INIT_EMPTY;
  return empty;
}

bool tweak_variant_is_equal(const tweak_variant* arg1, const tweak_variant* arg2) {
  if (!arg1 || !arg2)
    return false;

  if (arg1 == arg2)
    return true;

  assert(tweak_variant_is_valid(arg1));
  assert(tweak_variant_is_valid(arg2));

  if (arg1->type != arg2->type)
    return false;

  switch (arg1->type) {
  case TWEAK_VARIANT_TYPE_NULL:
    return false; /* null != null */
  case TWEAK_VARIANT_TYPE_BOOL:
    return arg1->value.b == arg2->value.b;
  case TWEAK_VARIANT_TYPE_SINT8:
    return arg1->value.sint8 == arg2->value.sint8;
  case TWEAK_VARIANT_TYPE_SINT16:
    return arg1->value.sint16 == arg2->value.sint16;
  case TWEAK_VARIANT_TYPE_SINT32:
    return arg1->value.sint32 == arg2->value.sint32;
  case TWEAK_VARIANT_TYPE_SINT64:
    return arg1->value.sint64 == arg2->value.sint64;
  case TWEAK_VARIANT_TYPE_UINT8:
    return arg1->value.uint8 == arg2->value.uint8;
  case TWEAK_VARIANT_TYPE_UINT16:
    return arg1->value.uint16 == arg2->value.uint16;
  case TWEAK_VARIANT_TYPE_UINT32:
    return arg1->value.uint32 == arg2->value.uint32;
  case TWEAK_VARIANT_TYPE_UINT64:
    return arg1->value.uint64 == arg2->value.uint64;
  case TWEAK_VARIANT_TYPE_FLOAT:
    return arg1->value.fp32 == arg2->value.fp32;
  case TWEAK_VARIANT_TYPE_DOUBLE:
    return arg1->value.fp64 == arg2->value.fp64;
  case TWEAK_VARIANT_TYPE_STRING:
    return strcmp(tweak_variant_string_c_str(&arg1->value.string),
                  tweak_variant_string_c_str(&arg2->value.string)) == 0;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    if (arg1->value.buffer.size == arg2->value.buffer.size) {
      return memcmp(tweak_buffer_get_data_const(&arg1->value.buffer),
                    tweak_buffer_get_data_const(&arg2->value.buffer),
                    arg1->value.buffer.size) == 0;
    } else {
      return false;
    }
  }
  return false;
}

size_t tweak_variant_get_item_count(const tweak_variant* arg)
{
  switch (arg->type) {
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
    return 1;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return arg->value.buffer.size / sizeof(int8_t);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return arg->value.buffer.size / sizeof(int16_t);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return arg->value.buffer.size / sizeof(int32_t);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return arg->value.buffer.size / sizeof(int64_t);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return arg->value.buffer.size / sizeof(uint8_t);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return arg->value.buffer.size / sizeof(uint16_t);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return arg->value.buffer.size / sizeof(uint32_t);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return arg->value.buffer.size / sizeof(uint64_t);
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return arg->value.buffer.size / sizeof(float);
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return arg->value.buffer.size / sizeof(double);
  }
  TWEAK_FATAL("Unknown type : %d", arg->type);
  return 0;
}

void tweak_variant_destroy(tweak_variant* variant) {
  if (!variant)
    return;

  assert(tweak_variant_is_valid(variant));

  switch (variant->type) {
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
    /* no op */
    break;
  case TWEAK_VARIANT_TYPE_STRING:
    tweak_variant_destroy_string(&variant->value.string);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    tweak_buffer_destroy(&variant->value.buffer);
    break;
  }
  memset(variant, 0, sizeof(*variant));
  variant->type = TWEAK_VARIANT_TYPE_NULL;
}
