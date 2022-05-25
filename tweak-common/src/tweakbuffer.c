/**
 * @file tweakbuffer.c
 * @ingroup tweak-api
 *
 * @brief Generic byte buffer for use with tweak-variant arbitrary length types.
 *
 * @copyright 2021-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#include <tweak2/buffer.h>
#include <tweak2/string.h>
#include <tweak2/log.h>

struct tweak_variant_buffer tweak_buffer_create(const void* source, size_t size) {
  assert(size > 0);
  struct tweak_variant_buffer result = { 0 };
  void* dest;
  if (size <= sizeof(result.buffers.small_buffer)) {
    dest = result.buffers.small_buffer;
  } else {
    result.buffers.large_buffer = malloc(size);
    if (!result.buffers.large_buffer) {
        TWEAK_FATAL("malloc() return NULL");
    }
    dest = result.buffers.large_buffer;
  }
  if (source != NULL) {
    memcpy(dest, source, size);
  } else {
    memset(dest, 0, size);
  }
  result.size = size;
  return result;
}

struct tweak_variant_buffer tweak_buffer_clone(const struct tweak_variant_buffer* source_buffer) {
  assert(source_buffer != NULL);
  return tweak_buffer_create(tweak_buffer_get_data_const(source_buffer), tweak_buffer_get_size(source_buffer));
}

void tweak_buffer_destroy(struct tweak_variant_buffer* buffer) {
  assert(buffer != NULL);
  if (buffer->size > sizeof(buffer->buffers.small_buffer)) {
    free(buffer->buffers.large_buffer);
  }
  memset(buffer, 0, sizeof(*buffer));
}

void tweak_buffer_swap(struct tweak_variant_buffer* buffer1, struct tweak_variant_buffer* buffer2) {
  assert(buffer1 != NULL);
  assert(buffer2 != NULL);
  struct tweak_variant_buffer tmp = *buffer1;
  *buffer1 = *buffer2;
  *buffer2 = tmp;
}

size_t tweak_buffer_get_size(const struct tweak_variant_buffer* buffer) {
  assert(buffer != NULL);
  return buffer->size;
}

const void* tweak_buffer_get_data_const(const struct tweak_variant_buffer* buffer) {
  assert(buffer != NULL);
  return buffer->size <= sizeof(buffer->buffers.small_buffer)
      ? buffer->buffers.small_buffer
      : buffer->buffers.large_buffer;
}

void* tweak_buffer_get_data(struct tweak_variant_buffer* buffer) {
  assert(buffer != NULL);
  return buffer->size <= sizeof(buffer->buffers.small_buffer)
      ? buffer->buffers.small_buffer
      : buffer->buffers.large_buffer;
}
