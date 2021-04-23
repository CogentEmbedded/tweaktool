/**
 * @file tweaklog.c
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

#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>

#define MAX_LENGTH_PTHREAD_NAME (16)

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS (16)
#endif

static __thread bool s_thread_name_initialized = false;

static __thread char s_thread_name[MAX_LENGTH_PTHREAD_NAME];

static pthread_mutex_t s_log_lock = { 0 };

static tweak_log_level s_log_level;

static FILE* s_log_file = NULL;

static void tweak_common_log_destroy();

enum { MAX_TRACE_CHUNK_SIZE = 128 };

static void tweak_common_log_init() {
  pthread_mutex_init(&s_log_lock, NULL);
  s_log_level = TWEAK_LOG_LEVEL;
  s_log_file = stderr;
  atexit(&tweak_common_log_destroy);
}

static void tweak_common_log_destroy() {
  pthread_mutex_destroy(&s_log_lock);
}

static pthread_once_t tweak_common_log_is_initialized = PTHREAD_ONCE_INIT;

static void check_initialization() {
  (void) pthread_once(&tweak_common_log_is_initialized, &tweak_common_log_init);
}

void tweak_common_set_output_file(FILE* file) {
  check_initialization();
  pthread_mutex_lock(&s_log_lock);
  s_log_file = file;
  pthread_mutex_unlock(&s_log_lock);
}

void tweak_common_log_hexdump(tweak_log_level log_level, const char* message, uint8_t* buffer, uint32_t length) {
  check_initialization();
  if (s_log_level > log_level)
    return;

  pthread_mutex_lock(&s_log_lock);
  uint32_t i, j;
  fputs(message, s_log_file);
  fputc('\n', s_log_file);
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
      fprintf(s_log_file, "0x%06x: ", i);
    }
    /* print hex data */
    if (i < cut_length) {
      fprintf(s_log_file, "%02x ", buffer[i]);
    } else  { /* end of block, just aligning for ASCII dump */
      fprintf(s_log_file, "   ");
    }

    /* print ASCII dump */
    if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
      for (j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
        if (j >= cut_length) {  /* end of block, not really printing */
          fputc (' ', s_log_file);
        } else if(isprint(buffer[j])) {  /* printable char */
          fputc (buffer[j], s_log_file);
        } else {  /* other char */
          fputc ('.', s_log_file);
        }
      }
      fputc ('\n', s_log_file);
    }
  }
  if (truncate) {
    fprintf(s_log_file, "... Truncated %u bytes ... \n", length - cut_length);
  }
  pthread_mutex_unlock(&s_log_lock);
}

void tweak_common_set_log_level(tweak_log_level log_level) {
  check_initialization();
  if (log_level <= TWEAK_LOG_LEVEL_FATAL) {
    s_log_level = log_level;
  } else {
    s_log_level = TWEAK_LOG_LEVEL_FATAL;
  }
}

void tweak_common_log(tweak_log_level log_level, const char * format, ... ) {
  check_initialization();

  if (s_log_level <= log_level) {
    pthread_mutex_lock(&s_log_lock);
    va_list args;
    va_start (args, format);
    vfprintf(s_log_file, format, args);
    va_end (args);
    fprintf(s_log_file, "\n");
    pthread_mutex_unlock(&s_log_lock);
  }

  if (log_level == TWEAK_LOG_LEVEL_FATAL) {
    abort();
  }
}

const char* tweak_common_get_current_thread_name() {
  if (!s_thread_name_initialized) {
    snprintf(s_thread_name, sizeof(s_thread_name), "%#" PRIx64 "", pthread_self());
    s_thread_name_initialized = true;
  }
  return s_thread_name;
}
