/**
 * @file test-uri-to-index.c
 * @ingroup tweak-app-implementation-test
 *
 * @brief part of test suite to test tweak2 application implementation.
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
#include <tweak2/variant.h>

#include "tweakmodel_uri_to_tweak_id_index.h"

#include <acutest.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { NUM_ENTRIES = 10000 };
enum { NUM_ITERATIONS = 10 };

static tweak_id seed = 0;

static tweak_id gen_id() {
  return ++seed;
}

tweak_variant_string uris[NUM_ENTRIES] = { 0 };
tweak_id ids[NUM_ENTRIES] = { 0 };

static uint32_t generate_random_uri(char *uri, uint32_t min, uint32_t max) {
  uint32_t length = rand() % max;
  if (length < min) {
    length = min;
  }
  uint32_t char_range = 1 + ('9' - '0') + ('Z' - 'A');
  for (uint32_t ix = 0; ix < length; ++ix) {
    uint32_t nchar = rand() % char_range;
    char c;
    if (nchar == 0) {
      c = '/';
    } else if (nchar < (1 + '9' - '0')) {
      c = (char)('0' +  nchar - 1);
    } else {
      c = (char)('A' +  nchar - 1 + '9' - '0');
    }
    uri[ix] = c;
  }
  uri[length] = '\0';
  return length;
}

enum { MAX_URI_LENGTH = 500 };

void test_uri_to_index(void) {
  srand((unsigned)time(NULL));
  tweak_model_uri_to_tweak_id_index index = tweak_model_uri_to_tweak_id_index_create();
  for (uint32_t itr_no = 0; itr_no < NUM_ITERATIONS; ++itr_no) {
    TWEAK_LOG_TEST("Iteration %d of %d\n", itr_no + 1, NUM_ITERATIONS);
    for (uint32_t tweak_no = 0; tweak_no < NUM_ENTRIES; ++tweak_no) {
      tweak_model_index_result index_result;
      char uri[MAX_URI_LENGTH + 1];
      generate_random_uri(uri, 10, MAX_URI_LENGTH);
      tweak_id id = gen_id();
      index_result = tweak_model_uri_to_tweak_id_index_insert(index, uri, id);
      TEST_CHECK(index_result == TWEAK_MODEL_INDEX_SUCCESS);
      tweak_assign_string(&uris[tweak_no], uri);
      ids[tweak_no] = id;
    }

    for (uint32_t tweak_no = 0; tweak_no < NUM_ENTRIES; ++tweak_no) {
      tweak_model_index_result index_result = tweak_model_uri_to_tweak_id_index_insert(index, tweak_variant_string_c_str(&uris[tweak_no]), 0);
      TEST_CHECK(index_result == TWEAK_MODEL_INDEX_KEY_ALREADY_EXISTS);
    }

    for (uint32_t tweak_no = 0; tweak_no < NUM_ENTRIES; ++tweak_no) {
      tweak_id id = tweak_model_uri_to_tweak_id_index_lookup(index, tweak_variant_string_c_str(&uris[tweak_no]));
      TEST_CHECK(id != TWEAK_INVALID_ID);
      TEST_CHECK(id == ids[tweak_no]);
    }

    for (uint32_t tweak_no = 0; tweak_no < NUM_ENTRIES; ++tweak_no) {
      char uri[MAX_URI_LENGTH + 1];
      generate_random_uri(uri, 10, MAX_URI_LENGTH);
      tweak_id id = tweak_model_uri_to_tweak_id_index_lookup(index, uri);
      TEST_CHECK(id == TWEAK_INVALID_ID);
    }

    for (uint32_t tweak_no = 0; tweak_no < NUM_ENTRIES; ++tweak_no) {
      tweak_model_index_result index_result;
      index_result = tweak_model_uri_to_tweak_id_index_remove(index, tweak_variant_string_c_str(&uris[tweak_no]));
      TEST_CHECK(index_result == TWEAK_MODEL_INDEX_SUCCESS);
      tweak_assign_string(&uris[tweak_no], NULL);
      ids[tweak_no] = TWEAK_INVALID_ID;
    }
  }
  tweak_model_uri_to_tweak_id_index_destroy(index);
}

TEST_LIST = {
   { "test-uri-to-index", test_uri_to_index },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};
