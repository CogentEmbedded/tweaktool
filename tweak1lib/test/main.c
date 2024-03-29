/**
 * @file main.c
 * @ingroup tweak-compatibility-implementation-test
 * @brief part of tweak2 - tweak1 compatibility layer test suite.
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
 * @defgroup tweak-compatibility-implementation-test Test suite for tweak2 - tweak1 compatibility layer.
 */

#include <tweak.h>
#include <tweak2/appclient.h>
#include <tweak2/log.h>
#include <tweak2/thread.h>

#include <acutest.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ITEM_COUNT = 20 };

static void tweak_update(const char *name, void *cookie) {
  (void)name;
  (void)cookie;
  TWEAK_LOG_TEST("%s = %.5f, cookie = %p \n", name, tweak_get(name, 0.0), cookie);
}

void test_local(void) {
  TEST_CHECK(tweak_connect());
  tweak_set_update_handler(tweak_update, NULL);
  tweak_add_widget("TAB1");
  tweak_add_layout(800, 0, "First");
  for (int i = 0; i < ITEM_COUNT; i++) {
    char name[256];
    sprintf(name, "value %d", i);
    tweak_add_slider(name, -1.0, 1.0, 0.2, i % 5);
  }
  tweak_add_layout(400, 1, "Secons");
  tweak_add_spinbox("some value 2", -1.0, 1.0, 0.2, 2);
  tweak_add_checkbox("boolean value", 1);
  tweak_add_groupbox("enum value", "variant 1;something else;variant 47", 0);
  tweak_add_widget("TAB2");
  tweak_add_layout(400, 1, "Secons2");
  tweak_add_spinbox("some value 11", -1.0, 1.0, 0.2, 0);
  tweak_add_layout(400, 0, "Secons3");
  tweak_add_spinbox("some value 12", -1.0, 1.0, 0.2, 0);
  tweak_add_spinbox("some value 13", -1.0, 1.0, 0.2, 0);
  tweak_add_spinbox("some value 14", -1.0, 1.0, 0.2, 0);
  tweak_add_layout(400, 1, "Secons4");
  tweak_add_spinbox("some value 15", -1.0, 1.0, 0.2, 1);
  tweak_add_spinbox("some value 16", -1.0, 1.0, 0.2, 2);
  tweak_add_spinbox("some value 17", -1.0, 1.0, 0.2, 3);
  double ground_truth = 3.77;
  tweak_set("value 4", ground_truth);
  double value = tweak_get("value 4", -1.0);
  TEST_CHECK(ground_truth == value);
  tweak_close();
}

struct test_connect_context {
  tweak_common_mutex lock;
  tweak_common_cond cond;
  bool is_connected;
  bool is_updated;
  int item_count;
  tweak_id ids[ITEM_COUNT];
  char expected_change[256];
  double received_value;
};

static void status_changed_callback(tweak_app_context context,
  bool is_connected, void *cookie)
{
  (void)context;
  struct test_connect_context* connect_context = (struct test_connect_context*) cookie;
  tweak_common_mutex_lock(&connect_context->lock);
  connect_context->is_connected = is_connected;
  tweak_common_cond_broadcast(&connect_context->cond);
  tweak_common_mutex_unlock(&connect_context->lock);
}

static void wait_connected(struct test_connect_context* connect_context) {
  tweak_common_mutex_lock(&connect_context->lock);
  while (!connect_context->is_connected) {
    tweak_common_cond_wait(&connect_context->cond, &connect_context->lock);
  }
  tweak_common_mutex_unlock(&connect_context->lock);
}

static void on_new_item_callback(tweak_app_context context,
  tweak_id id, void *cookie)
{
  (void) context;
  struct test_connect_context* connect_context = (struct test_connect_context*) cookie;
  tweak_common_mutex_lock(&connect_context->lock);
  connect_context->ids[connect_context->item_count] = id;
  ++connect_context->item_count;
  tweak_common_cond_broadcast(&connect_context->cond);
  tweak_common_mutex_unlock(&connect_context->lock);
}

static void wait_items(struct test_connect_context* connect_context, int count) {
  tweak_common_mutex_lock(&connect_context->lock);
  while (connect_context->item_count < count) {
    tweak_common_cond_wait(&connect_context->cond, &connect_context->lock);
  }
  tweak_common_mutex_unlock(&connect_context->lock);
}

static void on_current_value_changed_callback(tweak_app_context context,
  tweak_id id, tweak_variant* value, void *cookie)
{
  struct test_connect_context* connect_context = (struct test_connect_context*) cookie;
  tweak_common_mutex_lock(&connect_context->lock);
  while (connect_context->is_updated) {
    tweak_common_cond_wait(&connect_context->cond, &connect_context->lock);
  }
  tweak_app_item_snapshot* snapshot = tweak_app_item_get_snapshot(context, id);
  strcpy(connect_context->expected_change, tweak_variant_string_c_str(&snapshot->uri));
  tweak_app_release_snapshot(context, snapshot);
  connect_context->received_value = value->value.fp64;
  connect_context->is_updated = true;
  tweak_common_cond_broadcast(&connect_context->cond);
  tweak_common_mutex_unlock(&connect_context->lock);
}

