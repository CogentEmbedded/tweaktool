/**
 * @file main.c
 * @ingroup tweak-internal
 *
 * @brief test suite for tweak2::transport library.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>
#include <tweak2/pickle_client.h>
#include <tweak2/pickle_server.h>

#include <acutest.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int32_t answer_count = false;

static pthread_mutex_t lock;

static pthread_cond_t cond;

void reset_answer_count(int32_t new_value) {
  pthread_mutex_lock(&lock);
  answer_count = new_value;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
}

void notify_answer_received() {
  pthread_mutex_lock(&lock);
  --answer_count;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
}

void wait_answers() {
  pthread_mutex_lock(&lock);
  while (answer_count > 0) {
    pthread_cond_wait(&cond, &lock);
  }
  pthread_mutex_unlock(&lock);
}


static void
connection_state_listener_impl(tweak_pickle_connection_state connection_state,
                               void *cookie)
{
  notify_answer_received();
}

static void handle_subscribe_impl(tweak_pickle_subscribe* subscribe,
  void *cookie)
{
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&subscribe->uri_patterns), "*") == 0);
  notify_answer_received();
}

static void handle_client_change_tweak_impl(tweak_pickle_change_item *change,
  void *cookie)
{
  TEST_CHECK(change->tweak_id == 11);
  TWEAK_LOG_TEST("change->tweak_id is correct\n");
  TEST_CHECK(change->value.type == TWEAK_VARIANT_TYPE_FLOAT);
  TWEAK_LOG_TEST("change->value.type is correct\n");
  TEST_CHECK(abs(change->value.fp32 - 32.f) < .0001f);
  TWEAK_LOG_TEST("change->value.fp32 is correct\n");
  notify_answer_received();
}

static void
  handle_server_change_tweak_impl(tweak_pickle_change_item *change_item,
    void *cookie)
{
  TEST_CHECK(change_item->tweak_id == 17);
  TWEAK_LOG_TEST("change_item->tweak_i is correct\n");
  TEST_CHECK(change_item->value.type == TWEAK_VARIANT_TYPE_FLOAT);
  TWEAK_LOG_TEST("change_item->value.type is correct\n");
  TEST_CHECK(abs(change_item->value.fp32 - 36.f) < .00001f );
  TWEAK_LOG_TEST("change_item->value.fp32 is correct\n");
  notify_answer_received();
}

static void
  handle_add_item_impl(tweak_pickle_add_item *add_item,
    void *cookie)
{
  TEST_CHECK(add_item->tweak_id == 26);
  TWEAK_LOG_TEST("add_item->tweak_id is correct\n");
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&add_item->uri), "X") == 0);
  TWEAK_LOG_TEST("add_item->uri is correct\n");
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&add_item->meta), "spinbox") == 0);
  TWEAK_LOG_TEST("add_item->meta is correct\n");
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&add_item->description), "First control") == 0);
  TWEAK_LOG_TEST("add_item->description is correct\n");
  TEST_CHECK(add_item->current_value.type == TWEAK_VARIANT_TYPE_FLOAT);
  TWEAK_LOG_TEST("add_item->current_value.type is correct\n");
  TEST_CHECK(add_item->current_value.fp32 == 17.f);
  TWEAK_LOG_TEST("add_item->current_value.fp32 is correct\n");
  notify_answer_received();
}

static void
  handle_server_remove_tweak(tweak_pickle_remove_item* pickle_remove_item,
    void *cookie)
{
  TEST_CHECK(pickle_remove_item->tweak_id == 42);
  TWEAK_LOG_TEST("remove tweak_id is correct\n");
  notify_answer_received();
}

static tweak_variant_string create_variant_string(const char* arg) {
  tweak_variant_string rv = { 0 };
  tweak_variant_assign_string(&rv, arg);
  return rv;
}

void test_pickle() {
  srand(time(NULL));
  int port = 32769 + rand() % 20000;
  char uri0[256];
  snprintf(uri0, sizeof(uri0), "tcp://0.0.0.0:%d/", port);

  reset_answer_count(2);
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);
  tweak_pickle_server_descriptor server_descriptor = {
    .context_type = "nng",
    .uri = uri0,
    .params = "role=server",
    .skeleton = {
      .connection_state_listener = {
        .callback = &connection_state_listener_impl
      },
      .subscribe_listener = {
        .callback = &handle_subscribe_impl
      },
      .change_item_listener = {
        .callback = &handle_client_change_tweak_impl
      }
    }
  };

  tweak_pickle_server_endpoint sep = tweak_pickle_create_server_endpoint(&server_descriptor);
  TEST_CHECK(sep != NULL);

  tweak_pickle_client_descriptor client_descriptor = {
    .context_type = "nng",
    .uri = uri0,
    .params = "role=client",
    .skeleton = {
      .connection_state_listener = {
        .callback = &connection_state_listener_impl
      },
      .add_item_listener = {
        .callback = &handle_add_item_impl
      },
      .change_item_listener = {
        .callback = &handle_server_change_tweak_impl
      },
      .remove_item_listener = {
        .callback = &handle_server_remove_tweak
      }
    }
  };

  reset_answer_count(1);
  tweak_pickle_client_endpoint cep = tweak_pickle_create_client_endpoint(&client_descriptor);
  TEST_CHECK(sep != NULL);

  wait_answers();

  tweak_pickle_add_item new_item = {
    .uri = create_variant_string("X"),
    .tweak_id = 26,
    .meta = create_variant_string("spinbox"),
    .description = create_variant_string("First control"),
    .default_value = {
      .type = TWEAK_VARIANT_TYPE_FLOAT,
      .fp32 = 3.f
    },
    .current_value = {
      .type = TWEAK_VARIANT_TYPE_FLOAT,
      .fp32 = 17.f
    }
  };

  tweak_pickle_change_item change_item = {
    .tweak_id = 17,
    .value = {
      .type = TWEAK_VARIANT_TYPE_FLOAT,
      .fp32 = 36.f
    }
  };

  tweak_pickle_change_item change_item_client = {
    .tweak_id = 11,
    .value = {
      .type = TWEAK_VARIANT_TYPE_FLOAT,
      .fp32 = 32.f
    }
  };

  reset_answer_count(1);
  TWEAK_LOG_TEST("test server_add_item...");
  tweak_pickle_server_add_item(sep, &new_item);
  wait_answers();
  TWEAK_LOG_TEST("DONE\n");

  reset_answer_count(1);
  TWEAK_LOG_TEST("test push_item_changes...");
  tweak_pickle_server_change_item(sep, &change_item);
  wait_answers();
  TWEAK_LOG_TEST("DONE\n");

  reset_answer_count(1);
  TWEAK_LOG_TEST("test remove_item...");
  tweak_pickle_remove_item pickle_remove_item = {
    .tweak_id = 42
  };
  tweak_pickle_server_remove_item(sep, &pickle_remove_item);
  wait_answers();
  TWEAK_LOG_TEST("DONE\n");

  reset_answer_count(1);
  TWEAK_LOG_TEST("test client_subscribe...");
  tweak_pickle_client_subscribe(cep, NULL);
  wait_answers();
  TWEAK_LOG_TEST("DONE\n");

  reset_answer_count(1);
  TWEAK_LOG_TEST("test client_change_item...");
  tweak_pickle_client_change_item(cep, &change_item_client);
  wait_answers();
  TWEAK_LOG_TEST("DONE\n");

  tweak_pickle_destroy_client_endpoint(cep);
  tweak_pickle_destroy_server_endpoint(sep);

  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&lock);
}

TEST_LIST = {
   { "test-pickle", test_pickle },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};

