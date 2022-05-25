/**
 * @file main.c
 *
 * @brief Gateway application for Cogent Tweak Tool.
 *
 * @copyright (c) 2020-2022 Cogent Embedded, Inc.
 * ALL RIGHTS RESERVED.
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

#include <tweak2/defaults.h>
#include <tweak2/log.h>
#include <tweak2/thread.h>
#include <tweak2/wire.h>

#include <TI/tivx.h>

#include <app_init.h>
#include <utils/console_io/include/app_log.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/mem/include/app_mem.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/remote_service/include/app_remote_service.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*******************************************************************************
 * Standard routines for TI OpenVX app.
 ******************************************************************************/

struct tweak_gw_context
{
    tweak_common_mutex lock;
    tweak_wire_connection rpmsg_connection;
    tweak_wire_connection nng_connection;
    char* connection_type;
    char* params;
    char* uri;
    char* rpmsg_uri;
    tweak_wire_connection_state nng_connection_state;
};

const char* get_connection_type(struct tweak_gw_context* context) {
  return context->connection_type ? context->connection_type : "nng";
}

const char* get_params(struct tweak_gw_context* context) {
  return context->params ? context->params : "role=server";
}

const char* get_uri(struct tweak_gw_context* context) {
  return context->uri ? context->uri : TWEAK_DEFAULT_ENDPOINT;
}

const char* get_rpmsg_uri(struct tweak_gw_context* context) {
  return context->rpmsg_uri ? context->rpmsg_uri : "rpmsg://17";
}

static int32_t app_init()
{
    TWEAK_LOG_TRACE_ENTRY();
    int32_t status = appCommonInit();
    TWEAK_LOG_TRACE("appCommonInit returned %d", status);
    return status;
}

/**
 * @brief Application deinitialization
 */
static int32_t app_deinit()
{
    TWEAK_LOG_TRACE_ENTRY();

    appRemoteServiceDeInit();
    appIpcDeInit();
    appCommonDeInit();
    return 0;
}

/*******************************************************************************
 * Tweak Wire interface
 ******************************************************************************/

static void rpmsg_connection_state_listener(
    tweak_wire_connection connection,
    tweak_wire_connection_state connection_state,
    void *cookie)
{
    (void)connection;
    (void)cookie;
    TWEAK_LOG_TRACE_ENTRY("RPMSG Connection state: %s",
                          connection_state == TWEAK_WIRE_CONNECTED ? "connected" : "disconnected");
}

static void rpmsg_to_nng_receive_listener(const uint8_t *buffer, size_t size, void *cookie);

static void nng_connection_state_listener(tweak_wire_connection connection,
                                          tweak_wire_connection_state connection_state, void *cookie)
{
    TWEAK_LOG_TRACE_ENTRY("NNG Connection state: %s",
                          connection_state == TWEAK_WIRE_CONNECTED ? "connected" : "disconnected");
    (void)connection;
    struct tweak_gw_context *context = (struct tweak_gw_context *)cookie;
    assert(context);
    tweak_common_mutex_lock(&context->lock);
    context->nng_connection_state = connection_state;
    tweak_common_mutex_unlock(&context->lock);
}

static void rpmsg_to_nng_receive_listener(const uint8_t *buffer, size_t size, void *cookie)
{
    TWEAK_LOG_TRACE_ENTRY("buffer=%p, size=%zu, cookie=%p", buffer, size, cookie);
    struct tweak_gw_context *context = (struct tweak_gw_context *)cookie;
    assert(context);
    tweak_common_mutex_lock(&context->lock);
    if (context->nng_connection_state == TWEAK_WIRE_CONNECTED)
    {
        TWEAK_LOG_TRACE("Forwarding %zu bytes to %p", size, context->nng_connection);
        tweak_wire_error_code error_code = tweak_wire_transmit(context->nng_connection, buffer, size);
        if (error_code != TWEAK_WIRE_SUCCESS)
        {
            TWEAK_LOG_ERROR("Tweak wire transmit error %d", error_code);
        }
    }
    tweak_common_mutex_unlock(&context->lock);
}

