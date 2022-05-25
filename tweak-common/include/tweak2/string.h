/**
 * @file string.h
 * @ingroup tweak-api
 *
 * @brief Arbitrary length string for use within pickle and app layer APIs.
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

#ifndef TWEAK_STRING_H_INCLUDED
#define TWEAK_STRING_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <tweak2/log.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  /**
   * @brief The length of a string that could be stored outside heap memory.
   */
  TWEAK_VARIANT_INLINE_STRING_SIZE = 16 * sizeof (uint64_t) / sizeof(char)
};

/**
 * @brief Arbitrary length string for use within pickle and app layer APIs.
 *
 * @details this string is designed to be one of options for @p tweak_variant
 * container. Since @p tweak_variant type is designed to hold 4x4 matrices of doubles
 * in-place, we could follow similar approach to hold strings of size 128 within
 * instance of @p tweak_variant without any need to allocate memory on heap.
 * @note tweak_variant_string** shouldn't be used for output string parameters
 * within tweak2 API calls, only single level of indirection is desired.
 * Construct @p tweak_variant_string on stack, initialize it with { 0 }.
 * Pass tweak_variant_string* as an output parameter and check whether API
 * placed non-null value to this output parameter with @p tweak_variant_string_is_null.
 */
typedef struct {
  /**
   * @brief Length of the string, without '\0' terinator.
   */
  size_t length;
  /**
   * @brief Current buffer capacity.
   */
  size_t capacity;
  union {
   /**
    * if capacity <= TWEAK_VARIANT_INLINE_STRING_SIZE,
    * the string will be stored here.
    */
    char small_buffer[ TWEAK_VARIANT_INLINE_STRING_SIZE ];

   /**
    * if size > TWEAK_VARIANT_INLINE_STRING_SIZE,
    * the string will be on heap.
    */
    char* large_buffer;
  } buffers;
} tweak_variant_string;

#define TWEAK_VARIANT_STRING_EMPTY { 0 }

/**
 * @brief Checks string storage.
 *
 * @param[in] string A string instance.
 * @return true if the string is small and stored in-place, false is the string is on heap.
 */
bool tweak_variant_is_small_string(const tweak_variant_string* string);

/**
 * @brief Deallocate memory occupied by string, if it's on heap.
 *
 * No-op if the string is short an thus stored in-place.
 *
 * @param[in] string The string being deallocated.
 */
void tweak_variant_destroy_string(tweak_variant_string* string);

/**
 * @brief Assign new value to a string instance.
 *
 * @note If string being assigned is large and this instance
 * does already contain a string buffer with sufficient capacity allocated on heap,
 * it will reuse it. It its capacity is insufficient, it will be enlarged with @p realloc().
 *
 * @param[in] string The string instance to assign value to.
 * @param[in] arg new string value. Can be of arbitrary length.
 * Heap memory will be allocated if necessary.
 */
void tweak_assign_string(tweak_variant_string* string, const char* arg);

/**
 * @brief Append @p arg to the end of @p string, growing its internal buffer if necessary
 * with @see realloc.
 *
 * @param[in] string The string instance to append value to.
 * @param[in] arg C string to append.
 */
void tweak_string_append(tweak_variant_string* string, const char* arg);

/**
 * @brief sprintf-like assign new value to a string instance.
 *
 * @note If string being assigned is large and this instance
 * does already contain a string buffer with sufficient capacity allocated on heap,
 * it will reuse it. It its capacity is insufficient, it will be enlarged with @p realloc().
 *
 * @param[in] string The string instance to assign value to.
 * @param[in] format format of the string. Should match ellipsis parameters.
 */
void tweak_string_format(tweak_variant_string* string, const char* format, ...);

/**
 * @brief Swaps string values. Needed to implement move semantics.
 *
 * @param[in] string1 This value will be replaced with string2.
 * @param[in] string2 This value will be replaced with string1.
 */
void tweak_variant_swap_string(tweak_variant_string* string1, tweak_variant_string* string2);

/**
 * @brief Get pointer to underlying buffer to pass into external API.
 *
 * @param[in] string The string instance to access.
 * @return pointer to const C-style string.
 */
const char* tweak_variant_string_c_str(const tweak_variant_string* string);

/**
 * @brief Check if string is empty.
 *
 * @param[in] string The string instance to check.
 * @return true if the string is equal to "\0".
 */
bool tweak_variant_string_is_empty(const tweak_variant_string* string);

/**
 * @brief Copy string.
 * @param[in] arg source string.
 *
 * @return duplicate string. User becomes new owner.
 */
tweak_variant_string tweak_variant_string_copy(const tweak_variant_string* arg);

#ifdef __cplusplus
}
#endif

#endif
