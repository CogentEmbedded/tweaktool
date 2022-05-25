/**
 * @file buffer.h
 * @ingroup tweak-api
 *
 * @brief Buffers for vector data type.
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

#ifndef TWEAK_BUFFER_H_INCLUDED
#define TWEAK_BUFFER_H_INCLUDED

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  /**
   * @brief Matrices 4x4 should be stored inline to avoid heap traffic.
   */
  TWEAK_VARIANT_NUMBER_INLINE_QWORDS = 16
};

enum {
  /**
   * @brief Number of signed octets to keep in inline buffer.
   */
  TWEAK_VARIANT_SMALL_BUFFER_SIZE =  TWEAK_VARIANT_NUMBER_INLINE_QWORDS * sizeof (uint64_t)
};

/**
 * @brief Untyped container for variable length data.
 * Designed as underlying layer for variable length vector data in @see tweak_variant
 * instances.
 * Keeps small data buffers on stack, resorts to heap storage for larger data blocks.
 */
#pragma pack(push, 1)
struct tweak_variant_buffer {
  /**
   * @brief Size of the buffer in bytes.
   */
  size_t size;

  union {
    /**
     * @brief If size <= TWEAK_VARIANT_SMALL_BUFFER_SIZE,
     * the data will be stored here.
     */
    uint8_t small_buffer[ TWEAK_VARIANT_SMALL_BUFFER_SIZE ];

    /**
     * @brief If capacity > TWEAK_VARIANT_SMALL_BUFFER_SIZE,
     * the data will be on heap.
     */
    uint8_t* large_buffer;
  } buffers;
};
#pragma pack(pop)

/**
 * @brief Create a new buffer instance.
 *
 * @param[in] source data. Can be of arbitrary length. Heap memory will be allocated if necessary.
 * Can be NULL for zero initialized buffers.
 *
 * @param[in] size size of buffer pointed by @p source in bytes.
 *
 * @return new buffer instance.
 */
struct tweak_variant_buffer tweak_buffer_create(const void* source, size_t size);

/**
 * @brief Deallocate memory occupied by buffer, if it's on heap.
 *
 * No-op if the buffer is short an thus stored in-place.
 *
 * @param[in] buffer The buffer being deallocated.
 */
void tweak_buffer_destroy(struct tweak_variant_buffer* buffer);

/**
 * @brief Swaps buffer values. Needed to implement move semantics.
 *
 * @param[in] buffer1 This value will be replaced with @p buffer2.
 * @param[in] buffer2 This value will be replaced with @p buffer1.
 */
void tweak_buffer_swap(struct tweak_variant_buffer* buffer1, struct tweak_variant_buffer* buffer2);

/**
 * @brief Clones source buffer. Needed to implement variant copy.
 * @param[in] source_buffer Buffer to clone.
 *
 * @return copy of buffer. Has to be released with @see tweak_buffer_destroy.
 */
struct tweak_variant_buffer tweak_buffer_clone(const struct tweak_variant_buffer* source_buffer);

/**
 * @brief Get size of buffer returned by @see tweak_variant_sint8_buffer_data.
 *
 * @param[in] buffer The buffer instance to access.
 * @return size of buffer in octets.
 */
size_t tweak_buffer_get_size(const struct tweak_variant_buffer* buffer);

/**
 * @brief Get const pointer to underlying buffer to pass into external API.
 *
 * @param[in] buffer The buffer instance to access.
 * @return pointer to const C-style array.
 */
const void* tweak_buffer_get_data_const(const struct tweak_variant_buffer* buffer);

/**
 * @brief Get pointer to underlying buffer to pass into external API.
 *
 * @param[in] buffer The buffer instance to access.
 * @return pointer to C-style array.
 */
void* tweak_buffer_get_data(struct tweak_variant_buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif
