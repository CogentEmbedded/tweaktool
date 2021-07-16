/**
 * @file test-tweakmodel.c
 * @ingroup tweak-app-implementation-test
 *
 * @brief part of test suite to test tweak2 application implementation.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-app-implementation-test Test implementation for tweak-app internal interfaces.
 */

#include <tweak2/string.h>
#include <tweak2/variant.h>

#include "tweakmodel.h"

#include <acutest.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

enum { NUM_TWEAKS = 10000 };
enum { NUM_ITERATIONS = 100 };
enum { BATCH_SIZE = 100 };

static tweak_id seed = 0;
static tweak_id gen_id() {
  return ++seed;
}

tweak_id ids[NUM_TWEAKS];
tweak_variant_string uris[NUM_TWEAKS];
tweak_variant values[NUM_TWEAKS];

void test_model() {
  srand(time(0));
  tweak_model model = tweak_model_create();
  for (size_t ix = 0; ix < NUM_TWEAKS; ix++) {
    ids[ix] = gen_id();
    char buff[128] = { 0 };
    sprintf(buff, "id_%" PRIu64 "", ids[ix]);
    tweak_variant_assign_string(&uris[ix], buff);

    float val = ((float)rand()) / (float)RAND_MAX;
    tweak_variant_create_float(&values[ix], val);

    tweak_model_error_code ec = tweak_model_create_item(model, ids[ix],
      &uris[ix], &uris[ix], &uris[ix], &values[ix], &values[ix], NULL);

    TEST_CHECK(ec == TWEAK_MODEL_SUCCESS);
  }
  for (int itr = 0; itr < NUM_ITERATIONS; itr++) {
    TWEAK_LOG_TEST("Iteration %d\n", itr);
    for (size_t ix = 0; ix < NUM_TWEAKS; ix++) {
      tweak_id tweak_id = ids[ix];
      tweak_item* item = tweak_model_find_item_by_id(model, tweak_id);
      TEST_CHECK(tweak_id == item->id);
      TEST_CHECK(strcmp(tweak_variant_string_c_str(&uris[ix]), tweak_variant_string_c_str(&item->uri)) == 0);
      TEST_CHECK(item->current_value.type == TWEAK_VARIANT_TYPE_FLOAT);
      TEST_CHECK(item->current_value.value.fp32 == values[ix].value.fp32);
    }
    size_t to_replace[BATCH_SIZE] = { 0 };
    for (size_t ix = 0; ix < BATCH_SIZE; ix++) {
      to_replace[ix] = rand() % NUM_TWEAKS;
    }
    for (size_t iy = 0; iy < BATCH_SIZE; iy++) {
      size_t ix = to_replace[iy];

      tweak_model_error_code error_code;
      error_code = tweak_model_remove_item(model, ids[ix]);
      TEST_CHECK(error_code == TWEAK_MODEL_SUCCESS);

      error_code = tweak_model_remove_item(model, gen_id());
      TEST_CHECK(error_code == TWEAK_MODEL_ITEM_NOT_FOUND);

      ids[ix] = gen_id();
      char buff[128] = { 0 };
      sprintf(buff, "id_%" PRIu64 "", ids[ix]);

      tweak_variant_assign_string(&uris[ix], buff);

      tweak_variant_string uri = tweak_variant_string_copy(&uris[ix]);
      tweak_variant_string description = tweak_variant_string_copy(&uris[ix]);
      tweak_variant_string meta = tweak_variant_string_copy(&uris[ix]);

      float val = ((float)rand()) / (float)RAND_MAX;
      tweak_variant_create_float(&values[ix], val);

      tweak_variant default_value = tweak_variant_copy(&values[ix]);
      tweak_variant current_value = tweak_variant_copy(&values[ix]);

      tweak_model_error_code ec = tweak_model_create_item(model, ids[ix],
        &uri, &description, &meta, &default_value, &current_value, NULL);
      TEST_CHECK(ec == TWEAK_MODEL_SUCCESS);
    }
  }
  tweak_model_destroy(model);
}

TEST_LIST = {
   { "test_model", test_model },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};
