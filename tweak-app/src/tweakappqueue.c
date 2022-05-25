/**
 * @file tweakappqueue.c
 * @ingroup tweak-api
 *
 * @brief part of tweak2 application implementation.
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

#include "tweakappqueue.h"

#include <stdlib.h>
#include <string.h>

struct job_queue {
  tweak_common_mutex lock;
  tweak_common_cond cond;
  int current_array;
  struct job_array arrays[2];
  size_t max_size;
  bool is_stopped;
};

struct job_queue* tweak_app_queue_create(size_t max_size) {
  struct job_queue* job_queue = calloc(1, sizeof(*job_queue));
  if (!job_queue) {
    return NULL;
  }

  job_queue->max_size = max_size;
  tweak_common_mutex_init(&job_queue->lock);
  tweak_common_cond_init(&job_queue->cond);
  return job_queue;
}

struct pull_jobs_result tweak_app_queue_pull(struct job_queue* job_queue) {
  struct pull_jobs_result result;
  tweak_common_mutex_lock(&job_queue->lock);
  while (!job_queue->is_stopped && job_queue->arrays[job_queue->current_array].size == 0) {
    tweak_common_cond_wait(&job_queue->cond, &job_queue->lock);
  }
  if (job_queue->is_stopped) {
    result.is_stopped = true;
    result.job_array = NULL;
  } else {
    result.is_stopped = false;
    result.job_array = &job_queue->arrays[job_queue->current_array];
  }
  job_queue->current_array = (job_queue->current_array + 1) % 2;
  job_queue->arrays[job_queue->current_array].size = 0;
  tweak_common_cond_broadcast(&job_queue->cond);
  tweak_common_mutex_unlock(&job_queue->lock);
  return result;
}

static void ensure_job_array_capacity(struct job_array* job_array, size_t size) {
  if (size > job_array->capacity) {
    size_t new_capacity;
    if (size < 10) {
      new_capacity = 10;
    } else {
      new_capacity = size * 3 / 2;
    }
    job_array->jobs = realloc(job_array->jobs, new_capacity * sizeof(job_array->jobs[0]));
    if (!job_array->jobs) {
      TWEAK_FATAL("Can't allocate memory for io queue");
    }
    job_array->capacity = new_capacity;
  }
}

void tweak_app_queue_push(struct job_queue* job_queue, const struct job* job) {
  tweak_common_mutex_lock(&job_queue->lock);
  struct job_array* job_array = &job_queue->arrays[job_queue->current_array];
  for (int i = (int)(job_array->size - 1); i >= 0; --i) {
    if (job_array->jobs[i].tweak_id == job->tweak_id
        && job_array->jobs[i].job_proc == job->job_proc
        && job_array->jobs[i].cookie == job->cookie)
    {
      goto item_present;
    }
  }

  while (job_queue->arrays[job_queue->current_array].size >= job_queue->max_size) {
    tweak_common_cond_wait(&job_queue->cond, &job_queue->lock);
  }
  ensure_job_array_capacity(job_array, job_array->size + 1);
  job_array->jobs[job_array->size] = *job;
  ++job_array->size;

item_present:
  tweak_common_cond_broadcast(&job_queue->cond);
  tweak_common_mutex_unlock(&job_queue->lock);
}

void tweak_app_queue_stop(struct job_queue* job_queue) {
  tweak_common_mutex_lock(&job_queue->lock);
  job_queue->is_stopped = true;
  tweak_common_cond_broadcast(&job_queue->cond);
  tweak_common_mutex_unlock(&job_queue->lock);
}

static void free_job_array(struct job_array* job_array) {
  free(job_array->jobs);
  memset(job_array, 0, sizeof(*job_array));
}

void tweak_app_queue_destroy(struct job_queue* job_queue) {
  free_job_array(&job_queue->arrays[0]);
  free_job_array(&job_queue->arrays[1]);
  tweak_common_cond_destroy(&job_queue->cond);
  tweak_common_mutex_destroy(&job_queue->lock);
  free(job_queue);
}

void tweak_app_queue_wait_empty(struct job_queue* job_queue) {
  tweak_common_mutex_lock(&job_queue->lock);
  while (job_queue->arrays[job_queue->current_array].size != 0) {
    tweak_common_cond_wait(&job_queue->cond, &job_queue->lock);
  }
  tweak_common_mutex_unlock(&job_queue->lock);
}

bool tweak_app_queue_is_stopped(struct job_queue* job_queue) {
  bool result;
  tweak_common_mutex_lock(&job_queue->lock);
  result = job_queue->is_stopped;
  tweak_common_mutex_unlock(&job_queue->lock);
  return result;
}
