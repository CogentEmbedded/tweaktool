/**
 * @file test-app.c
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

#include <tweak2/appclient.h>
#include <tweak2/appserver.h>
#include <tweak2/log.h>
#include <tweak2/string.h>
#include <tweak2/variant.h>

#include <acutest.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

enum { NUM_TWEAKS = 1000 };

enum { WAIT_MILLIS = 1000 };

enum { MAX_URI_LENGTH = 500 };

enum { CHANGE_ITERATIONS = 100 };

enum { CHANGE_BATCH_SIZE = 10 };

static void* client_loop(void *arg);

struct id_value_pair {
  tweak_id tweak_id;
  tweak_variant value;
};

struct client_control {
  char* uri;
  pthread_mutex_t lock;
  pthread_cond_t start_client_cond;
  bool start_client;
  pthread_cond_t change_items_cond;
  bool change_items;
  pthread_cond_t stop_server_exchange_cond;
  bool stop_server_exchange;
  pthread_cond_t wait_items_cond;
  uint32_t items_count;
  pthread_cond_t pairs_cond;
  struct id_value_pair* pairs;
  uint32_t pairs_count;
};

static void initialize_client_control(struct client_control* client_control, const char* uri) {
  client_control->uri = strdup(uri);
  pthread_mutex_init(&client_control->lock, NULL);
  pthread_cond_init(&client_control->start_client_cond, NULL);
  client_control->start_client = false;
  pthread_cond_init(&client_control->change_items_cond, NULL);
  client_control->change_items = false;
  pthread_cond_init(&client_control->stop_server_exchange_cond, NULL);
  client_control->stop_server_exchange = false;
  pthread_cond_init(&client_control->wait_items_cond, NULL);
  client_control->items_count = 0;
  pthread_cond_init(&client_control->pairs_cond, NULL);
  client_control->pairs = NULL;
  client_control->pairs_count = 0;
}

static void request_stop_server_exchange(struct client_control* client_control) {
  pthread_mutex_lock(&client_control->lock);
  client_control->stop_server_exchange = true;
  pthread_cond_broadcast(&client_control->stop_server_exchange_cond);
  pthread_mutex_unlock(&client_control->lock);
}

static void wait_stop_server_exchange(struct client_control* client_control, uint64_t millis) {
  pthread_mutex_lock(&client_control->lock);
  while (!client_control->stop_server_exchange) {
    pthread_cond_wait(&client_control->stop_server_exchange_cond, &client_control->lock);
  }
  pthread_mutex_unlock(&client_control->lock);
}

static void request_client_start(struct client_control* client_control) {
  pthread_mutex_lock(&client_control->lock);
  client_control->start_client = true;
  pthread_cond_broadcast(&client_control->start_client_cond);
  pthread_mutex_unlock(&client_control->lock);
}

static void wait_client_start(struct client_control* client_control, uint64_t millis) {
  pthread_mutex_lock(&client_control->lock);
  while (!client_control->start_client) {
    pthread_cond_wait(&client_control->start_client_cond, &client_control->lock);
  }
  pthread_mutex_unlock(&client_control->lock);
}

static void request_client_change_items(struct client_control* client_control) {
  pthread_mutex_lock(&client_control->lock);
  client_control->change_items = true;
  pthread_cond_broadcast(&client_control->change_items_cond);
  pthread_mutex_unlock(&client_control->lock);
}

static void wait_client_change_items(struct client_control* client_control, uint64_t millis) {
  pthread_mutex_lock(&client_control->lock);
  while (!client_control->change_items) {
    pthread_cond_wait(&client_control->change_items_cond, &client_control->lock);
  }
  pthread_mutex_unlock(&client_control->lock);
}

static uint32_t get_client_item_count(struct client_control* client_control) {
  uint32_t result;
  pthread_mutex_lock(&client_control->lock);
  result = client_control->items_count;
  pthread_mutex_unlock(&client_control->lock);
  return result;
}

static void update_client_item_count(struct client_control* client_control, uint32_t item_count) {
  pthread_mutex_lock(&client_control->lock);
  client_control->items_count = item_count;
  pthread_cond_broadcast(&client_control->wait_items_cond);
  pthread_mutex_unlock(&client_control->lock);
}

static uint32_t wait_client_item_count(struct client_control* client_control, uint32_t num_tweaks, uint64_t millis) {
  uint32_t result = 0;
  pthread_mutex_lock(&client_control->lock);
  while (client_control->items_count != num_tweaks) {
    pthread_cond_wait(&client_control->wait_items_cond, &client_control->lock);
  }
  result = client_control->items_count;
  pthread_mutex_unlock(&client_control->lock);
  return result;
}

static void destroy_client_control(struct client_control* client_control) {
  pthread_mutex_destroy(&client_control->lock);
  pthread_cond_destroy(&client_control->change_items_cond);
  pthread_cond_destroy(&client_control->wait_items_cond);
  pthread_cond_destroy(&client_control->start_client_cond);
  pthread_cond_destroy(&client_control->stop_server_exchange_cond);
  free(client_control->uri);
}

static bool count_item_callback(const tweak_app_item_snapshot* snapshot, void* cookie) {
  uint32_t* counter = cookie;
  ++(*counter);
  return true;
}

static uint32_t count_items(tweak_app_context context) {
  uint32_t counter = 0U;
  tweak_app_traverse_items(context, &count_item_callback, &counter);
  return counter;
}

static const char char_table[] = "/0123456789ABCDEFHLMNOQRSTUVWXYZ";

static uint32_t generate_random_uri(char *uri, uint32_t min, uint32_t max) {
  uint32_t length = rand() % max;
  if (length < min) {
    length = min;
  }
  for (uint32_t ix = 0; ix < length; ++ix) {
    uri[ix] = char_table[rand() % (sizeof(char_table) - 1)];
  }
  uri[length] = '\0';
  return length;
}

static void generate_tweaks(tweak_app_server_context server_context, uint32_t num_tweaks) {
  for (uint32_t ix = 0U; ix < num_tweaks; ++ix) {
    char uri[MAX_URI_LENGTH + 1];
    memset(uri, '\0', sizeof(uri));
    generate_random_uri(uri, 10, MAX_URI_LENGTH);
    tweak_variant value = {};
    float rand_val = (float)rand() / (RAND_MAX / 2) - 1.f;
    tweak_variant_create_float(&value, rand_val);
    tweak_id tweak_id = tweak_app_server_add_item(server_context, uri, "test", "test", &value,
      NULL);
    TEST_CHECK(tweak_id != TWEAK_INVALID_ID);
  }
}

struct tweak_enum_context {
  tweak_id* tweak_ids;
  uint32_t num_tweaks;
};

static bool tweak_ids_callback(const tweak_app_item_snapshot* snapshot, void* cookie) {
  struct tweak_enum_context* enum_context = cookie;
  enum_context->tweak_ids = realloc(enum_context->tweak_ids,
    (enum_context->num_tweaks + 1) * sizeof(*enum_context->tweak_ids));
  TEST_CHECK(enum_context->tweak_ids != NULL);
  enum_context->tweak_ids[enum_context->num_tweaks] = snapshot->id;
  ++enum_context->num_tweaks;
  return true;
}

static void enumerate_tweak_ids(tweak_app_server_context server_context,
  tweak_id** p_tweak_ids, uint32_t* num_tweaks)
{
  struct tweak_enum_context tweak_enum_context = {};
  tweak_app_traverse_items(server_context, &tweak_ids_callback, &tweak_enum_context);
  *p_tweak_ids = tweak_enum_context.tweak_ids;
  *num_tweaks = tweak_enum_context.num_tweaks;
}

static void shuffle_tweak_ids(tweak_id* tweak_ids, uint32_t num_tweaks) {
  for (uint32_t ix = 0; ix < num_tweaks; ++ix) {
    uint32_t swap_ix = rand() % num_tweaks;
    tweak_id tmp = tweak_ids[ix];
    tweak_ids[ix] = tweak_ids[swap_ix];
    tweak_ids[swap_ix] = tmp;
  }
}

static void remove_random_tweaks(tweak_app_server_context server_context, uint32_t num_tweaks_to_remove) {
  tweak_id* tweak_ids = NULL;
  uint32_t num_tweaks;
  enumerate_tweak_ids(server_context, &tweak_ids, &num_tweaks);
  TEST_CHECK(tweak_ids != NULL);
  TEST_CHECK(num_tweaks > num_tweaks_to_remove);
  shuffle_tweak_ids(tweak_ids, num_tweaks);
  for (uint32_t ix = 0; ix < num_tweaks_to_remove; ++ix) {
    bool item_removed = tweak_app_server_remove_item(server_context, tweak_ids[ix]);
    TEST_CHECK(item_removed);
  }
  free(tweak_ids);
}

static void populate_random_ids(tweak_app_server_context server_context, tweak_id* tweak_ids, uint32_t num_tweaks) {
  tweak_id* tweak_ids0 = NULL;
  uint32_t num_tweaks0 = 0UL;
  enumerate_tweak_ids(server_context, &tweak_ids0, &num_tweaks0);
  TEST_CHECK(num_tweaks0 >= num_tweaks);
  shuffle_tweak_ids(tweak_ids0, num_tweaks0);
  for (uint32_t ix = 0; ix < num_tweaks; ++ix) {
    tweak_ids[ix] = tweak_ids0[ix];
  }
  free(tweak_ids0);
}

static void clear_change_counter(struct client_control* client_control) {
  pthread_mutex_lock(&client_control->lock);
  client_control->pairs_count = 0;
  pthread_cond_broadcast(&client_control->pairs_cond);
  pthread_mutex_unlock(&client_control->lock);
}

static void wait_change_count(struct client_control* client_control, uint32_t count) {
  pthread_mutex_lock(&client_control->lock);
  while (client_control->pairs_count != count) {
    pthread_cond_wait(&client_control->pairs_cond, &client_control->lock);
  }
  pthread_mutex_unlock(&client_control->lock);
}

bool compare_pairs_set(struct id_value_pair* set1, struct id_value_pair* set2, uint32_t count) {
  for (uint32_t ix = 0; ix < count; ++ix) {
    for (uint32_t iy = 0; iy < count; ++iy) {
      if (set1[ix].tweak_id == set1[iy].tweak_id) {
        if (set1[ix].value.type != TWEAK_VARIANT_TYPE_FLOAT)
            return false;

        if (set2[iy].value.type != TWEAK_VARIANT_TYPE_FLOAT)
            return false;

        if (set1[ix].value.fp32 != set2[iy].value.fp32)
            return false;
      }
    }
  }
  return true;
}

static void item_changed_handler(tweak_app_context context,
  tweak_id id, tweak_variant* value, void *cookie)
{
  struct client_control* client_control = cookie;
  pthread_mutex_lock(&client_control->lock);
  client_control->pairs = realloc(client_control->pairs, (client_control->pairs_count + 1) * sizeof(*client_control->pairs));
  client_control->pairs[client_control->pairs_count].tweak_id = id;
  client_control->pairs[client_control->pairs_count].value = tweak_variant_copy(value);
  ++client_control->pairs_count;
  pthread_cond_broadcast(&client_control->pairs_cond);
  pthread_mutex_unlock(&client_control->lock);
}

void test_app() {
  srand(time(NULL));
  int port = 32769 + rand() % 20000;
  char uri0[256];
  snprintf(uri0, sizeof(uri0), "tcp://0.0.0.0:%d/", port);

  struct client_control client_control = {};
  initialize_client_control(&client_control, uri0);

  tweak_app_server_callbacks server_callbacks = {
    .cookie = &client_control,
    .on_current_value_changed = &item_changed_handler
  };
  tweak_app_server_context server_context = tweak_app_create_server_context("nng", "role=server",
    uri0, &server_callbacks);

  TWEAK_LOG_TEST("Create thread...");
  pthread_t thread;
  int status = pthread_create(&thread, NULL, &client_loop, &client_control);
  TEST_CHECK(status == 0);

  uint32_t item_count;
  item_count = count_items(server_context);
  TEST_CHECK(item_count == 0);

  TWEAK_LOG_TEST("Generate items..");
  generate_tweaks(server_context, NUM_TWEAKS / 2);

  TWEAK_LOG_TEST("Starting client..");
  request_client_start(&client_control);

  TWEAK_LOG_TEST("Generate more items..");
  generate_tweaks(server_context, NUM_TWEAKS / 2);

  item_count = count_items(server_context);
  TEST_CHECK(item_count == NUM_TWEAKS);

  item_count = wait_client_item_count(&client_control, NUM_TWEAKS, WAIT_MILLIS);
  TEST_CHECK(item_count == NUM_TWEAKS);

  TWEAK_LOG_TEST("Remove some items..");
  remove_random_tweaks(server_context, NUM_TWEAKS / 5);
  item_count = count_items(server_context);

  TEST_CHECK(item_count == NUM_TWEAKS - NUM_TWEAKS / 5);

  item_count = wait_client_item_count(&client_control, NUM_TWEAKS - NUM_TWEAKS / 5, WAIT_MILLIS);
  TEST_CHECK(item_count == NUM_TWEAKS - NUM_TWEAKS / 5);

  for (uint32_t itr_no = 0; itr_no < CHANGE_ITERATIONS; ++itr_no) {
    TWEAK_LOG_TEST("Change items iteration %d..\n", itr_no);
    tweak_id tweak_ids[CHANGE_BATCH_SIZE];
    struct id_value_pair pairs[CHANGE_BATCH_SIZE];
    populate_random_ids(server_context, tweak_ids, CHANGE_BATCH_SIZE);
    clear_change_counter(&client_control);
    for (uint32_t ix = 0; ix < CHANGE_BATCH_SIZE; ++ix) {
      tweak_variant value = {};
      float rand_val = (float)rand() / (RAND_MAX / 2) - 1.f;
      tweak_variant_create_float(&pairs[ix].value, rand_val);
      pairs[ix].tweak_id = tweak_ids[ix];
      tweak_variant tmp = tweak_variant_copy(&pairs[ix].value);
      tweak_app_item_replace_current_value(server_context, tweak_ids[ix], &tmp);
    }
    wait_change_count(&client_control, CHANGE_BATCH_SIZE);
    TEST_CHECK(client_control.pairs_count == CHANGE_BATCH_SIZE);
    TEST_CHECK(compare_pairs_set(pairs, client_control.pairs, CHANGE_BATCH_SIZE));
  }

  TWEAK_LOG_TEST("Generate even some more items..");

  generate_tweaks(server_context, NUM_TWEAKS / 5);
  item_count = wait_client_item_count(&client_control, NUM_TWEAKS, WAIT_MILLIS);
  TEST_CHECK(item_count == NUM_TWEAKS);

  item_count = count_items(server_context);
  TEST_CHECK(item_count == NUM_TWEAKS);

  TWEAK_LOG_TEST("Testing reverse scenario..");
  request_client_change_items(&client_control);

  pthread_join(thread, NULL);
  TWEAK_LOG_TEST("Shutdown..");

  tweak_app_destroy_context(server_context);

  destroy_client_control(&client_control);
}

static void client_loop_item_added(tweak_app_context context,
  tweak_id id, void *cookie)
{
  struct client_control* client_control = cookie;
  uint32_t c = get_client_item_count(client_control);
  update_client_item_count(client_control, c + 1);
}

static void client_loop_item_removed(tweak_app_context context,
  tweak_id id, void *cookie)
{
  struct client_control* client_control = cookie;
  uint32_t c = get_client_item_count(client_control);
  update_client_item_count(client_control, c - 1);
}

static void* client_loop(void *arg) {
  struct client_control* client_control = arg;

  wait_client_start(client_control, UINT64_MAX);

  tweak_app_client_callbacks client_callbacks = {
    .cookie = client_control,
    .on_new_item = &client_loop_item_added,
    .on_current_value_changed = &item_changed_handler,
    .on_item_removed = &client_loop_item_removed
  };

  tweak_app_client_context client_context = tweak_app_create_client_context("nng", "role=client",
    client_control->uri, &client_callbacks);

  wait_client_change_items(client_control, UINT64_MAX);

  for (uint32_t itr_no = 0; itr_no < CHANGE_ITERATIONS; ++itr_no) {
    TWEAK_LOG_TEST("Change items iteration %d..\n", itr_no);
    tweak_id tweak_ids[CHANGE_BATCH_SIZE];
    struct id_value_pair pairs[CHANGE_BATCH_SIZE];
    populate_random_ids(client_context, tweak_ids, CHANGE_BATCH_SIZE);
    clear_change_counter(client_control);
    for (uint32_t ix = 0; ix < CHANGE_BATCH_SIZE; ++ix) {
      tweak_variant value = {};
      float rand_val = (float)rand() / (RAND_MAX / 2) - 1.f;
      tweak_variant_create_float(&pairs[ix].value, rand_val);
      pairs[ix].tweak_id = tweak_ids[ix];
      tweak_variant tmp = tweak_variant_copy(&pairs[ix].value);
      tweak_app_item_replace_current_value(client_context, tweak_ids[ix], &tmp);
    }
    wait_change_count(client_control, CHANGE_BATCH_SIZE);
    TEST_CHECK(client_control->pairs_count == CHANGE_BATCH_SIZE);
    TEST_CHECK(compare_pairs_set(pairs, client_control->pairs, CHANGE_BATCH_SIZE));
  }

  tweak_app_destroy_context((tweak_app_context)client_context);
}

TEST_LIST = {
   { "test-app", test_app },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};
