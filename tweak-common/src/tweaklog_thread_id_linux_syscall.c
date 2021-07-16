/**
 * @file tweaklog_thread_id_linux_syscall.c
 * @ingroup tweak-api
 *
 * @brief Routine to get printable thread id for logging.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

uint64_t tweak_common_log_get_tid() {
    return syscall(__NR_gettid);
}
