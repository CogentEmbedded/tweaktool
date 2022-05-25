/**
 * @file log.h
 * @ingroup tweak-api
 *
 * @brief Common logging routine.
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

/**
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_LOG_H_INCLUDED
#define TWEAK_LOG_H_INCLUDED

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__clang__)
#   if __clang_major__ > 3 || (__clang_major__ == 3  && __clang_minor__ >= 4)
#       if __has_warning("-Wgnu-zero-variadic-macro-arguments")
#           pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#       endif
#   endif
#endif

/**
 * @brief strings shorter than this constant shouldn't be truncated.
 * If string is longer that that, it could be truncated depending on
 * concrete logger backend implementation. Default implementation does truncate.
 */
enum { TWEAK_MAX_LOG_ENTRY_STRING_LENGTH = 1024 };

/**
 * @brief Severity of condition being logged.
 */
typedef enum {
  /**
   * @brief Messages for fine tuning. Defined as no-op at compile stage by default.
   */
  TWEAK_LOG_LEVEL_TRACE = 0,
  /**
   * @brief Debug messages.
   *
   * Defined as no-op at compile stage for release builds by default.
   */
  TWEAK_LOG_LEVEL_DEBUG = 1,
  /**
   * @brief Test messages.
   *
   * Severity is identical to debug level, but isn't disabled for release builds.
   * For use in test scenarios only.
   */
  TWEAK_LOG_LEVEL_TEST = 2,
  /**
   * @brief Invalid configuration.
   *
   * Recovery is possible, but user of the library is supposed to fix this condition.
   * Some library features could be unavailable or not function as expected.
   */
  TWEAK_LOG_LEVEL_WARN = 3,
  /**
   * @brief Expected undesirable situation such as network packet being lost.
   *
   * Recovery is still possible.
   */
  TWEAK_LOG_LEVEL_ERROR = 4,
  /**
   * @brief Unrecoverable condition in the software, most likely a critical bug.
   *
   * Message with this level of severity causes abnormal program termination.
   */
  TWEAK_LOG_LEVEL_FATAL = 5
} tweak_log_level;

/**
 * @brief Prototype of logging routine.
 *
 * @param string Output string. Should not contain trailing endline delimiter.
 */
typedef void (*tweak_log_output_proc)(const char* string);

/**
 * @brief Redirect log output to given handler.
 *
 * @note Independent from tweak library configuration.
 * Could be invoked prior to any call related to tweak library.
 *
 * @note Default logging behavior that is expected to be valid
 * without runtime configuration calls. Use this function
 * when you have to export the log and desired behaviour cannot
 * be achieved by using compile options and/or custom logging
 * backend.
 *
 * @param log_output_proc handler to collect log records.
 * @param cookie arbitrary context.
 */
void tweak_common_set_custom_handler(tweak_log_output_proc log_output_proc);

/**
 * @brief Default output handler. Unless @see tweak_common_set_custom_handler has been
 * invoked with alternate log handler, all log records are handled by this routine.
 * Despite reference implementation sends @p string followed by '\n' char to stderr
 * with @see fputs, this function may be implemented in a different way on a platform
 * other than x86/Linux.
 *
 * @see tweak_log_output_proc
 * @see tweak_common_set_custom_handler
 *
 * @param string Output string. Should not contain trailing endline delimiter.
 */
void tweak_common_stderr_log_handler(const char* string);

/**
 * @brief thread id provider.
 *
 * @see tweak_log_output_proc
 * @see tweak_common_set_custom_handler
 *
 * @return thread id of current thread as string.
 *
 * @cond PRIVATE
 * Platform specific feature. May be unavailable on some platforms.
 * @endcond
 */
const char* tweak_common_log_get_thread_id(void);

/**
 * @brief format time for logger.
 *
 * @return time string in ISO 8601.
 */
const char* tweak_common_log_format_time();

