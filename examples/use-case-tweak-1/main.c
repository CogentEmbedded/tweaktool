/**
 * @file main.c
 * @ingroup tweak-examples
 *
 * @brief Clean and nice simple example.
 *
 * @copyright 2020-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#define _GNU_SOURCE
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <tweak2/tweak2.h>
#include <unistd.h>

/**
 * @brief Print log_message message with newline in the end.
 * @details Arguments are same as in @ref printf.
 */
void __attribute__((format(printf, 1, 2))) log_message(const char *format, ...)
{
    va_list(args);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    if (format[strlen(format) - 1] != '\n')
    {
        /*.. Print newline character if needed */
        fprintf(stderr, "\n");
    }
}

/**
 * @brief File descriptor to request application termination.
 */
volatile int terminate_fd = false;

/**
 * @brief Handler of SIGINT.
 */
void sigint_handler(int dummy)
{
    (void)dummy;
    write(terminate_fd, "\001", 1);
}

/**
 * @brief Important job.
 * @details This work package does a very important job:
 *   1. Prints "hello" every 3 seconds
 *   2. Calculates y = sin(\omega * t)
 *   3. Prints y if t % 10 = 0
 *   Where t is an integer cycle count.
 * @param fd File descriptor to watch for.
 */
void important_work(int fd)
{
    const char *meta_bool = "{"
                            "\"type\": \"bool\","
                            "\"control\": \"checkbox\""
                            "}";


    const char *meta_coefficient = "{"
                                   "\"control\": \"slider\","
                                   "\"min\": -100,"
                                   "\"max\": 100,"
                                   "\"readonly\": false,"
                                   "\"decimals\": 4"
                                   "}";

    const char *meta_readonly = "{"
                                "\"readonly\": true"
                                "}";

    tweak_id enable_tid = tweak_add_scalar_bool("/enable", "Argument A of quadratic equation", meta_bool, true);
    tweak_id a_tid = tweak_add_scalar_float("/coefficients/a", "Argument A of quadratic equation", meta_coefficient, 0.f);
    tweak_id b_tid = tweak_add_scalar_float("/coefficients/b", "Argument B of quadratic equation", meta_coefficient, 0.f);
    tweak_id c_tid = tweak_add_scalar_float("/coefficients/c", "Argument C of quadratic equation", meta_coefficient, 0.f);
    tweak_id x1_tid = tweak_add_scalar_float("/roots/x1", "Root x1 of quadratic equation", meta_readonly, 0.f);
    tweak_id x2_tid = tweak_add_scalar_float("/roots/x2", "Root x2 of quadratic equation", meta_readonly, 0.f);

    bool terminate = false;

    /*.. main loop */
    while (!terminate)
    {
        /*.. solve quadratic equation */
        bool enable = tweak_get_scalar_bool(enable_tid);
        float x1 = NAN;
        float x2 = NAN;

        if (enable)
        {
            float a = tweak_get_scalar_float(a_tid);
            float b = tweak_get_scalar_float(b_tid);
            float c = tweak_get_scalar_float(c_tid);

            float d = b * b - 4.f * a * c;

            x1 = (-b + sqrtf(d)) / 2. * a;
            x2 = (-b - sqrtf(d)) / 2. * a;
        }

        tweak_set_scalar_float(x1_tid, x1);
        tweak_set_scalar_float(x2_tid, x2);

        /*.. Poll for new event or timeout */
        struct pollfd fds[1] = {0};
        const int timeout = 1000;
        fds[0].fd = fd;
        fds[0].events = 0;
        fds[0].events = fds[0].events | POLLIN;
        int ret = poll(fds, 1, timeout);

        if (ret > 0)
        {
            terminate = true;
        }
    }
}

int main()
{
    /*.. Initialize with default parameters */
    tweak_initialize_library("nng", "role=server", "tcp://0.0.0.0:7777/");

    /*.. Setup termination signals */
    int fd[2];
    if (pipe(fd) != 0)
    {
        perror("pipe2()");
        return 1;
    };
    terminate_fd = fd[1];
    signal(SIGINT, sigint_handler);

    /*.. Print application banner */
    log_message("Hello and welcome");

    /*.. Start working */
    /*.. TODO: Remove hard-code */
    important_work(fd[0]);

    /*.. Terminate gracefully */
    return 0;
}