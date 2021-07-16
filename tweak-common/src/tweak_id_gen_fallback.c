/**
 * @file tweak_id_gen_fallback.c
 * @ingroup tweak-api
 *
 * @brief Utility method to generate tweak ids in multi thread env.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/types.h>
#include <pthread.h>

static pthread_once_t lock_init_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static tweak_id seed = 1UL;

static void lock_init() {
  pthread_mutex_init(&lock, NULL);
}

tweak_id tweak_common_genid() {
  tweak_id result;
  pthread_once(&lock_init_once, lock_init);
  pthread_mutex_lock(&lock);
  result = seed;
  ++seed;
  pthread_mutex_unlock(&lock);
  return result;
}
