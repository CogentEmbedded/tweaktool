/**
 * @file tweakstring.c
 * @ingroup tweak-api
 *
 * @brief string relater routines.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/string.h>

bool tweak_variant_is_small_string(const tweak_variant_string* string) {
  assert(string);
  return string->capacity <= sizeof(string->small_buffer);
}

void tweak_variant_destroy_string(tweak_variant_string* string) {
  if (string) {
    if (!tweak_variant_is_small_string(string)) {
      free(string->large_buffer);
    }
    tweak_variant_string empty = { 0 };
    *string = empty;
  }
}

void tweak_variant_assign_string(tweak_variant_string* string, const char* arg) {
  assert(string);
  if (arg) {
    if (string->capacity == 0)
      string->capacity = TWEAK_VARIANT_INLINE_STRING_SIZE;

    size_t length = strlen(arg);
    size_t desired_capacity = length + 1;
    if (desired_capacity > string->capacity) {
      if (!tweak_variant_is_small_string(string))
        free(string->large_buffer);

      size_t overcommitted_capacity = desired_capacity * 3 / 2;

      string->large_buffer = (char*)malloc(overcommitted_capacity);
      assert(string->large_buffer);

      string->capacity = overcommitted_capacity;
    }

    char* destination =
      tweak_variant_is_small_string(string)
        ? string->small_buffer
        : string->large_buffer;

    strncpy(destination, arg, string->capacity);
    string->length = length;
  } else {
    tweak_variant_destroy_string(string);
  }
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
    ? string->small_buffer
    : string->large_buffer;
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
      .large_buffer = large_buffer
    };
    return result;
  }
}
