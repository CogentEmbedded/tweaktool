/**
 * @file main.c
 * @ingroup tweak-examples
 * 
 * @brief Clean and nice simple example.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <tweak.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <poll.h>

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
};

/**
 * @brief File descriptor to request application termination.
 */
volatile int terminate_fd = false;

/**
 * @brief Handler of SIGINT.
 */
void sigint_handler(int dummy)
{
    write(terminate_fd, "\001", 1);
}

// tweak_model_t model[] = {
//     {"my_controls", TWEAK_START_GROUP},
//     {"omega", TWEAK_FLOAT, TWEAK_BIND_DIRECTLY, &omega},
//     {"beta", TWEAK_UINT},
//     {"gamma", TWEAK_VEC3_FLOAT, TWEAK_BIND_UNLOCKED, &getter, &setter},
//     {"Z", TWEAK_DOUBLE, TWEAK_BIND_AUTOLOCK, &getter2, &setter2},
//     {NULL}
// };

// if (!tweak_update_model(&model))
// {
//     abort();
// };
// tweak_add_slider_full("omega", -100, 100, omega, 3, getter, setter, &omega);


/**
 * @brief Important job.
 * @details This work package does a very important job:
 *   1. Prints "hello" every 3 seconds
 *   2. Calculates y = sin(\omega * t)
 *   3. Prints y if t % 10 = 0
 *   Where t is an integer cycle count.
 * @param fd File descriptor to watch for.
 * @param omega Wave frequency.
 */
void important_work(int fd, float omega)
{
    /*.. Cycle count. It roll-overs. */
    uint64_t t = 0;

    /*.. Tune omega */
    tweak_add_slider("omega", -100, 100, omega, 3);

    bool terminate = false;

    /*.. main loop */
    while (!terminate)
    {
        /*.. Tune omega */
        double double_omega;
        tweak_get("omega", &double_omega);
        omega = double_omega;

        /*.. Important calculations */
        float y = sinf(omega * t);

        /*.. Generate the output */
        log_message("Iteration %12lu: %12.6f", t, y);

        /*.. Increase cycle count */
        t++;

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

    log_message("We have been interrupted after %llu iterations", t);
}

int main()
{
    /*.. Initialize with default parameters */
    tweak_init_library("0.0.0.0", 7777);

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
    important_work(fd[0], 0.1);

    /*.. Terminate gracefully */
    return 0;
}