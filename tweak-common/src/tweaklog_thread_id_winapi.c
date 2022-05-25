/**
 * @file tweaklog_thread_id_winapi.c
 * @ingroup tweak-api
 *
 * @brief Routine to get printable thread id for logging.
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
#include <windows.h>

enum { TWEAK_COMMON_LOG_LINUX_THREAD_ID_SIZE = 32 };

__declspec(thread) static char s_thread_local_buff[TWEAK_COMMON_LOG_LINUX_THREAD_ID_SIZE];

uint64_t tweak_common_log_get_tid(void) {
    return GetCurrentThreadId();
}

const char *tweak_common_log_get_thread_id(void) {
  (void) snprintf(s_thread_local_buff, TWEAK_COMMON_LOG_LINUX_THREAD_ID_SIZE, "tid=%" PRId64, tweak_common_log_get_tid());
  return s_thread_local_buff;
}