/**
 * @brief Adjusts log granularity at run time.
 *
 * @note Independent from tweak library configuration.
 * Could be invoked prior to any call related to tweak library.
 *
 * @note Default logging behavior that is expected to be valid
 * without runtime configuration calls. Use this function
 * when it isn't the case.
 *
 * @cond PRIVATE
 * Logging might be disabled statically with respective compilation options (TWEAK_COMMON_LOG_LEVEL).
 * tweak_common_set_log_level(TWEAK_LOG_LEVEL_TRACE) won't provide desired result in this case.
 * @endcond
 *
 * @param log_level Desired log level. Messages with lesser severity shall be ignored.
 */
void tweak_common_set_log_level(tweak_log_level log_level);

/**
 * @brief Main entry logging function.
 *
 * @note Shouldn't be called directly. User is expected to use TWEAK_LOG_*
 * macros.
 *
 * User is expected to write his own platform dependent realization of this function
 * for non-POSIX environments where printing stdout and stderr is unavailable
 * or undesired.
 */
void tweak_common_log(tweak_log_level log_level, const char * format, ... );

/**
 * @brief Prints chunk of binary data to logging backend.
 *
 * @param log_level minimal log level for which this hexdump is relevant.
 * @param message message printed above dump of given binary data.
 * @param buffer memory buffer to log.
 * @param length size of the @p buffer.
 */
void tweak_common_log_hexdump(tweak_log_level log_level, const char* message, uint8_t* buffer, uint32_t length);

/**
 * @brief Dummy function to make all variadic arguments used.
 */
static inline void tweak_common_log_unused_arguments(int dummy, ...)
{
    (void)dummy;
}

/**
 * @brief Returns OS-specific thread id for logging.
 *
 * @return OS-specific thread id.
 */
uint64_t tweak_common_log_get_tid(void);

#if defined(_MSC_BUILD)

#if TWEAK_LOG_LEVEL <= 0
#define TWEAK_LOG_TRACE2(FORMAT, ...) tweak_common_log(TWEAK_LOG_LEVEL_TRACE, "TRACE " FORMAT, __VA_ARGS__)
#define TWEAK_LOG_TRACE(...) TWEAK_LOG_TRACE2(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_TRACE(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#define TWEAK_LOG_TRACE_ENTRY2(FORMAT, ...) TWEAK_LOG_TRACE("Enter " FORMAT, __VA_ARGS__)

#define TWEAK_LOG_TRACE_ENTRY(...) TWEAK_LOG_TRACE_ENTRY2(__VA_ARGS__, 0)

#if TWEAK_LOG_LEVEL <= 0
#define TWEAK_LOG_TRACE_HEXDUMP(MESSAGE, BUFFER, LENGTH) tweak_common_log_hexdump(TWEAK_LOG_LEVEL_TRACE, MESSAGE, BUFFER, LENGTH);
#else
#define TWEAK_LOG_TRACE_HEXDUMP(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 1
#define TWEAK_LOG_DEBUG2(FORMAT, ...) tweak_common_log(TWEAK_LOG_LEVEL_DEBUG, "DEBUG " FORMAT, __VA_ARGS__)
#define TWEAK_LOG_DEBUG(...) TWEAK_LOG_DEBUG2(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_DEBUG(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 2
#define TWEAK_LOG_TEST2(FORMAT, ...) tweak_common_log(TWEAK_LOG_LEVEL_TEST, "TEST" FORMAT, __VA_ARGS__)
#define TWEAK_LOG_TEST(...) TWEAK_LOG_TEST2(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_TEST(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 3
#define TWEAK_LOG_WARN2(FORMAT, ...) tweak_common_log(TWEAK_LOG_LEVEL_WARN, "WARN " FORMAT, __VA_ARGS__)
#define TWEAK_LOG_WARN(...) TWEAK_LOG_WARN2(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_WARN(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 4
#define TWEAK_LOG_ERROR2(FORMAT, ...) tweak_common_log(TWEAK_LOG_LEVEL_ERROR, "ERROR " FORMAT, __VA_ARGS__)
#define TWEAK_LOG_ERROR(...) TWEAK_LOG_ERROR2(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_ERROR(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 5
#define TWEAK_FATAL2(FORMAT, ...) tweak_common_log(TWEAK_LOG_LEVEL_FATAL, "FATAL " FORMAT, __VA_ARGS__)
#define TWEAK_FATAL(...) TWEAK_FATAL2(__VA_ARGS__, 0)
#else
#define TWEAK_FATAL(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#else

