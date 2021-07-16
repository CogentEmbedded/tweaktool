/**
 * @file tweaklog_out_stderr.c
 * @ingroup tweak-api
 *
 * @brief Common logging routine, reference POSIX implementation.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include <TI/j7.h>
#include <TI/tivx.h>
#include <tivx_platform.h>

void tweak_common_stderr_log_handler(const char* string) {
    char buffer[1024];
    int nchars = snprintf(buffer, sizeof(buffer), "tweak: %s\n", string);
    if (nchars >= sizeof(buffer))
    {
        buffer[sizeof(buffer) - 5] = '.';
        buffer[sizeof(buffer) - 4] = '.';
        buffer[sizeof(buffer) - 3] = '.';
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
    }
    buffer[sizeof(buffer) - 1] = '\0';
    tivxPlatformPrintf(buffer);
}
