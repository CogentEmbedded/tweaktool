/**
 * @file tweakappqueue.h
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

#ifndef TWEAK_APP_QUEUE_H_INCLUDED
#define TWEAK_APP_QUEUE_H_INCLUDED

#include <tweak2/types.h>

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief job routine.
 *
 * @param tweak_id id of an item to apply the job.
 * @param cookie user defined context for the job.
 */
typedef void (*job_proc)(tweak_id tweak_id, void* cookie);

/**
 * @brief a job for event queue.
 */
struct job {
  /**
   * @brief job routine.
   */
  job_proc job_proc;
  /**
   * @brief id of an item to apply the job.
   */
  tweak_id tweak_id;
  /**
   * @brief user defined context for the job.
   */
  void* cookie;
};

/**
 * @brief Container for jobs.
 */
struct job_array {
  /**
   * @brief Current array size.
   */
  size_t size;
  /**
   * @brief How much entries are allocated for jobs array.
   */
  size_t capacity;
  /**
   * @brief Jobs array.
   */
  struct job* jobs;
};

/**
 * @brief Queue implementation.
 */
struct job_queue;

/**
 * @brief Result of pull operation.
 * @see tweak_app_queue_pull.
 */
struct pull_jobs_result {
  /**
   * @brief Jobs to execute. Can be NULL if termination_request is true.
   */
  const struct job_array* job_array;
  /**
   * @brief If true, job processing thread should finish its work and exit.
   */
  bool is_stopped;
};

/**
 * @brief Initialize a queue.
 *
 * @param max_size Queue won't grow larger than this.
 * tweak_app_queue_push will block unless queue is emptied
 * by tweak_app_queue_pull method called from job processing thread.
 * @return job_queue instance or NULL if it couldn't initialize
 * synchronization primitives or allocate memory.
 */
struct job_queue* tweak_app_queue_create(size_t max_size);

/**
 * @brief Pull a job batch from a queue.
 *
 * Blocks unless there's at least 1 job available in the queue
 * or termination is requested.
 *
 * @param job_queue Queue instance.
 * @return batch containing all pending jobs
 */
struct pull_jobs_result tweak_app_queue_pull(struct job_queue* job_queue);

/**
 * @brief Push a job into a queue.
 *
 * Blocks until there's less than max_capacity jobs in the queue
 * or termination is requested.
 *
 * @param job_queue Queue struct.
 * @return true if job is appended to the queue, false if operation
 * is interrupted by termination request.
 */
void tweak_app_queue_push(struct job_queue* job_queue, const struct job* job);

/**
 * @brief Push a termination request into a queue.
 *
 * Doesn't block. All locks are released. Their owners are notified to
 * abandon all pending jobs and exit from their data handling loops.
 *
 * @param job_queue Queue struct.
 */
void tweak_app_queue_stop(struct job_queue* job_queue);

/**
 * @brief Waits until there's no pending entries in the queue.
 *
 * @param job_queue Queue struct.
 */
void tweak_app_queue_wait_empty(struct job_queue* job_queue);

/**
 * @brief Check if @see tweak_app_queue_stop has been called on this instance.
 *
 * @param job_queue Queue struct.
 * @return true if condition has been met.
 */
bool tweak_app_queue_is_stopped(struct job_queue* job_queue);

/**
 * @brief Release all resources.

 * @param job_queue Queue struct.
 */
void tweak_app_queue_destroy(struct job_queue* job_queue);

#endif
