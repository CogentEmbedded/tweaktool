/**
 * @file tweaklog_format_time_winapi.c
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
#include <stdio.h>
#include <inttypes.h>
#include <windows.h>

enum { TWEAK_COMMON_LOG_POSIX_TIMESTAMP_SIZE = 128 };

 __declspec(thread) static char s_thread_local_buff[TWEAK_COMMON_LOG_POSIX_TIMESTAMP_SIZE];

const char* tweak_common_log_format_time(void) {
  FILETIME ft = { 0 };
  GetSystemTimePreciseAsFileTime(&ft);
  SYSTEMTIME st = { 0 };
  FileTimeToSystemTime(&ft, &st);
  snprintf(s_thread_local_buff, sizeof(s_thread_local_buff),
    "%04d-%02d-%02dT%02d:%02d:%02d.%07dZ",
    st.wYear, st.wMonth, st.wDay,
    st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
  return s_thread_local_buff;
}