static void reset_updated(struct test_connect_context* connect_context) {
  tweak_common_mutex_lock(&connect_context->lock);
  connect_context->is_updated = false;
  tweak_common_mutex_unlock(&connect_context->lock);
}


static void wait_updated(struct test_connect_context* connect_context) {
  tweak_common_mutex_lock(&connect_context->lock);
  while (!connect_context->is_updated) {
    tweak_common_cond_wait(&connect_context->cond, &connect_context->lock);
  }
  tweak_common_mutex_unlock(&connect_context->lock);
}

static void on_item_removed_callback(tweak_app_context context,
  tweak_id id, void *cookie)
{
  (void)context;
  (void)id;
  (void)cookie;
}

void test_connect(void) {
  struct test_connect_context connect_context = { 0 };
  tweak_common_mutex_init(&connect_context.lock);
  tweak_common_cond_init(&connect_context.cond);

  TEST_CHECK(tweak_connect());

  for (int i = 0; i < ITEM_COUNT; i++) {
    char name[256];
    sprintf(name, "value %d", i);
    tweak_add_slider(name, -1.0, 1.0, 0.2, i % 5);
  }

  tweak_app_client_callbacks app_client_callback = {
    .cookie = &connect_context,
    .on_connection_status_changed = &status_changed_callback,
    .on_new_item = &on_new_item_callback,
    .on_current_value_changed = &on_current_value_changed_callback,
    .on_item_removed = &on_item_removed_callback
  };

  tweak_app_client_context app_client_context = tweak_app_create_client_context("nng", "role=client",
    TWEAK_URI, &app_client_callback);

  wait_connected(&connect_context);
  wait_items(&connect_context, ITEM_COUNT);

  for (int i = 0; i < ITEM_COUNT; i++) {
    char name[256];
    sprintf(name, "value %d", i);
    double value = 1. + (double) rand() * 2. / RAND_MAX;
    reset_updated(&connect_context);
    tweak_set(name, value);
    wait_updated(&connect_context);
    char* tail = &connect_context.expected_change[strlen(connect_context.expected_change) - strlen(name)];
    TEST_CHECK(strcmp(tail, name) == 0);
    TEST_CHECK(connect_context.received_value == value);
  }

  tweak_close();

  tweak_app_destroy_context(app_client_context);

  tweak_common_cond_destroy(&connect_context.cond);
  tweak_common_mutex_destroy(&connect_context.lock);
}

static void server_tweak_update_handler(const char* name, void* cookie) {
  struct test_connect_context* connect_context = (struct test_connect_context*) cookie;
  tweak_common_mutex_lock(&connect_context->lock);
  while (connect_context->is_updated) {
    tweak_common_cond_wait(&connect_context->cond, &connect_context->lock);
  }
  strcpy(connect_context->expected_change, name);
  connect_context->received_value = tweak_get(name, 0.);
  connect_context->is_updated = true;
  tweak_common_cond_broadcast(&connect_context->cond);
  tweak_common_mutex_unlock(&connect_context->lock);
}

void test_server_update(void) {
  struct test_connect_context connect_context = { 0 };
  tweak_common_mutex_init(&connect_context.lock);
  tweak_common_cond_init(&connect_context.cond);

  TEST_CHECK(tweak_connect());
  tweak_set_update_handler(&server_tweak_update_handler, &connect_context);

  for (int i = 0; i < ITEM_COUNT; i++) {
    char name[256];
    sprintf(name, "value %d", i);
    tweak_add_slider(name, -1.0, 1.0, 0.2, i % 5);
  }

  tweak_app_client_callbacks app_client_callback = {
    .cookie = &connect_context,
    .on_connection_status_changed = &status_changed_callback,
    .on_new_item = &on_new_item_callback,
    .on_current_value_changed = &on_current_value_changed_callback,
    .on_item_removed = &on_item_removed_callback
  };

  tweak_app_client_context app_client_context = tweak_app_create_client_context("nng", "role=client",
    TWEAK_URI, &app_client_callback);

  wait_connected(&connect_context);
  wait_items(&connect_context, ITEM_COUNT);

  for (int i = 0; i < ITEM_COUNT; i++) {
    char name[256];
    sprintf(name, "value %d", i);
    double value = 1. + (double) rand() * 2. / RAND_MAX;
    tweak_variant tmp =  TWEAK_VARIANT_INIT_EMPTY;
    tweak_variant_assign_double(&tmp, value);
    reset_updated(&connect_context);
    tweak_app_error_code error_code =
      tweak_app_item_replace_current_value(app_client_context, connect_context.ids[i], &tmp);
    TEST_CHECK(error_code == TWEAK_APP_SUCCESS);
    wait_updated(&connect_context);
    TEST_CHECK(strcmp(connect_context.expected_change, name) == 0);
    TEST_CHECK(connect_context.received_value == value);
  }

  tweak_app_destroy_context(app_client_context);

  tweak_close();

  tweak_common_cond_destroy(&connect_context.cond);
  tweak_common_mutex_destroy(&connect_context.lock);
}


TEST_LIST = {
   { "test_local", test_local },
   { "test_connect", test_connect },
   { "test_server_update", test_server_update },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};

