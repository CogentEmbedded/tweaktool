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

void tweak_common_stderr_log_handler(const char* string) {
  fputs(string, stderr);
  fputc('\n', stderr);
}
