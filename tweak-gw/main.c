/*******************************************************************************
 * main.c
 *
 * Gateway application for Cogent Tweak Tool.
 *
 * Copyright (c) 2020-2021 Cogent Embedded Inc.
 * ALL RIGHTS RESERVED.
 *
 * The source code contained or described herein and all documents related to the
 * source code("Software") or their modified versions are owned by
 * Cogent Embedded Inc. or its affiliates.
 *
 * No part of the Software may be used, copied, reproduced, modified, published,
 * uploaded, posted, transmitted, distributed, or disclosed in any way without
 * prior express written permission from Cogent Embedded Inc.
 *
 * Cogent Embedded Inc. grants a nonexclusive, non-transferable, royalty-free
 * license to use the Software to Licensee without the right to sublicense.
 * Licensee agrees not to distribute the Software to any third-party without
 * the prior written permission of Cogent Embedded Inc.
 *
 * Unless otherwise agreed by Cogent Embedded Inc. in writing, you may not remove
 * or alter this notice or any other notice embedded in Software in any way.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *******************************************************************************/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <TI/tivx.h>

#include <app_init.h>
#include <utils/console_io/include/app_log.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/mem/include/app_mem.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/remote_service/include/app_remote_service.h>

#include <tweak2/log.h>
#include <tweak2/tweak2.h>
#include <tweak2/wire.h>

/*******************************************************************************
 * Standard routines for TI OpenVX app.
 ******************************************************************************/

struct tweak_gw_context
{
    pthread_mutex_t lock;
    tweak_wire_connection rpmsg_connection;
    tweak_wire_connection nng_connection;
};

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
    pthread_mutex_lock(&context->lock);
    switch (connection_state)
    {
    case TWEAK_WIRE_CONNECTED:
        assert(context->rpmsg_connection == TWEAK_WIRE_INVALID_CONNECTION);
        context->rpmsg_connection = tweak_wire_create_connection("rpmsg", "", "rpmsg://17",
                                                                 rpmsg_connection_state_listener, context,
                                                                 rpmsg_to_nng_receive_listener, context);
        if (context->rpmsg_connection == TWEAK_WIRE_INVALID_CONNECTION)
        {
            TWEAK_FATAL("Can't respawn RPMSG connection");
        }
        break;
    case TWEAK_WIRE_DISCONNECTED:
    {
        tweak_wire_destroy_connection(context->rpmsg_connection);
        context->rpmsg_connection = TWEAK_WIRE_INVALID_CONNECTION;
        TWEAK_LOG_DEBUG("NNG upstream state: disconnected, RPMSG downstream: disconnected");
    }
    break;
    default:
        abort();
        break;
    }
    pthread_mutex_unlock(&context->lock);
}

static void rpmsg_to_nng_receive_listener(const uint8_t *buffer, size_t size, void *cookie)
{
    TWEAK_LOG_TRACE_ENTRY("buffer=%p, size=%zu, cookie=%p", buffer, size, cookie);
    struct tweak_gw_context *context = (struct tweak_gw_context *)cookie;
    assert(context);
    pthread_mutex_lock(&context->lock);
    if (context->nng_connection != TWEAK_WIRE_INVALID_CONNECTION)
    {
        TWEAK_LOG_TRACE("Forwarding %zu bytes to %p", size, context->nng_connection);
        tweak_wire_error_code error_code = tweak_wire_transmit(context->nng_connection, buffer, size);
        if (error_code != TWEAK_WIRE_SUCCESS)
        {
            TWEAK_LOG_ERROR("Tweak wire transmit error %d", error_code);
        }
    }
    else
    {
        TWEAK_LOG_WARN("Dropping %zu bytes to downstream server");
    }
    pthread_mutex_unlock(&context->lock);
}

static void nng_to_rpmsg_receive_listener(const uint8_t *buffer, size_t size, void *cookie)
{
    TWEAK_LOG_TRACE_ENTRY("buffer=%p, size=%zu, cookie=%p", buffer, size, cookie);
    struct tweak_gw_context *context = (struct tweak_gw_context *)cookie;
    assert(context);
    pthread_mutex_lock(&context->lock);
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
    pthread_mutex_unlock(&context->lock);
}

static void destroy_tweak_gw_context(struct tweak_gw_context *context)
{
    pthread_mutex_lock(&context->lock);
    tweak_wire_destroy_connection(context->nng_connection);
    tweak_wire_destroy_connection(context->rpmsg_connection);
    pthread_mutex_unlock(&context->lock);
    pthread_mutex_destroy(&context->lock);
}

/*******************************************************************************
 * Entry point
 ******************************************************************************/

int main()
{
    int sfd;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGHUP);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
    {
        TWEAK_FATAL("sigprocmask() failed");
        return 1;
    }

    sfd = signalfd(-1, &mask, 0);
    if (sfd < 0)
    {
        TWEAK_FATAL("signalfd() failed");
        return 1;
    }

    int status = app_init();
    if (status != 0)
    {
        TWEAK_LOG_ERROR("Failed to start application: %d", status);
        return -1;
    }

    struct tweak_gw_context context = {0};
    pthread_mutex_init(&context.lock, NULL);
    context.nng_connection = tweak_wire_create_connection("nng",
                                                          "role=server", "tcp://0.0.0.0:7777",
                                                          nng_connection_state_listener, &context,
                                                          nng_to_rpmsg_receive_listener, &context);

    if (context.nng_connection == TWEAK_WIRE_INVALID_CONNECTION)
    {
        TWEAK_LOG_ERROR("Cannot create nng connection");
        return 1;
    }

    context.rpmsg_connection = TWEAK_WIRE_INVALID_CONNECTION;

    struct signalfd_siginfo si = {0};
    ssize_t res = read(sfd, &si, sizeof(si));
    if (res == sizeof(si))
    {
        TWEAK_LOG_TRACE("Got signal %d", si.ssi_signo);
    }
    else
    {
        TWEAK_LOG_ERROR("Error reading from signalfd: %m");
    }

    destroy_tweak_gw_context(&context);

    close(sfd);
    app_deinit();

    return 0;
}
