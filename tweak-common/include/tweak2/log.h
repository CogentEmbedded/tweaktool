/**
 * @file log.h
 * @ingroup tweak-api
 *
 * @brief Common logging routine.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_LOG_H_INCLUDED
#define TWEAK_LOG_H_INCLUDED

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

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
  TWEAK_LOG_LEVEL_DEBUG,
  /**
   * @brief Test messages.
   *
   * Severity is identical to debug level, but isn't disabled for release builds.
   * For use in test scenarios only.
   */
  TWEAK_LOG_LEVEL_TEST,
  /**
   * @brief Invalid configuration.
   *
   * Recovery is possible, but user of the library is supposed to fix this condition.
   * Some library features could be unavailable or not function as expected.
   */
  TWEAK_LOG_LEVEL_WARN,
  /**
   * @brief Expected undesirable situation such as network packet being lost.
   *
   * Recovery is still possible.
   */
  TWEAK_LOG_LEVEL_ERROR,
  /**
   * @brief Unrecoverable condition in the software, most likely a critical bug.
   *
   * Message with this level of severity causes abnormal program termination.
   */
  TWEAK_LOG_LEVEL_FATAL
} tweak_log_level;

/**
 * @brief Redirect log output to given file.
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
 * @param file file to collect log records.
 */
void tweak_common_set_output_file(FILE* file);

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
 * @brief Get unique thread name for logging.
 *
 * @return Unique thread name.
 */
const char* tweak_common_get_current_thread_name();

/**
 * @brief Prints chunk of binary data to logging backend.
 *
 * @param log_level minimal log level for which this hexdump is relevant.
 * @param message message printed above dump of given binary data.
 * @param buffer memory buffer to log.
 * @param length size of the @p buffer.
 */
void tweak_common_log_hexdump(tweak_log_level log_level, const char* message, uint8_t* buffer, uint32_t length);

#if TWEAK_LOG_LEVEL <= 0
#define TWEAK_LOG_TRACE(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_TRACE, "Trace %s : %s@%d | "FORMAT, \
  tweak_common_get_current_thread_name(), __func__, __LINE__, ##__VA_ARGS__)
#else
#define TWEAK_LOG_TRACE(...)
#endif

#define TWEAK_LOG_TRACE_ENTRY(FORMAT, ...) \
  TWEAK_LOG_TRACE("Enter "FORMAT, ##__VA_ARGS__)

#if TWEAK_LOG_LEVEL <= 0
#define TWEAK_LOG_TRACE_HEXDUMP(MESSAGE, BUFFER, LENGTH) \
  tweak_common_log_hexdump(TWEAK_LOG_LEVEL_TRACE, MESSAGE, BUFFER, LENGTH);
#else
#define TWEAK_LOG_TRACE_HEXDUMP(...)
#endif

#if TWEAK_LOG_LEVEL <= 1
#define TWEAK_LOG_DEBUG(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_DEBUG, "Debug %s : %s@%d | "FORMAT, \
  tweak_common_get_current_thread_name(), __func__, __LINE__, ##__VA_ARGS__)
#else
#define TWEAK_LOG_DEBUG(...)
#endif

#if TWEAK_LOG_LEVEL <= 2
#define TWEAK_LOG_TEST(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_TEST, "Test %s : %s@%d | "FORMAT, \
  tweak_common_get_current_thread_name(), __func__, __LINE__, ##__VA_ARGS__)
#else
#define TWEAK_LOG_TEST(...)
#endif

#if TWEAK_LOG_LEVEL <= 3
#define TWEAK_LOG_WARN(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_WARN, "Warning %s : %s@%d | "FORMAT, \
  tweak_common_get_current_thread_name(), __func__, __LINE__, ##__VA_ARGS__)
#else
#define TWEAK_LOG_WARN(...)
#endif

#if TWEAK_LOG_LEVEL <= 4
#define TWEAK_LOG_ERROR(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_ERROR, "Error %s : %s@%d | "FORMAT, \
  tweak_common_get_current_thread_name(), __func__, __LINE__, ##__VA_ARGS__)
#else
#define TWEAK_LOG_ERROR(...)
#endif

#if TWEAK_LOG_LEVEL <= 5
#define TWEAK_FATAL(FORMAT, ...) \
  tweak_common_log(TWEAK_LOG_LEVEL_FATAL, "Fatal %s : %s@%d | "FORMAT, \
  tweak_common_get_current_thread_name(), __func__, __LINE__, ##__VA_ARGS__)
#else
#define TWEAK_FATAL(...)
#endif

#ifdef __cplusplus
}
#endif

#endif
