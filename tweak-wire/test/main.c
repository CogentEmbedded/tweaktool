/**
 * @file main.c
 * @ingroup tweak-internal-test
 *
 * @brief test suite for tweak wire library.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-internal-test tests for internal tweak2 components.
 */

#include <tweak2/wire.h>

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acutest.h>

#define DEFAULT_ENDPOINT "tcp://0.0.0.0:8888/"

static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_cond = PTHREAD_COND_INITIALIZER;
static tweak_wire_connection_state s_server_conn_state = TWEAK_WIRE_DISCONNECTED;
static tweak_wire_connection_state s_client_conn_state = TWEAK_WIRE_DISCONNECTED;

struct receive_buff {
  pthread_mutex_t lock;
  pthread_cond_t cond;
  bool has_value;
  uint8_t* buffer;
  size_t size;
};

static void test_receive_listener(const uint8_t* buffer, size_t size, void * cookie) {
  struct receive_buff* q = cookie;
  pthread_mutex_lock(&q->lock);
  q->has_value = true;
  q->buffer = calloc(1, size);
  memcpy(q->buffer, buffer, size);
  q->size = size;
  pthread_cond_signal(&q->cond);
  pthread_mutex_unlock(&q->lock);
}

static void wait_buffer(struct receive_buff* q) {
  pthread_mutex_lock(&q->lock);
  while (!q->has_value) {
    pthread_cond_wait(&q->cond, &q->lock);
  }
  pthread_mutex_unlock(&q->lock);
}

static void clear_wait_buffer(struct receive_buff* q) {
  pthread_mutex_lock(&q->lock);
  free(q->buffer);
  q->buffer = NULL;
  q->size = 0;
  q->has_value = false;
  pthread_mutex_unlock(&q->lock);
}

static void server_connection_state_listener(tweak_wire_connection connection,
                                             tweak_wire_connection_state conn_state,
                                             void *cookie)
{
  (void) connection;
  (void) cookie;
  pthread_mutex_lock(&s_lock);
  s_server_conn_state = conn_state;
  pthread_cond_broadcast(&s_cond);
  pthread_mutex_unlock(&s_lock);
}

static void server_wait_connection() {
  pthread_mutex_lock(&s_lock);
  while (s_server_conn_state != TWEAK_WIRE_CONNECTED) {
    pthread_cond_wait(&s_cond, &s_lock);
  }
  pthread_mutex_unlock(&s_lock);
}

static void client_connection_state_listener(tweak_wire_connection connection,
                                             tweak_wire_connection_state conn_state,
                                             void *cookie)
{
  (void) connection;
  (void) cookie;
  pthread_mutex_lock(&s_lock);
  s_client_conn_state = conn_state;
  pthread_cond_broadcast(&s_cond);
  pthread_mutex_unlock(&s_lock);
}

static void client_wait_connection() {
  pthread_mutex_lock(&s_lock);
  while (s_client_conn_state != TWEAK_WIRE_CONNECTED) {
    pthread_cond_wait(&s_cond, &s_lock);
  }
  pthread_mutex_unlock(&s_lock);
}

void test_wire() {
  tweak_wire_connection server_context = NULL, client_context = NULL;
  struct receive_buff server_buff = {
    .cond = PTHREAD_COND_INITIALIZER,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .has_value = false
  };
  struct receive_buff client_buff = {
    .cond = PTHREAD_COND_INITIALIZER,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .has_value = false
  };

  tweak_wire_connection_state server_conn_state;
  pthread_mutex_lock(&s_lock);
  server_conn_state = s_server_conn_state;
  pthread_mutex_unlock(&s_lock);

  puts("Create listener node...");
  server_context = tweak_wire_create_connection("nng", "role=server",
    DEFAULT_ENDPOINT, &server_connection_state_listener,
    NULL, &test_receive_listener, &server_buff);

  TEST_CHECK(server_context != TWEAK_WIRE_INVALID_CONNECTION);
  TEST_CHECK(server_conn_state == TWEAK_WIRE_DISCONNECTED);
  puts("SUCCESS");

  puts("Transmit datagram from server to non-existing client...");
  uint8_t lost[] = "Lost!";
  TEST_CHECK(tweak_wire_transmit(server_context, lost, sizeof(lost)) == TWEAK_WIRE_ERROR_TIMEOUT);
  puts("Got timeout error, SUCCESS");

  puts("Create connector node...");
  client_context = tweak_wire_create_connection("nng", "role=client",
    DEFAULT_ENDPOINT, &client_connection_state_listener,
    NULL, &test_receive_listener, &client_buff);
  TEST_CHECK(client_context != TWEAK_WIRE_INVALID_CONNECTION);
  puts("SUCCESS");

  puts("Wait for connection...");
  server_wait_connection();
  client_wait_connection();
  puts("Connection established");

  pthread_mutex_lock(&s_lock);
  server_conn_state = s_server_conn_state;
  pthread_mutex_unlock(&s_lock);

  TEST_CHECK(server_conn_state == TWEAK_WIRE_CONNECTED);

  puts("Transmit datagram from client to server...");
  const char *data= "Hello!";
  tweak_wire_transmit(client_context, (const uint8_t*)data, strlen(data));
  wait_buffer(&server_buff);

  TEST_CHECK(strncmp(data, (const char *)server_buff.buffer, server_buff.size) == 0);
  puts("SUCCESS");

  clear_wait_buffer(&server_buff);

  puts("Transmit datagram from server to client...");
  const char *atad = "!olleH";
  tweak_wire_transmit(server_context, (const uint8_t*)atad, strlen(atad));
  wait_buffer(&client_buff);

  TEST_CHECK(strncmp(atad, (const char *)client_buff.buffer, client_buff.size) == 0);
  puts("SUCCESS");

  clear_wait_buffer(&client_buff);

  puts("Shut down replica node...");
  tweak_wire_destroy_connection(client_context);
  puts("SUCCESS");
  sleep(1);

  pthread_mutex_lock(&s_lock);
  server_conn_state = s_server_conn_state;
  pthread_mutex_unlock(&s_lock);

  puts("Shut down master node...");
  TEST_CHECK(server_conn_state == TWEAK_WIRE_DISCONNECTED);
  tweak_wire_destroy_connection(server_context);
  puts("SUCCESS");
}

TEST_LIST = {
   { "test-wire", test_wire },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};
