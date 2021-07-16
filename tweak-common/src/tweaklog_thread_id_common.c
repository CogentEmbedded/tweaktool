/**
 * @file tweaklog_thread_id_common.c
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
#include <pthread.h>
#include <stdio.h>
#include <inttypes.h>

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

enum { TWEAK_COMMON_LOG_LINUX_THREAD_ID_SIZE = 32 };

static void destroy_thread_id(void* ptr) {
    free(ptr);
}

static void make_key() {
    (void) pthread_key_create(&key, &destroy_thread_id);
}

const char* tweak_common_log_get_thread_id() {
  char *ptr;
  (void) pthread_once(&key_once, make_key);
  if ((ptr = pthread_getspecific(key)) == NULL) {
    ptr = calloc(1, TWEAK_COMMON_LOG_LINUX_THREAD_ID_SIZE);
    if (ptr == NULL) {
        abort();
    }
    uint64_t tid = tweak_common_log_get_tid();
    (void) snprintf(ptr, TWEAK_COMMON_LOG_LINUX_THREAD_ID_SIZE, "tid=%" PRId64, tid);
    (void) pthread_setspecific(key, ptr);
  }
  return ptr;
}
