/**
 * @file main.c
 * @ingroup tweak-internal-test
 *
 * @brief test suite for tweak wire library.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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
 * @defgroup tweak-internal-test tests for internal tweak2 components.
 */

#include <tweak2/wire.h>
#include <tweak2/thread.h>
#include <tweak2/defaults.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acutest.h>

#define TEST_CLIENTS 2U

static tweak_common_mutex s_lock = { 0 };
static tweak_common_cond s_cond = { 0 };
static tweak_wire_connection_state s_server_conn_state = TWEAK_WIRE_DISCONNECTED;
static tweak_wire_connection_state s_client_conn_state[TEST_CLIENTS] =
    { TWEAK_WIRE_DISCONNECTED };
static unsigned int server_conn_cb_cnt = 0U;
static unsigned int client_conn_cb_cnt[TEST_CLIENTS] = { 0U };

struct receive_buff {
  tweak_common_mutex lock;
  tweak_common_cond cond;
  bool has_value;
  uint8_t* buffer;
  size_t size;
};

static void test_receive_listener(const uint8_t* buffer, size_t size, void * cookie) {
  struct receive_buff* q = cookie;
  tweak_common_mutex_lock(&q->lock);
  q->has_value = true;
  q->buffer = calloc(1, size);
  memcpy(q->buffer, buffer, size);
  q->size = size;
  tweak_common_cond_signal(&q->cond);
  tweak_common_mutex_unlock(&q->lock);
}

static void wait_buffer(struct receive_buff* q) {
  tweak_common_mutex_lock(&q->lock);
  while (!q->has_value) {
    tweak_common_cond_wait(&q->cond, &q->lock);
  }
  tweak_common_mutex_unlock(&q->lock);
}

static void clear_wait_buffer(struct receive_buff* q) {
  tweak_common_mutex_lock(&q->lock);
  free(q->buffer);
  q->buffer = NULL;
  q->size = 0;
  q->has_value = false;
  tweak_common_mutex_unlock(&q->lock);
}

static void server_connection_state_listener(tweak_wire_connection connection,
                                             tweak_wire_connection_state conn_state,
                                             void *cookie)
{
  (void) connection;
  (void) cookie;
  tweak_common_mutex_lock(&s_lock);
  s_server_conn_state = conn_state;
  server_conn_cb_cnt++;
  tweak_common_cond_broadcast(&s_cond);
  tweak_common_mutex_unlock(&s_lock);
}

static void server_wait_connection(void) {
  tweak_common_mutex_lock(&s_lock);
  while (s_server_conn_state != TWEAK_WIRE_CONNECTED) {
    tweak_common_cond_wait(&s_cond, &s_lock);
  }
  tweak_common_mutex_unlock(&s_lock);
}

static void client_connection_state_listener(tweak_wire_connection connection,
                                             tweak_wire_connection_state conn_state,
                                             void *cookie)
{
  (void) connection;
  size_t index = (size_t)cookie;

  tweak_common_mutex_lock(&s_lock);
  s_client_conn_state[index] = conn_state;
  client_conn_cb_cnt[index]++;
  tweak_common_cond_broadcast(&s_cond);
  tweak_common_mutex_unlock(&s_lock);
}

static void client_wait_connection(size_t index) {
  tweak_common_mutex_lock(&s_lock);
  while (s_client_conn_state[index] != TWEAK_WIRE_CONNECTED) {
    tweak_common_cond_wait(&s_cond, &s_lock);
  }
  tweak_common_mutex_unlock(&s_lock);
}

static void initialize(void) {
  tweak_common_mutex_init(&s_lock);
  tweak_common_cond_init(&s_cond);
}

static void finalize(void) {
  tweak_common_cond_destroy(&s_cond);
  tweak_common_mutex_destroy(&s_lock);
}