static void nng_to_rpmsg_receive_listener(const uint8_t *buffer, size_t size, void *cookie)
{
    TWEAK_LOG_TRACE_ENTRY("buffer=%p, size=%zu, cookie=%p", buffer, size, cookie);
    struct tweak_gw_context *context = (struct tweak_gw_context *)cookie;
    assert(context);
    tweak_common_mutex_lock(&context->lock);
    if (context->rpmsg_connection != TWEAK_WIRE_INVALID_CONNECTION)
    {
        TWEAK_LOG_TRACE("Forwarding %zu bytes to %p", size, context->rpmsg_connection);
        tweak_wire_error_code error_code = tweak_wire_transmit(context->rpmsg_connection, buffer, size);
        if (error_code != TWEAK_WIRE_SUCCESS)
        {
            TWEAK_LOG_ERROR("Tweak wire transmit error %d", error_code);
        }
    }
    else
    {
        TWEAK_LOG_WARN("Dropping %zu bytes to downstream server");
    }
    tweak_common_mutex_unlock(&context->lock);
}

static void destroy_tweak_gw_context(struct tweak_gw_context *context)
{
    tweak_common_mutex_lock(&context->lock);
    tweak_wire_connection rpmsg_connection = context->rpmsg_connection;
    context->rpmsg_connection = TWEAK_WIRE_INVALID_CONNECTION;
    tweak_wire_destroy_connection(rpmsg_connection);
    tweak_common_mutex_unlock(&context->lock);
    tweak_wire_destroy_connection(context->nng_connection);
    tweak_common_mutex_destroy(&context->lock);
    free(context->connection_type);
    free(context->params);
    free(context->uri);
    free(context->rpmsg_uri);
}

/*******************************************************************************
 * Entry point
 ******************************************************************************/

int main(int argc, char**argv)
{
    int sig;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    sigprocmask( SIG_BLOCK, &set, NULL );

    struct tweak_gw_context context = { 0 };
    int opt;
    while ((opt = getopt(argc, argv, "t:p:u:r:")) != -1) {
      switch (opt) {
      case 't':
        context.connection_type = strdup(optarg);
        break;
      case 'p':
        context.params = strdup(optarg);
        break;
      case 'u':
        context.uri = strdup(optarg);
        break;
      case 'r':
        context.rpmsg_uri = strdup(optarg);
        break;
      default: /* '?' */
        fprintf(stderr, "Usage: %s [-t connection type] [-p params] [-u uri] [-r rpmsg_uri]\n", argv[0]);
        exit(EXIT_FAILURE);
      }
    }

    tweak_common_mutex_init(&context.lock);

    int status = 0;
    status = app_init();
    if (status != 0)
    {
        TWEAK_LOG_ERROR("Failed to start application: %d", status);
        return -1;
    }
    context.rpmsg_connection = tweak_wire_create_connection("rpmsg", "role=client",
                                                             get_rpmsg_uri(&context),
                                                             rpmsg_connection_state_listener, &context,
                                                             rpmsg_to_nng_receive_listener, &context);

    context.nng_connection = tweak_wire_create_connection(get_connection_type(&context),
                                                          get_params(&context), get_uri(&context),
                                                          nng_connection_state_listener, &context,
                                                          nng_to_rpmsg_receive_listener, &context);


    if (context.nng_connection == TWEAK_WIRE_INVALID_CONNECTION)
    {
        TWEAK_LOG_ERROR("Cannot create nng connection");
        return 1;
    }

    if (sigwait(&set, &sig) >= 0) {
        TWEAK_LOG_DEBUG("sigwait returned with sig: %d", sig);
    } else {
        TWEAK_LOG_ERROR("sigwait failed");
    }

    destroy_tweak_gw_context(&context);

    app_deinit();

    return 0;
}
