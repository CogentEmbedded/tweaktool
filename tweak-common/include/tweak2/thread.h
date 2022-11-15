/**
 * @file thread.h
 * @ingroup tweak-api
 *
 * @brief Cross platform wrapper for thread primitives.
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

#ifndef TWEAK_THREAD_H_INCLUDED
#define TWEAK_THREAD_H_INCLUDED

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define TWEAK_COMMON_NSEC_PER_SEC 1000000000

#define TWEAK_COMMON_NANOS_IN_MILLIS 1000000

#define TWEAK_COMMON_NANOS_IN_USEC 1000

typedef enum {
  TWEAK_COMMON_THREAD_SUCCESS = 0,
  TWEAK_COMMON_THREAD_FAILURE,
  TWEAK_COMMON_THREAD_TIMEOUT
} tweak_common_thread_error;

typedef uint64_t tweak_common_milliseconds;

typedef uint64_t tweak_common_nanoseconds;

#define TWEAK_COMMON_TIMESPAN_INFINITE UINT64_MAX
#define TWEAK_DEFAULT_STACK_SIZE (32 * 1024)

typedef void *(*tweak_common_thread_routine)(void *);

#if defined(TI_ARM_R5F)
#include <TI/tivx.h>
#include <TI/tivx_event.h>
#include <TI/tivx_mutex.h>
#include <TI/tivx_task.h>

struct thread_param_thunk {
  tweak_common_thread_routine thread_proc;
  void *client_cookie;
  void *result;
  uint8_t stack[TWEAK_DEFAULT_STACK_SIZE];
};

typedef struct _tweak_common_thread {
  tivx_task task;
  struct thread_param_thunk *thread_param_thunk;
} tweak_common_thread;

typedef tivx_mutex tweak_common_mutex;
typedef tivx_mutex tweak_common_rwlock;
typedef tivx_event tweak_common_cond;
typedef uint64_t tweak_common_timestamp;

static void VX_CALLBACK tiovx_task_entry_point(void *app_var){
    tweak_common_thread *thread = (tweak_common_thread *)app_var;

    thread->thread_param_thunk->result =
        thread->thread_param_thunk->thread_proc(thread->thread_param_thunk->client_cookie);
}

static inline tweak_common_thread_error tweak_common_map_vx_status(vx_status status) {
  switch (status) {
  case VX_SUCCESS:
    return TWEAK_COMMON_THREAD_SUCCESS;
  case TIVX_ERROR_EVENT_TIMEOUT:
    return TWEAK_COMMON_THREAD_TIMEOUT;
  default:
    return TWEAK_COMMON_THREAD_FAILURE;
  }
}

#elif defined(_MSC_BUILD)
#include <windows.h>
#include <process.h>

struct thread_param_thunk {
  tweak_common_thread_routine thread_proc;
  void *client_cookie;
  void *result;
};

typedef struct {
  HANDLE h;
  struct thread_param_thunk *thread_param_thunk;
} tweak_common_thread;

typedef CRITICAL_SECTION tweak_common_mutex;

typedef SRWLOCK tweak_common_rwlock;

typedef CONDITION_VARIABLE tweak_common_cond;

typedef ULONGLONG tweak_common_timestamp; /* Based on GetTickCount64() */
                                          /* which returns number of milliseconds since system boot */

static unsigned __stdcall winapi_thread_entry_point(void *arg) {

  struct thread_param_thunk *thread_param_thunk =
      (struct thread_param_thunk *)arg;
  thread_param_thunk->result =
      thread_param_thunk->thread_proc(thread_param_thunk->client_cookie);
  return 0;
}
#else
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#if defined(__ZEPHYR__)
#include <zephyr/posix/pthread.h>
#include <zephyr/posix/time.h>
#endif

typedef pthread_t tweak_common_thread;

typedef pthread_mutex_t tweak_common_mutex;

typedef pthread_rwlock_t tweak_common_rwlock;

typedef pthread_cond_t tweak_common_cond;

typedef struct timespec tweak_common_timestamp;

