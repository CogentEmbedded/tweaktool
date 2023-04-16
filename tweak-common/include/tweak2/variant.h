/**
 * @file variant.h
 * @ingroup tweak-api
 *
 * @brief Tweak variant type.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#ifndef TWEAK_VARIANT_H_INCLUDED
#define TWEAK_VARIANT_H_INCLUDED

#include <tweak2/log.h>
#include <tweak2/string.h>
#include <tweak2/buffer.h>
#include <tweak2/types.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tweak variant type.
 * Used for tweak item values.
 */
typedef struct {
  tweak_variant_type type;
  union {
    bool b;
    int8_t sint8;
    int16_t sint16;
    int32_t sint32;
    int64_t sint64;
    uint8_t uint8;
    uint16_t uint16;
    uint32_t uint32;
    uint64_t uint64;
    float fp32;
    double fp64;
    tweak_variant_string string;
    struct tweak_variant_buffer buffer;
  } value;
} tweak_variant;

#define TWEAK_VARIANT_INIT_EMPTY { TWEAK_VARIANT_TYPE_NULL, { 0 } }

/**
 * @brief Checks if variant instance is initialized and
 * has value known to this particular build of tweak library.
 *
 * @param[in] arg variant instance to check.
 *
 * @return true in @p arg is valid
 */
static inline bool tweak_variant_is_valid(const tweak_variant* arg) {
  switch(arg->type) {
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
    return true;
  }
  return false;
}

/**
 * @brief Swaps variants. Needed for move semantics to pass ownership
 * of the variant value.
 *
 * @param[in] variant1 pointer to value being replaced by @p variant2.
 * @param[in] variant2 pointer to value being replaced by @p variant1.
 */
static inline void tweak_variant_swap(tweak_variant* variant1, tweak_variant* variant2) {
  tweak_variant tmp = *variant1;
  *variant1 = *variant2;
  *variant2 = tmp;
}

/**
 * @brief Destroys variant instance.
 *
 * @param[in] variant value to deallocate.
 */
void tweak_variant_destroy(tweak_variant* variant);

/**
 * @brief assign bool value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_bool(tweak_variant* variant, bool arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_BOOL;
  tmp.value.b = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign int8_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_sint8(tweak_variant* variant, int8_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_SINT8;
  tmp.value.sint8 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign int16_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_sint16(tweak_variant* variant, int16_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type =  TWEAK_VARIANT_TYPE_SINT16;
  tmp.value.sint16 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign int32_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_sint32(tweak_variant* variant, int32_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type =  TWEAK_VARIANT_TYPE_SINT32;
  tmp.value.sint32 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign int64_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_sint64(tweak_variant* variant, int64_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_SINT64;
  tmp.value.sint64 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint8_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_uint8(tweak_variant* variant, uint8_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_UINT8;
  tmp.value.uint8 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint16_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant1 target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_uint16(tweak_variant* variant, uint16_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_UINT16;
  tmp.value.uint16 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint32_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant1 target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_uint32(tweak_variant* variant, uint32_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_UINT32;
  tmp.value.uint32 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint64_t value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_uint64(tweak_variant* variant, uint64_t arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_UINT64,
  tmp.value.uint64 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign float value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_float(tweak_variant* variant, float arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_FLOAT;
  tmp.value.fp32 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign double value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_double(tweak_variant* variant, double arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_DOUBLE;
  tmp.value.fp64 = arg;
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign string value to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_assign_string(tweak_variant* variant, const char* arg) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_STRING,
  tweak_assign_string(&tmp.value.string, arg);
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign int8_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_sint8_vector(tweak_variant* variant, const int8_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_SINT8;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief Checks whether two variant values are equal.
 *
 * @param[in] arg1 first variant value.
 * @param[in] arg2 second variant value.
 */
bool tweak_variant_is_equal(const tweak_variant* arg1, const tweak_variant* arg2);

/**
 * @brief assign int16_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_sint16_vector(tweak_variant* variant, const int16_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_SINT16;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign int32_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_sint32_vector(tweak_variant* variant, const int32_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_SINT32;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign int64_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_sint64_vector(tweak_variant* variant, const int64_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_SINT64;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint8_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_uint8_vector(tweak_variant* variant, const uint8_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_UINT8;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint16_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_uint16_vector(tweak_variant* variant, const uint16_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_UINT16,
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint32_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_uint32_vector(tweak_variant* variant, const uint32_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_UINT32,
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign uint64_t array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_uint64_vector(tweak_variant* variant, const uint64_t* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_UINT64;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign float array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_float_vector(tweak_variant* variant, const float* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_FLOAT;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief assign double array to variant.
 *
 * @note existing non-null value being stored in @p variant shall be destroyed.
 *
 * @param[in] variant target variant.
 * @param[in] arg pointer to vector data.
 * @param[in] size number of elements in vector.
 */
static inline void tweak_variant_assign_double_vector(tweak_variant* variant, const double* arg, size_t size) {
  tweak_variant tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.type = TWEAK_VARIANT_TYPE_VECTOR_DOUBLE;
  tmp.value.buffer = tweak_buffer_create(arg, size * sizeof(arg[0]));
  tweak_variant_swap(&tmp, variant);
  tweak_variant_destroy(&tmp);
}

/**
 * @brief Clones variant value. A user assumes ownership of a new instance.
 *
 * @param[in] variant pointer to value being coped.
 */
tweak_variant tweak_variant_copy(const tweak_variant* variant);

/**
 * @brief Result of a type conversion operation.
 */
typedef enum {
  /**
   * @brief Type conversion success.
   */
  TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS = 0,
  /**
   * @brief Argument lies outside of range and was
   * truncated to nearest representable value.
   */
  TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED,
  /**
   * @brief Can't perform type conversion.
   */
  TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL
} tweak_variant_type_conversion_result;

/**
 * @brief Constructs tweak_variant instance given string and its type.
 *
 * @note Intended to be used in tests or interactive applications.
 *
 * @param arg source string. Its contents are supposed to be parseable to type @p target_type.
 * @param target_type type of value being returned. This function chooses parser suitable
 * to interpret string provided by @p arg parameter.
 * @param out placeholder for a result. It as assumed to be equal to TWEAK_VARIANT_INIT_EMPTY
 * initially, otherwise this function shall fail assertion check.
 *
 * @return conversion status. When it's equal to TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS
 * or TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED, value pointed by @p out is changed.
 * When it's equal to TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL, value pointed by @p out
 * is left as is.
 */
tweak_variant_type_conversion_result tweak_variant_from_string(const char* arg,
  tweak_variant_type target_type, tweak_variant* out);

/**
 * @brief Constructs tweak_variant_string instance given variant argument.
 *
 * @note Intended to be used in tests or interactive applications.
 *
 * @param arg Source variant value.
 *
 * @return String representing value of given variant as string.
 */
tweak_variant_string tweak_variant_to_string(const tweak_variant* arg);

/**
 * @brief Constructs tweak_variant_string instance given variant argument.
 *
 * @note Intended to be used in tests or interactive applications.
 *
 * @param arg Source variant value.
 *
 * @return String representing value of given variant as string in format {type:value}.
 */
tweak_variant_string tweak_variant_to_json(const tweak_variant* arg);

/**
 * @brief Returns number of items in variant instance.
 *
 * @param arg variant value.
 *
 * @return number of items in a vector. returns 1 if type is a scalar.
 */
size_t tweak_variant_get_item_count(const tweak_variant* arg);

#ifdef __cplusplus
}
#endif

#endif