void test_wire(void) {
  initialize();
  tweak_wire_connection server_context = NULL;
  tweak_wire_connection client_context[TEST_CLIENTS] = { NULL };

  struct receive_buff server_buff = {
    .has_value = false
  };
  struct receive_buff client_buff[TEST_CLIENTS];

  tweak_wire_connection_state server_conn_state;
  unsigned int cb_cnt;

  tweak_common_cond_init(&server_buff.cond);
  tweak_common_mutex_init(&server_buff.lock);

  for(size_t i = 0U; i < TEST_CLIENTS; i++) {
    tweak_common_cond_init(&client_buff[i].cond);
    tweak_common_mutex_init(&client_buff[i].lock);
    client_buff[i].has_value = false;
  }

  tweak_common_mutex_lock(&s_lock);
  server_conn_state = s_server_conn_state;
  tweak_common_mutex_unlock(&s_lock);

  puts("Create listener node...");
  server_context = tweak_wire_create_connection("nng", "role=server",
    TWEAK_DEFAULT_ENDPOINT, &server_connection_state_listener,
    NULL, &test_receive_listener, &server_buff);

  TEST_CHECK(server_context != TWEAK_WIRE_INVALID_CONNECTION);
  TEST_CHECK(server_conn_state == TWEAK_WIRE_DISCONNECTED);
  puts("SUCCESS");

  puts("Transmit datagram from server to non-existing client...");
  uint8_t lost[] = "Lost!";
  TEST_CHECK(tweak_wire_transmit(server_context, lost, sizeof(lost)) == TWEAK_WIRE_SUCCESS);
  puts("Got SUCCESS");

  puts("Create connector 1 node...");
  client_context[0] = tweak_wire_create_connection("nng", "role=client",
    TWEAK_DEFAULT_ENDPOINT, &client_connection_state_listener,
    (void*)0, &test_receive_listener, &client_buff[0]);
  TEST_CHECK(client_context[0] != TWEAK_WIRE_INVALID_CONNECTION);
  puts("SUCCESS");

  puts("Wait for connection...");
  server_wait_connection();
  client_wait_connection(0);
  puts("Connection established");

  tweak_common_mutex_lock(&s_lock);
  server_conn_state = s_server_conn_state;
  cb_cnt = server_conn_cb_cnt;
  tweak_common_mutex_unlock(&s_lock);

  TEST_CHECK(server_conn_state == TWEAK_WIRE_CONNECTED);

  for(size_t i = 1U; i < TEST_CLIENTS; i++) {
    puts("Create connector next node...");
    client_context[i] = tweak_wire_create_connection("nng", "role=client",
      TWEAK_DEFAULT_ENDPOINT, &client_connection_state_listener,
      (void*)i, &test_receive_listener, &client_buff[i]);
    TEST_CHECK(client_context[i] != TWEAK_WIRE_INVALID_CONNECTION);
    puts("SUCCESS");

    puts("Wait for connection...");
    client_wait_connection(i);
    puts("Connection established");

    bool check;
    tweak_common_mutex_lock(&s_lock);
    server_conn_state = s_server_conn_state;
    check = cb_cnt == server_conn_cb_cnt;
    tweak_common_mutex_unlock(&s_lock);

    TEST_CHECK(server_conn_state == TWEAK_WIRE_CONNECTED);
    TEST_CHECK(check);
  }

  for(size_t i = 0U; i < TEST_CLIENTS; i++) {
    printf("Transmit datagram from client %zu to server...\n", i);
    const char *data = "Hello!";
    tweak_wire_transmit(client_context[i], (const uint8_t*)data, strlen(data));
    wait_buffer(&server_buff);

    TEST_CHECK(strncmp(data, (const char *)server_buff.buffer, server_buff.size) == 0);
    puts("SUCCESS");

    clear_wait_buffer(&server_buff);
  }

  puts("Transmit datagram from server to clients...");
  const char *atad = "!olleH";
  tweak_wire_transmit(server_context, (const uint8_t*)atad, strlen(atad));

  for(size_t i = 0U; i < TEST_CLIENTS; i++) {
    wait_buffer(&client_buff[i]);

    TEST_CHECK(strncmp(atad, (const char *)client_buff[i].buffer, client_buff[i].size) == 0);
    printf("Client %zu received\n", i);

    clear_wait_buffer(&client_buff[i]);
  }

  for(size_t i = 0U; i < TEST_CLIENTS; i++) {
    printf("Shut down client %zu...\n", i);
    tweak_wire_destroy_connection(client_context[i]);
    puts("SUCCESS");
  }
  tweak_common_sleep(1000);

  bool check;
  tweak_common_mutex_lock(&s_lock);
  server_conn_state = s_server_conn_state;
  check = cb_cnt + 1 == server_conn_cb_cnt;
  tweak_common_mutex_unlock(&s_lock);

  puts("Shut down master node...");
  TEST_CHECK(server_conn_state == TWEAK_WIRE_DISCONNECTED);
  TEST_CHECK(check);
  tweak_wire_destroy_connection(server_context);
  puts("SUCCESS");

  tweak_common_mutex_destroy(&server_buff.lock);
  tweak_common_cond_destroy(&server_buff.cond);

  for(size_t i = 0U; i < TEST_CLIENTS; i++) {
    tweak_common_mutex_destroy(&client_buff[i].lock);
    tweak_common_cond_destroy(&client_buff[i].cond);
  }
  finalize();
}

TEST_LIST = {
   { "test-wire", test_wire },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};