static inline tweak_common_thread_error tweak_common_convert_map_posix_error_code(int posix_ec) {
  switch (posix_ec) {
  case 0:
    return TWEAK_COMMON_THREAD_SUCCESS;
  case ETIMEDOUT:
    return TWEAK_COMMON_THREAD_TIMEOUT;
  default:
    return TWEAK_COMMON_THREAD_FAILURE;
  }
}
#endif

static inline tweak_common_thread_error tweak_common_thread_create(tweak_common_thread *thread,
  tweak_common_thread_routine thread_routine, void* cookie)
{
#if defined(TI_ARM_R5F)
  assert(thread != NULL);

  thread->thread_param_thunk =
   (struct thread_param_thunk *)calloc(1, sizeof(*thread->thread_param_thunk));
  if (thread->thread_param_thunk == NULL)
    return TWEAK_COMMON_THREAD_FAILURE;

  thread->thread_param_thunk->thread_proc = thread_routine;
  thread->thread_param_thunk->client_cookie = cookie;

  tivx_task_create_params_t params;
  tivxTaskSetDefaultCreateParams(&params);
  params.stack_ptr = thread->thread_param_thunk->stack;
  params.stack_size = sizeof(thread->thread_param_thunk->stack);
  params.app_var = thread;
  params.task_main = &tiovx_task_entry_point;

  return tweak_common_map_vx_status(tivxTaskCreate(&thread->task, &params));

#elif defined(_MSC_BUILD)
  assert(thread != NULL);
  unsigned tid;

  thread->thread_param_thunk =
    (struct thread_param_thunk *)calloc(1, sizeof(*thread->thread_param_thunk));
  if (thread->thread_param_thunk == NULL)
    return TWEAK_COMMON_THREAD_FAILURE;

  thread->thread_param_thunk->thread_proc = thread_routine;
  thread->thread_param_thunk->client_cookie = cookie;
  thread->h = (HANDLE)_beginthreadex(NULL, 0, &winapi_thread_entry_point,
                                     thread->thread_param_thunk, 0, &tid);

  if (thread->h == INVALID_HANDLE_VALUE) {
    free(thread->thread_param_thunk);
    return TWEAK_COMMON_THREAD_FAILURE;
  }

  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  pthread_attr_t *attr = NULL;

#if defined(__ZEPHYR__)
  pthread_attr_t zephyr_attr;
  attr = &zephyr_attr;

  pthread_attr_init(attr);
  void *stack = malloc(TWEAK_DEFAULT_STACK_SIZE);
  if (stack == NULL)
  {
      TWEAK_FATAL("malloc() returned NULL");
  }
  pthread_attr_setstack(attr, stack, TWEAK_DEFAULT_STACK_SIZE);
#endif

  return tweak_common_convert_map_posix_error_code(pthread_create(thread, attr, thread_routine, cookie));
#endif
}

static inline tweak_common_thread_error tweak_common_thread_join(tweak_common_thread thread,
  void **outval)
{
#if defined(TI_ARM_R5F)
  assert(thread.thread_param_thunk != NULL);
  tweak_common_thread_error ret = tweak_common_map_vx_status(tivxTaskDelete(&thread.task));
  if (outval) {
      *outval = thread.thread_param_thunk->result;
  }
  free(thread.thread_param_thunk);
  return ret;
#elif defined(_MSC_BUILD)
  assert(thread.thread_param_thunk != NULL);
  DWORD wait_result = WaitForSingleObject(thread.h, INFINITE);
  if (wait_result == WAIT_OBJECT_0) {
    if (outval) {
      *outval = thread.thread_param_thunk->result;
    }

    free(thread.thread_param_thunk);
    CloseHandle(thread.h);

    return TWEAK_COMMON_THREAD_SUCCESS;
  } else {
    return TWEAK_COMMON_THREAD_FAILURE;
  }
#else
  return tweak_common_convert_map_posix_error_code(pthread_join(thread, outval));
#endif
}

static inline tweak_common_thread_error tweak_common_mutex_init(tweak_common_mutex* mutex) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexCreate(mutex));
#elif defined(_MSC_BUILD)
  InitializeCriticalSection(mutex);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_mutex_init(mutex, NULL));
#endif
}

static inline tweak_common_thread_error tweak_common_mutex_lock(tweak_common_mutex* mutex) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexLock(*mutex));
#elif defined(_MSC_BUILD)
  EnterCriticalSection(mutex);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_mutex_lock(mutex));
#endif
}

static inline tweak_common_thread_error tweak_common_mutex_unlock(tweak_common_mutex* mutex) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexUnlock(*mutex));
#elif defined(_MSC_BUILD)
  LeaveCriticalSection(mutex);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_mutex_unlock(mutex));
#endif
}

static inline tweak_common_thread_error tweak_common_mutex_destroy(tweak_common_mutex* mutex) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexDelete(mutex));
#elif defined(_MSC_BUILD)
  DeleteCriticalSection(mutex);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_mutex_destroy(mutex));
#endif
}

static inline tweak_common_thread_error tweak_common_rwlock_init(tweak_common_rwlock* rwlock) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexCreate(rwlock));
#elif defined(_MSC_BUILD)
  InitializeSRWLock(rwlock);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_rwlock_init(rwlock, NULL));
#endif
}

static inline tweak_common_thread_error tweak_common_rwlock_read_lock(tweak_common_rwlock* rwlock) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexLock(*rwlock));
#elif defined(_MSC_BUILD)
  AcquireSRWLockShared(rwlock);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_rwlock_rdlock(rwlock));
#endif
}

static inline tweak_common_thread_error tweak_common_rwlock_write_lock(tweak_common_rwlock* rwlock) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexLock(*rwlock));
#elif defined(_MSC_BUILD)
  AcquireSRWLockExclusive(rwlock);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_rwlock_wrlock(rwlock));
#endif
}

static inline tweak_common_thread_error tweak_common_rwlock_read_unlock(tweak_common_rwlock* rwlock) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexUnlock(*rwlock));
#elif defined(_MSC_BUILD)
  ReleaseSRWLockShared(rwlock);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_rwlock_unlock(rwlock));
#endif
}

static inline tweak_common_thread_error tweak_common_rwlock_write_unlock(tweak_common_rwlock* rwlock) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexUnlock(*rwlock));
#elif defined(_MSC_BUILD)
  ReleaseSRWLockExclusive(rwlock);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_rwlock_unlock(rwlock));
#endif
}

static inline tweak_common_thread_error tweak_common_rwlock_destroy(tweak_common_rwlock* rwlock) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxMutexDelete(rwlock));
#elif defined(_MSC_BUILD)
  (void)rwlock;
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_rwlock_destroy(rwlock));
#endif
}

static inline tweak_common_thread_error tweak_common_cond_init(tweak_common_cond* cond) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxEventCreate(cond));
#elif defined(_MSC_BUILD)
  InitializeConditionVariable(cond);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_cond_init(cond, NULL));
#endif
}

static inline tweak_common_thread_error tweak_common_cond_timed_wait(tweak_common_cond* cond,
  tweak_common_mutex* mutex, tweak_common_milliseconds timeout_millis);

static inline tweak_common_thread_error tweak_common_cond_wait(tweak_common_cond* cond, tweak_common_mutex* mutex) {
#if defined(TI_ARM_R5F)
  return tweak_common_cond_timed_wait(cond, mutex, TWEAK_COMMON_TIMESPAN_INFINITE);
#elif defined(_MSC_BUILD)
  return SleepConditionVariableCS(cond, mutex, INFINITE)
              ? TWEAK_COMMON_THREAD_SUCCESS
              : TWEAK_COMMON_THREAD_FAILURE;
#else
  return tweak_common_convert_map_posix_error_code(pthread_cond_wait(cond, mutex));
#endif
}

static inline tweak_common_thread_error tweak_common_cond_broadcast(tweak_common_cond* cond) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxEventPost(*cond));
#elif defined(_MSC_BUILD)
  WakeAllConditionVariable(cond);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_cond_broadcast(cond));
#endif
}

static inline tweak_common_thread_error tweak_common_cond_signal(tweak_common_cond* cond) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxEventPost(*cond));
#elif defined(_MSC_BUILD)
  WakeConditionVariable(cond);
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_cond_signal(cond));
#endif
}

static inline tweak_common_thread_error tweak_common_cond_destroy(tweak_common_cond* cond) {
#if defined(TI_ARM_R5F)
  return tweak_common_map_vx_status(tivxEventDelete(cond));
#elif defined(_MSC_BUILD)
  (void)cond;
  return TWEAK_COMMON_THREAD_SUCCESS;
#else
  return tweak_common_convert_map_posix_error_code(pthread_cond_destroy(cond));
#endif
}

static inline void tweak_common_sleep(uint32_t millis) {
#if defined(TI_ARM_R5F)
  tivxTaskWaitMsecs(millis);
#elif defined(_MSC_BUILD)
  SleepEx(millis, FALSE);
#else
  usleep(millis * 1000);
#endif
}

#if !defined(_MSC_BUILD) && !defined(TI_ARM_R5F)

static inline tweak_common_nanoseconds tweak_common_msec_to_nsec(tweak_common_milliseconds arg)
{
  return arg * TWEAK_COMMON_NANOS_IN_MILLIS;
}

static inline tweak_common_milliseconds tweak_common_nsec_to_msec(tweak_common_nanoseconds arg)
{
  return arg / TWEAK_COMMON_NANOS_IN_MILLIS;
}

static inline void tweak_common_timespec_sub(struct timespec *r,
  const struct timespec *a, const struct timespec *b)
{
  r->tv_sec = a->tv_sec - b->tv_sec;
  r->tv_nsec = a->tv_nsec - b->tv_nsec;
  if (r->tv_nsec < 0) {
    r->tv_sec--;
    r->tv_nsec += TWEAK_COMMON_NSEC_PER_SEC;
  }
}

static inline tweak_common_nanoseconds tweak_common_timespec_to_nsec(const struct timespec *a)
{
  return (tweak_common_nanoseconds)a->tv_sec * TWEAK_COMMON_NSEC_PER_SEC + a->tv_nsec;
}

static inline tweak_common_nanoseconds tweak_common_timespec_sub_to_nsec(const struct timespec *a,
  const struct timespec *b)
{
  struct timespec r;
  tweak_common_timespec_sub(&r, a, b);
  return tweak_common_timespec_to_nsec(&r);
}

static inline tweak_common_milliseconds tweak_common_timespec_sub_to_msec(const struct timespec *a,
  const struct timespec *b)
{
  return tweak_common_nsec_to_msec(tweak_common_timespec_sub_to_nsec(a, b));
}

static inline void tweak_common_timespec_add_nsec(struct timespec *r,
  const struct timespec *a, tweak_common_nanoseconds b)
{
  r->tv_sec = a->tv_sec + (b / TWEAK_COMMON_NSEC_PER_SEC);
  r->tv_nsec = a->tv_nsec + (b % TWEAK_COMMON_NSEC_PER_SEC);

  if (r->tv_nsec >= TWEAK_COMMON_NSEC_PER_SEC) {
    r->tv_sec++;
    r->tv_nsec -= TWEAK_COMMON_NSEC_PER_SEC;
  } else if (r->tv_nsec < 0) {
    r->tv_sec--;
    r->tv_nsec += TWEAK_COMMON_NSEC_PER_SEC;
  }
}

static inline void tweak_common_timespec_add_msec(struct timespec *r,
  const struct timespec *a, tweak_common_milliseconds b)
{
  tweak_common_timespec_add_nsec(r, a, tweak_common_msec_to_nsec(b));
}

#endif

static inline void tweak_common_timestamp_now(tweak_common_timestamp* timestamp) {
  assert(timestamp != NULL);
#if defined(TI_ARM_R5F)
  *timestamp = tivxPlatformGetTimeInUsecs() * TWEAK_COMMON_NANOS_IN_USEC;
#elif defined(_MSC_BUILD)
  LARGE_INTEGER time;
  LARGE_INTEGER frequency;

  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&time);

  *timestamp = time.QuadPart * 1000000000ULL / frequency.QuadPart;
#else
  clock_gettime(CLOCK_MONOTONIC, timestamp);
#endif
}

static inline void tweak_common_timestamp_add_milliseconds(tweak_common_timestamp* result,
  tweak_common_timestamp* timestamp, tweak_common_milliseconds milliseconds)
{
  assert(result != NULL);
  assert(timestamp != NULL);
#if defined(TI_ARM_R5F) || defined(_MSC_BUILD)
  *result = *timestamp + TWEAK_COMMON_NANOS_IN_MILLIS * milliseconds;
#else
  tweak_common_timespec_add_msec(result, timestamp, milliseconds);
#endif
}

