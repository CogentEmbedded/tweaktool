/**
 * @file tweakstring.c
 * @ingroup tweak-api
 *
 * @brief string relater routines.
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

#include <tweak2/string.h>

#include <stdarg.h>

bool tweak_variant_is_small_string(const tweak_variant_string* string) {
  assert(string);
  return string->capacity <= sizeof(string->buffers.small_buffer);
}

void tweak_variant_destroy_string(tweak_variant_string* string) {
  if (string) {
    if (!tweak_variant_is_small_string(string)) {
      free(string->buffers.large_buffer);
    }
    tweak_variant_string empty = { 0 };
    *string = empty;
  }
}

void tweak_assign_string(tweak_variant_string* string, const char* arg) {
  assert(string);
  if (arg) {
    if (string->capacity == 0)
      string->capacity = TWEAK_VARIANT_INLINE_STRING_SIZE;

    size_t length = strlen(arg);
    size_t desired_capacity = length + 1;
    if (desired_capacity > string->capacity) {
      if (!tweak_variant_is_small_string(string))
        free(string->buffers.large_buffer);

      size_t overcommitted_capacity = desired_capacity * 3 / 2;

      string->buffers.large_buffer = (char*)malloc(overcommitted_capacity);
      assert(string->buffers.large_buffer);

      string->capacity = overcommitted_capacity;
    }

    char* destination =
      tweak_variant_is_small_string(string)
        ? string->buffers.small_buffer
        : string->buffers.large_buffer;

    strncpy(destination, arg, string->capacity);
    string->length = length;
  } else {
    tweak_variant_destroy_string(string);
  }
}

void tweak_string_append(tweak_variant_string* string, const char* arg) {
  assert(string);
  assert(arg);

  if (string->capacity == 0)
    string->capacity = TWEAK_VARIANT_INLINE_STRING_SIZE;

  size_t arglen = strlen(arg);
  size_t length = string->length + arglen;
  size_t desired_capacity = length + 1;

  if (desired_capacity > string->capacity) {
    size_t overcommitted_capacity = desired_capacity * 3 / 2;
    char* large_buffer;
    if (tweak_variant_is_small_string(string)) {
      large_buffer = malloc(overcommitted_capacity);
      memcpy(large_buffer, tweak_variant_string_c_str(string), string->length);
      large_buffer[string->length] = '\0';
    } else {
      large_buffer = realloc(string->buffers.large_buffer, overcommitted_capacity);
    }
    assert(large_buffer);
    string->buffers.large_buffer = large_buffer;
    string->capacity = overcommitted_capacity;
  }

  char* destination =
    tweak_variant_is_small_string(string)
      ? string->buffers.small_buffer
      : string->buffers.large_buffer;

  memcpy(&destination[string->length], arg, arglen + 1);
  string->length = length;
}

void tweak_string_format(tweak_variant_string* string, const char* format, ...) {
  assert(string);
  assert(format);
  tweak_variant_string tmp = TWEAK_VARIANT_STRING_EMPTY;
  size_t size;
  va_list args;
  va_start (args, format);
  size = vsnprintf(tmp.buffers.small_buffer, sizeof(tmp.buffers.small_buffer), format, args) + 1;
  va_end (args);
  tmp.length = size - 1;
  if (size <= sizeof(tmp.buffers.small_buffer)) {
    tmp.capacity = sizeof(tmp.buffers.small_buffer);
  } else {
    tmp.buffers.large_buffer = malloc(size);
    if (tmp.buffers.large_buffer == NULL) {
      TWEAK_FATAL("malloc() returned NULL");
    }
    tmp.capacity = size;
    va_start (args, format);
    vsnprintf(tmp.buffers.large_buffer, size, format, args);
    va_end (args);
  }
  tweak_variant_swap_string(&tmp, string);
  tweak_variant_destroy_string(&tmp);
}

void tweak_variant_swap_string(tweak_variant_string* string1, tweak_variant_string* string2) {
  assert(string1);
  assert(string2);
  tweak_variant_string tmp = *string1;
  *string1 = *string2;
  *string2 = tmp;
}

const char* tweak_variant_string_c_str(const tweak_variant_string* string) {
  if (!string)
    return NULL;

  return tweak_variant_is_small_string(string)
    ? string->buffers.small_buffer
    : string->buffers.large_buffer;
}

bool tweak_variant_string_is_empty(const tweak_variant_string* string) {
  assert(string);
  return *tweak_variant_string_c_str(string) == '\0';
}

tweak_variant_string tweak_variant_string_copy(const tweak_variant_string* arg) {
  if (tweak_variant_is_small_string(arg)) {
    return *arg;
  } else {
    size_t capacity = arg->length + 1;
    char* large_buffer = (char*)malloc(capacity);
    if (!large_buffer) {
      TWEAK_FATAL("Can't allocate memory for string copy");
    }
    strncpy(large_buffer, tweak_variant_string_c_str(arg), capacity);
    tweak_variant_string result = {
      .length = arg->length,
      .capacity = capacity,
      {
        .large_buffer = large_buffer
      }
    };
    return result;
  }
}
