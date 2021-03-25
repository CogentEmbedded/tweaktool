/**
 * @file tweakstring.h
 * @ingroup tweak-api
 *
 * @brief Tweak variant type.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_VARIANT_H_INCLUDED
#define TWEAK_VARIANT_H_INCLUDED

#include <tweak2/log.h>
#include <tweak2/string.h>
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
    bool value_bool;
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
  };
} tweak_variant;

#define TWEAK_VARIANT_INIT_EMPTY { .type = TWEAK_VARIANT_TYPE_IS_NULL, }

/**
 * @brief assign bool value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_bool(tweak_variant* variant, bool arg) {
  variant->type = TWEAK_VARIANT_TYPE_BOOL;
  variant->value_bool = arg;
}

/**
 * @brief assign int8_t value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_sint8(tweak_variant* variant, int8_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_SINT8;
  variant->sint8 = arg;
}

/**
 * @brief assign int16_t value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_sint16(tweak_variant* variant, int16_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_SINT16;
  variant->sint16 = arg;
}

/**
 * @brief assign int32_t value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_sint32(tweak_variant* variant, int32_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_SINT32;
  variant->sint32 = arg;
}

/**
 * @brief assign int64_t value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_sint64(tweak_variant* variant, int64_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_SINT64;
  variant->sint64 = arg;
}

/**
 * @brief assign uint8_t value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_uint8(tweak_variant* variant, uint8_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_UINT8;
  variant->uint8 = arg;
}

/**
 * @brief assign uint16_t value to variant.
 *
 * @param[in] variant1 target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_uint16(tweak_variant* variant, uint16_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_UINT16;
  variant->uint16 = arg;
}

/**
 * @brief assign uint32_t value to variant.
 *
 * @param[in] variant1 target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_uint32(tweak_variant* variant, uint32_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_UINT32;
  variant->uint32 = arg;
}

/**
 * @brief assign uint64_t value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_uint64(tweak_variant* variant, uint64_t arg) {
  variant->type = TWEAK_VARIANT_TYPE_UINT64;
  variant->uint64 = arg;
}

/**
 * @brief assign float value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_float(tweak_variant* variant, float arg) {
  variant->type = TWEAK_VARIANT_TYPE_FLOAT;
  variant->fp32 = arg;
}

/**
 * @brief assign double value to variant.
 *
 * @param[in] variant target variant.
 * @param[in] arg value to assign.
 */
static inline void tweak_variant_create_double(tweak_variant* variant, double arg) {
  variant->type = TWEAK_VARIANT_TYPE_DOUBLE;
  variant->fp64 = arg;
}

/**
 * @brief Swaps variants. Needed for move sematics to pass ownership
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
 * @brief Clones variant value. A user assumes ownership of a new instance.
 *
 * @param[in] variant pointer to value being coped.
 */
static inline tweak_variant tweak_variant_copy(const tweak_variant* variant) {
  switch (variant->type) {
  case TWEAK_VARIANT_TYPE_IS_NULL:
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
  default:
    break;
  }
  tweak_variant null_value = {
    .type = TWEAK_VARIANT_TYPE_IS_NULL
  };
  return null_value;
}

/**
 * @brief Destroys variant instance.
 *
 * @param[in] variant value to deallocate.
 */
static inline void tweak_variant_destroy(tweak_variant* variant) {
  switch (variant->type) {
  case TWEAK_VARIANT_TYPE_IS_NULL:
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
    break;
  default:
    break;
  }
  memset(variant, 0, sizeof(*variant));
  variant->type = TWEAK_VARIANT_TYPE_IS_NULL;
}

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

#ifdef __cplusplus
}
#endif

#endif