static inline tweak_common_nanoseconds tweak_common_timestamp_subtract_timestamps(tweak_common_timestamp* timestamp1,
  tweak_common_timestamp* timestamp2)
{
  assert(timestamp1 != NULL);
  assert(timestamp2 != NULL);
#if defined(TI_ARM_R5F) || defined(_MSC_BUILD)
  if (*timestamp2 > *timestamp1)
    return 0ULL;

  return *timestamp1 - *timestamp2;
#else
  tweak_common_timestamp r;
  tweak_common_timespec_sub(&r, timestamp1, timestamp2);
  return tweak_common_timespec_to_nsec(&r);
#endif
}

static inline tweak_common_thread_error tweak_common_cond_timed_wait(tweak_common_cond* cond,
  tweak_common_mutex* mutex, tweak_common_milliseconds timeout_millis)
{
  assert(cond != NULL);
  assert(mutex != NULL);
#if defined(TI_ARM_R5F)
  tweak_common_thread_error ret = tweak_common_mutex_unlock(mutex);
  if (ret == TWEAK_COMMON_THREAD_SUCCESS)
  {
      ret = tweak_common_map_vx_status(tivxEventWait(*cond, timeout_millis));
      /*.. TODO Review if this logic is correct */
      tweak_common_mutex_lock(mutex);
  }
  return ret;
#elif defined(_MSC_BUILD)
  if (SleepConditionVariableCS(cond, mutex, (DWORD)timeout_millis)) {
    return TWEAK_COMMON_THREAD_SUCCESS;
  } else {
    return GetLastError() == ERROR_TIMEOUT
      ? TWEAK_COMMON_THREAD_TIMEOUT
      : TWEAK_COMMON_THREAD_FAILURE;
  }
#else
  if (timeout_millis == TWEAK_COMMON_TIMESPAN_INFINITE) {
    return tweak_common_convert_map_posix_error_code(pthread_cond_wait(cond, mutex));
  } else {
    tweak_common_timestamp now;
    tweak_common_timestamp wait_until;

    tweak_common_timestamp_now(&now);
    tweak_common_timestamp_add_milliseconds(&wait_until, &now, timeout_millis);

    return tweak_common_convert_map_posix_error_code(pthread_cond_timedwait(cond, mutex, &wait_until));
  }
#endif
}

typedef bool (*tweak_common_thread_cond_wait_predicate_proc)(void* cookie);

static inline tweak_common_thread_error tweak_common_cond_timed_wait_with_pred(tweak_common_cond* cond,
  tweak_common_mutex* mutex, tweak_common_milliseconds timeout_millis,
  tweak_common_thread_cond_wait_predicate_proc predicate_proc, void* cookie)
{
  assert(cond != NULL);
  assert(mutex != NULL);

  bool condition_met = predicate_proc(cookie);
  tweak_common_thread_error result = TWEAK_COMMON_THREAD_SUCCESS;
  if (timeout_millis != TWEAK_COMMON_TIMESPAN_INFINITE) {
    int64_t milliseconds_to_wait = (uint64_t)timeout_millis;

    tweak_common_timestamp now;
    tweak_common_timestamp wait_until;

    tweak_common_timestamp_now(&now);
    tweak_common_timestamp_add_milliseconds(&wait_until, &now, timeout_millis);

    while (!condition_met && milliseconds_to_wait > 0 && result != TWEAK_COMMON_THREAD_FAILURE)
    {
      result = tweak_common_cond_timed_wait(cond, mutex, timeout_millis);
      condition_met = predicate_proc(cookie);
      tweak_common_timestamp_now(&now);
      milliseconds_to_wait = tweak_common_timestamp_subtract_timestamps(&wait_until, &now) / TWEAK_COMMON_NANOS_IN_MILLIS;
    }
  } else {
    while (!condition_met && result != TWEAK_COMMON_THREAD_FAILURE)
    {
      result = tweak_common_cond_wait(cond, mutex);
      condition_met = predicate_proc(cookie);
    }
  }

  return condition_met ? TWEAK_COMMON_THREAD_SUCCESS : result;
}

#endif
