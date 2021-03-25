/**
 * @file tweakappqueue.c
 * @ingroup tweak-api
 *
 * @brief part of tweak2 application implementation.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/log.h>

#include "tweakappqueue.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
struct job_queue {
  pthread_mutex_t lock;
  pthread_cond_t cond;
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

  if (pthread_mutex_init(&job_queue->lock, NULL) != 0) {
    free(job_queue);
    return NULL;
  }

  if (pthread_cond_init(&job_queue->cond, NULL) != 0) {
    pthread_mutex_destroy(&job_queue->lock);
    free(job_queue);
    return NULL;
  }

  return job_queue;
}

struct pull_jobs_result tweak_app_queue_pull(struct job_queue* job_queue) {
  struct pull_jobs_result result;
  pthread_mutex_lock(&job_queue->lock);
  while (!job_queue->is_stopped && job_queue->arrays[job_queue->current_array].size == 0) {
    pthread_cond_wait(&job_queue->cond, &job_queue->lock);
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
  pthread_cond_broadcast(&job_queue->cond);
  pthread_mutex_unlock(&job_queue->lock);
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
  pthread_mutex_lock(&job_queue->lock);
  struct job_array* job_array = &job_queue->arrays[job_queue->current_array];
  while (job_queue->arrays[job_queue->current_array].size >= job_queue->max_size) {
    pthread_cond_wait(&job_queue->cond, &job_queue->lock);
  }
  ensure_job_array_capacity(job_array, job_array->size + 1);
  job_array->jobs[job_array->size] = *job;
  ++job_array->size;
  pthread_cond_broadcast(&job_queue->cond);
  pthread_mutex_unlock(&job_queue->lock);
}

void tweak_app_queue_stop(struct job_queue* job_queue) {
  pthread_mutex_lock(&job_queue->lock);
  job_queue->is_stopped = true;
  pthread_cond_broadcast(&job_queue->cond);
  pthread_mutex_unlock(&job_queue->lock);
}

static void free_job_array(struct job_array* job_array) {
  free(job_array->jobs);
  memset(job_array, 0, sizeof(*job_array));
}

void tweak_app_queue_destroy(struct job_queue* job_queue) {
  free_job_array(&job_queue->arrays[0]);
  free_job_array(&job_queue->arrays[1]);
  pthread_cond_destroy(&job_queue->cond);
  pthread_mutex_destroy(&job_queue->lock);
  free(job_queue);
}

bool tweak_app_queue_is_stopped(struct job_queue* job_queue) {
  bool result;
  pthread_mutex_lock(&job_queue->lock);
  result = job_queue->is_stopped;
  pthread_mutex_unlock(&job_queue->lock);
  return result;
}
