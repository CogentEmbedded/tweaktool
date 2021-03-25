/**
 * @file types.h
 * @ingroup tweak-api
 *
 * @brief Universal definitions.
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

#ifndef TWEAK_COMMON_H_INCLUDED
#define TWEAK_COMMON_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Unique identifier of a tweak item in client server pair.
 */
typedef uint64_t tweak_id;

enum {
  /**
   * @brief Default value for @p tweak_id or error indicator.
   */
  TWEAK_INVALID_ID = 0,
};

/**
 * @brief Type discriminator for tweak variant type.
 */
typedef enum {
  TWEAK_VARIANT_TYPE_IS_NULL = 0x0,
  TWEAK_VARIANT_TYPE_BOOL,
  TWEAK_VARIANT_TYPE_SINT8,
  TWEAK_VARIANT_TYPE_SINT16,
  TWEAK_VARIANT_TYPE_SINT32,
  TWEAK_VARIANT_TYPE_SINT64,
  TWEAK_VARIANT_TYPE_UINT8,
  TWEAK_VARIANT_TYPE_UINT16,
  TWEAK_VARIANT_TYPE_UINT32,
  TWEAK_VARIANT_TYPE_UINT64,
  TWEAK_VARIANT_TYPE_FLOAT,
  TWEAK_VARIANT_TYPE_DOUBLE,
} tweak_variant_type;

#ifdef __cplusplus
}
#endif

#endif
