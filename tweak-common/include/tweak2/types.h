/**
 * @file types.h
 * @ingroup tweak-api
 *
 * @brief Universal definitions.
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
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_COMMON_H_INCLUDED
#define TWEAK_COMMON_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

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
  TWEAK_VARIANT_TYPE_NULL = 0x0,
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
  TWEAK_VARIANT_TYPE_STRING,
  TWEAK_VARIANT_TYPE_VECTOR_SINT8,
  TWEAK_VARIANT_TYPE_VECTOR_SINT16,
  TWEAK_VARIANT_TYPE_VECTOR_SINT32,
  TWEAK_VARIANT_TYPE_VECTOR_SINT64,
  TWEAK_VARIANT_TYPE_VECTOR_UINT8,
  TWEAK_VARIANT_TYPE_VECTOR_UINT16,
  TWEAK_VARIANT_TYPE_VECTOR_UINT32,
  TWEAK_VARIANT_TYPE_VECTOR_UINT64,
  TWEAK_VARIANT_TYPE_VECTOR_FLOAT,
  TWEAK_VARIANT_TYPE_VECTOR_DOUBLE,
} tweak_variant_type;

#ifdef __cplusplus
}
#endif

#endif
