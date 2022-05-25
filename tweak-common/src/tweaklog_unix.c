/**
 * @file tweaklog_unix.c
 * @ingroup tweak-api
 *
 * @brief Common logging routine, reference POSIX implementation.
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
#include <tweak2/thread.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_LENGTH_PTHREAD_NAME (16)

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS (16)
#endif

static tweak_common_mutex s_log_lock = {0};

static tweak_log_level s_log_level;

static tweak_log_output_proc s_log_output_proc = NULL;

static char s_string_concat_buffer[TWEAK_MAX_LOG_ENTRY_STRING_LENGTH];

static void tweak_common_log_destroy(void);

enum { MAX_TRACE_CHUNK_SIZE = 128 };

static void tweak_common_log_init(void) {
  tweak_common_mutex_init(&s_log_lock);
  s_log_level = (tweak_log_level)TWEAK_LOG_LEVEL;
  s_log_output_proc = tweak_common_stderr_log_handler;
  atexit(&tweak_common_log_destroy);
}

static void tweak_common_log_destroy(void) {
  tweak_common_mutex_destroy(&s_log_lock);
}

static volatile bool s_initialized = false;

static void check_initialization(void) {
  if (!s_initialized) {
    tweak_common_log_init();
    s_initialized = true;
  }
}

void tweak_common_set_custom_handler(tweak_log_output_proc log_output_proc) {
  check_initialization();
  tweak_common_mutex_lock(&s_log_lock);
  s_log_output_proc = log_output_proc;
  tweak_common_mutex_unlock(&s_log_lock);
}

void tweak_common_log_hexdump(tweak_log_level log_level, const char* message, uint8_t* buffer, uint32_t length) {
  check_initialization();
  if (s_log_level > log_level)
    return;

  tweak_common_mutex_lock(&s_log_lock);
  uint32_t i, j;
  s_log_output_proc(message);
  bool truncate = false;
  uint32_t cut_length = length;
  if (length > MAX_TRACE_CHUNK_SIZE) {
    cut_length = MAX_TRACE_CHUNK_SIZE;
    truncate = true;
  }
  uint32_t upper_bound = cut_length + ((cut_length % HEXDUMP_COLS) ? (HEXDUMP_COLS - cut_length % HEXDUMP_COLS) : 0);
  for (i = 0; i < upper_bound; i++) {
    /* print offset */
    if(i % HEXDUMP_COLS == 0) {
      char buff[64];
      sprintf(buff, "0x%06x: ", i);
      strcpy(s_string_concat_buffer, buff);
    }
    /* print hex data */
    if (i < cut_length) {
      char buff[64];
      sprintf(buff, "%02x ", buffer[i]);
      strcat(s_string_concat_buffer, buff);
    } else  { /* end of block, just aligning for ASCII dump */
      strcat(s_string_concat_buffer, "   ");
    }

    /* print ASCII dump */
    if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
      for (j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
        if (j >= cut_length) {  /* end of block, not really printing */
          strcat(s_string_concat_buffer, " ");
        } else if(isprint(buffer[j])) {  /* printable char */
          char buff[2] = { 0 };
          buff[0] = buffer[j];
          strcat(s_string_concat_buffer, buff);
        } else {  /* other char */
          char buff[2] = { 0 };
          buff[0] = '.';
          strcat(s_string_concat_buffer, buff);
        }
      }
      s_log_output_proc(s_string_concat_buffer);
    }
  }
  if (truncate) {
    char buff[128];
    sprintf(buff, "... Truncated %u bytes ...", length - cut_length);
    s_log_output_proc(buff);
  }
  tweak_common_mutex_unlock(&s_log_lock);
}

void tweak_common_set_log_level(tweak_log_level log_level) {
  check_initialization();
  if (log_level <= TWEAK_LOG_LEVEL_FATAL) {
    s_log_level = log_level;
  } else {
    s_log_level = TWEAK_LOG_LEVEL_FATAL;
  }
}

void tweak_common_log(tweak_log_level log_level, const char *format, ...)
{
  check_initialization();

  if (s_log_level <= log_level) {
    tweak_common_mutex_lock(&s_log_lock);
    va_list args;
    va_start (args, format);
    int nchars = vsnprintf(s_string_concat_buffer, sizeof(s_string_concat_buffer), format, args);
    va_end(args);
    if (nchars > 0) {
      if ((unsigned)nchars >= sizeof(s_string_concat_buffer)) {
        s_string_concat_buffer[sizeof(s_string_concat_buffer) - 4] = '.';
        s_string_concat_buffer[sizeof(s_string_concat_buffer) - 3] = '.';
        s_string_concat_buffer[sizeof(s_string_concat_buffer) - 2] = '.';
        s_string_concat_buffer[sizeof(s_string_concat_buffer) - 1] = '\0';
      }
      s_log_output_proc(s_string_concat_buffer);
    }
    tweak_common_mutex_unlock(&s_log_lock);
  }

  if (log_level == TWEAK_LOG_LEVEL_FATAL) {
    abort();
  }
}