#if TWEAK_LOG_LEVEL <= 0
#define TWEAK_LOG_TRACE_(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_TRACE, "%s TRACE [%s] [%s@%d] " FORMAT, \
  tweak_common_log_format_time(), tweak_common_log_get_thread_id(), __func__, __LINE__, ##__VA_ARGS__)
#define TWEAK_LOG_TRACE(...) TWEAK_LOG_TRACE_(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_TRACE(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#define TWEAK_LOG_TRACE_ENTRY2(FORMAT, ...) \
  TWEAK_LOG_TRACE("Enter " FORMAT, ##__VA_ARGS__)

#define TWEAK_LOG_TRACE_ENTRY(...) \
  TWEAK_LOG_TRACE_ENTRY2(__VA_ARGS__, 0)

#if TWEAK_LOG_LEVEL <= 0
#define TWEAK_LOG_TRACE_HEXDUMP(MESSAGE, BUFFER, LENGTH) \
  tweak_common_log_hexdump(TWEAK_LOG_LEVEL_TRACE, MESSAGE, BUFFER, LENGTH);
#else
#define TWEAK_LOG_TRACE_HEXDUMP(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 1
#define TWEAK_LOG_DEBUG_(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_DEBUG, "%s DEBUG [%s] [%s@%d] " FORMAT, \
  tweak_common_log_format_time(), tweak_common_log_get_thread_id(), __func__, __LINE__, ##__VA_ARGS__)
#define TWEAK_LOG_DEBUG(...) TWEAK_LOG_DEBUG_(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_DEBUG(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 2
#define TWEAK_LOG_TEST_(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_TEST, "%s TEST [%s] [%s@%d] " FORMAT, \
  tweak_common_log_format_time(), tweak_common_log_get_thread_id(), __func__, __LINE__, ##__VA_ARGS__)
#define TWEAK_LOG_TEST(...) TWEAK_LOG_TEST_(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_TEST(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 3
#define TWEAK_LOG_WARN_(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_WARN, "%s WARN [%s] [%s@%d] " FORMAT, \
  tweak_common_log_format_time(), tweak_common_log_get_thread_id(), __func__, __LINE__, ##__VA_ARGS__)
#define TWEAK_LOG_WARN(...) TWEAK_LOG_WARN_(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_WARN(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 4
#define TWEAK_LOG_ERROR_(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_ERROR, "%s ERROR [%s] [%s@%d] | " FORMAT, \
  tweak_common_log_format_time(), tweak_common_log_get_thread_id(), __func__, __LINE__, ##__VA_ARGS__)
#define TWEAK_LOG_ERROR(...) TWEAK_LOG_ERROR_(__VA_ARGS__, 0)
#else
#define TWEAK_LOG_ERROR(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#if TWEAK_LOG_LEVEL <= 5
#define TWEAK_FATAL_(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_FATAL, "%s FATAL [%s] [%s@%d] | " FORMAT, \
  tweak_common_log_format_time(), tweak_common_log_get_thread_id(), __func__, __LINE__, ##__VA_ARGS__)
#define TWEAK_FATAL(...) TWEAK_FATAL_(__VA_ARGS__, 0)
#else
#define TWEAK_FATAL(...) tweak_common_log_unused_arguments(0, ##__VA_ARGS__)
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
