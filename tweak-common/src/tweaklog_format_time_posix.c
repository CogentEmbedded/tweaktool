/**
 * @file tweaklog_format_time_posix.c
 * @ingroup tweak-api
 *
 * @brief Routine to get printable ISO 8601 time string for logging.
 *
 * @copyright 2020-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#include <tweak2/log.h>
#include <pthread.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/time.h>

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

enum { TWEAK_COMMON_LOG_POSIX_TIMESTAMP_SIZE = 128 };

static void destroy_thread_id(void* ptr) {
    free(ptr);
}

static void make_key() {
    (void) pthread_key_create(&key, &destroy_thread_id);
}

const char* tweak_common_log_format_time() {
  char *ptr;
  (void) pthread_once(&key_once, make_key);
  if ((ptr = pthread_getspecific(key)) == NULL) {
    ptr = calloc(1, TWEAK_COMMON_LOG_POSIX_TIMESTAMP_SIZE);
    if (ptr == NULL) {
        abort();
    }
    (void) pthread_setspecific(key, ptr);
  }
  struct timeval cur_time = {0};
  gettimeofday(&cur_time, NULL);
  int milli = cur_time.tv_usec / 1000;
  char *p = ptr + strftime(ptr, TWEAK_COMMON_LOG_POSIX_TIMESTAMP_SIZE, "%FT%T", gmtime(&cur_time.tv_sec));
  sprintf(p, ".%03dZ", milli);
  return ptr;
}
