/**
 * @file test-queue.c
 * @ingroup tweak-app-implementation-test
 *
 * @brief part of test suite to test tweak2 application implementation.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-app-implementation-test Test implementation for tweak-app internal interfaces.
 */

#include <tweak2/string.h>
#include <tweak2/variant.h>

#include "tweakappqueue.h"

#include <acutest.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

enum { NUM_ITERATIONS = 1000 };

void sleepmillis(int milliseconds) // Cross-platform sleep function
{
  usleep(milliseconds * 1000);
}

static tweak_id seed = 0;
static tweak_id gen_id() {
  return ++seed;
}

static void job1(tweak_id id, void* cookie) {
  tweak_id* pout = cookie;
  *pout = ((id * 17) % 19);
}

static void job2(tweak_id id, void* cookie) {
  tweak_id* pout = cookie;
  *pout = (((int)id * 19) % 23);
}

static void job3(tweak_id id, void* cookie) {
  tweak_id* pout = cookie;
  *pout = (((int)id * 23) % 29);
}

job_proc jobs[] = {&job1, &job2, &job3};

enum { JOB_ARR_SIZE = sizeof(jobs) / sizeof(jobs[0])};

static tweak_id s_out = TWEAK_INVALID_ID;

static void* producer(void *arg) {
  struct job_queue* job_queue = arg;
  for (uint32_t ix = 0; ix < NUM_ITERATIONS; ix++) {
    if (!(ix % 1000)) {
      TWEAK_LOG_TEST("Produce: %d\n", (ix / 1000) + 1);
    }
    tweak_id tweak_id = gen_id();
    struct job job = {
      .tweak_id = tweak_id,
      .job_proc = jobs[tweak_id % JOB_ARR_SIZE],
      .cookie = &s_out
    };
    tweak_app_queue_push(job_queue, &job);
  }
  tweak_app_queue_stop(job_queue);
}

static void* slow_producer(void *arg) {
  struct job_queue* job_queue = arg;
  for (uint32_t ix = 0; ix < NUM_ITERATIONS; ix++) {
    if (!(ix % 1000)) {
      TWEAK_LOG_TEST("Produce: %d\n", (ix / 10) + 1);
    }
    tweak_id tweak_id = gen_id();
    struct job job = {
      .tweak_id = tweak_id,
      .job_proc = jobs[tweak_id % JOB_ARR_SIZE],
      .cookie = &s_out
    };
    tweak_app_queue_push(job_queue, &job);
    sleepmillis(10);
  }
  tweak_app_queue_stop(job_queue);
}

void test_queue() {
  int status;
  pthread_t thread = { 0 };
  struct job_queue* job_queue = NULL;
  TWEAK_LOG_TEST("Equal speed produces/consumer");
  {
    job_queue = tweak_app_queue_create(100);
    TEST_CHECK(job_queue != NULL);
    status = pthread_create(&thread, NULL, &producer, job_queue);
    TEST_CHECK(status == 0);
    uint32_t batch_no = 0;
    for (;;) {
      struct pull_jobs_result pull_jobs_result = tweak_app_queue_pull(job_queue);
      if (pull_jobs_result.is_stopped)
        break;

      ++batch_no;
      TWEAK_LOG_TEST("Consume: %d\n", batch_no);

      const struct job_array* job_array = pull_jobs_result.job_array;

      for (size_t ix = 0; ix < job_array->size; ix++) {
        const struct job* job  = &job_array->jobs[ix];
        job_proc job_proc = jobs[job->tweak_id % JOB_ARR_SIZE];
        tweak_id tst;
        job_proc(job->tweak_id, &tst);
        job->job_proc(job->tweak_id, job->cookie);
        TEST_CHECK(tst == s_out);
      }
    }
    pthread_join(thread, NULL);
    tweak_app_queue_destroy(job_queue);
  }
  {
    TWEAK_LOG_TEST("Slow producer");
    job_queue = tweak_app_queue_create(100);
    TEST_CHECK(job_queue != NULL);
    status = pthread_create(&thread, NULL, &slow_producer, job_queue);
    TEST_CHECK(status == 0);
    uint32_t batch_no = 0;
    for (;;) {
      struct pull_jobs_result pull_jobs_result = tweak_app_queue_pull(job_queue);
      if (pull_jobs_result.is_stopped)
        break;

      ++batch_no;
      TWEAK_LOG_TEST("Consume: %d\n", batch_no);

      const struct job_array* job_array = pull_jobs_result.job_array;

      for (size_t ix = 0; ix < job_array->size; ix++) {
        const struct job* job  = &job_array->jobs[ix];
        job_proc job_proc = jobs[job->tweak_id % JOB_ARR_SIZE];
        tweak_id tst;
        job_proc(job->tweak_id, &tst);
        job->job_proc(job->tweak_id, job->cookie);
        TEST_CHECK(tst == s_out);
      }
    }
    pthread_join(thread, NULL);
    tweak_app_queue_destroy(job_queue);
  }
  {
    TWEAK_LOG_TEST("Slow consumer");
    job_queue = tweak_app_queue_create(100);
    TEST_CHECK(job_queue != NULL);
    status = pthread_create(&thread, NULL, &producer, job_queue);
    TEST_CHECK(status == 0);
    uint32_t batch_no = 0;
    for (;;) {
      struct pull_jobs_result pull_jobs_result = tweak_app_queue_pull(job_queue);
      if (pull_jobs_result.is_stopped)
        break;

      ++batch_no;
      TWEAK_LOG_TEST("Consume: %d\n", batch_no);

      const struct job_array* job_array = pull_jobs_result.job_array;

      for (size_t ix = 0; ix < job_array->size; ix++) {
        const struct job* job  = &job_array->jobs[ix];
        job_proc job_proc = jobs[job->tweak_id % JOB_ARR_SIZE];
        tweak_id tst;
        job_proc(job->tweak_id, &tst);
        job->job_proc(job->tweak_id, job->cookie);
        TEST_CHECK(tst == s_out);
      }
      sleepmillis(10);
    }
    pthread_join(thread, NULL);
    tweak_app_queue_destroy(job_queue);
  }
}

TEST_LIST = {
   { "test_queue", test_queue },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};

