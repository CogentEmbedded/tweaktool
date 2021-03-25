/**
 * @file tweakappinternal.h
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

#ifndef TWEAK_APP_INTERNAL_H_INCLUDED
#define TWEAK_APP_INTERNAL_H_INCLUDED

#include <tweak2/appcommon.h>

#include "tweakappqueue.h"
#include "tweakmodel.h"
#include "tweakmodel_uri_to_tweak_id_index.h"

#include <pthread.h>

/**
 * @brief Prototype for virtual method pushing change request to io queue.
 *
 * @param context a context instance.
 * @param tweak_id item on which change event had occurred.
 */
typedef void (*push_changes_proc)(struct tweak_app_context_base* context, tweak_id tweak_id);

/**
 * @brief Prototype for virtual method to clone an item's value.
 *
 * @param context a context instance.
 * @param tweak_id id item id to access.
 * @param value as output parameter to store cloned value.
 *
 * @return value indicating success of an operation.
 */
typedef tweak_app_error_code (*clone_current_value_proc)(tweak_app_context context,
  tweak_id id, tweak_variant* value);

/**
 * @brief Prototype for virtual method to alter an item's value.
 *
 * @param context a context instance.
 * @param tweak_id id item id to access.
 * @param value as input parameter to store value.
 *
 * @return value indicating success of an operation.
 */
typedef tweak_app_error_code (*replace_current_value_proc)(tweak_app_context context,
  tweak_id tweak_id, tweak_variant* value);

/**
 * @brief Prototype for virtual destructor for all context types.
 *
 * @param context a context instance.
 */
typedef void (*tweak_app_destroy_context_proc)(struct tweak_app_context_base* context);

struct tweak_model_impl {
  /**
   * @brief Synchronization primitive to access model data.
   */
  pthread_rwlock_t model_lock;
  /**
   * @brief Item storage.
   */
  tweak_model model;
  /**
   * @brief Item index.
   */
  tweak_model_uri_to_tweak_id_index index;
};

/**
 * @brief Abstract base class for all context types.
 *
 * @note Concrete implementation shall
 * provide valid implementation for all these methods.
 */
struct tweak_app_context_base {
  /**
   * @brief Synchronization primitive to access connected status.
   */
  pthread_mutex_t conn_state_lock;
  /**
   * @brief True if remote peer is connected to the context.
   */
  bool connected;
  /**
   * @brief Asynchronous I/O routine.
   */
  pthread_t worker_thread;
  /**
   * @brief Queue of jobs to run by worker_thread.
   */
  struct job_queue* job_queue;
  /**
   * @brief Item storage.
   */
  struct tweak_model_impl model_impl;
  /**
   * @brief Virtual function to clone item value.
   */
  clone_current_value_proc clone_current_value_proc;
  /**
   * @brief Virtual function to replace item value.
   */
  replace_current_value_proc replace_current_value_proc;
  /**
   * @brief Virtual function to push change request to connected peer.
   */
  push_changes_proc push_changes_proc;
  /**
   * @brief Virtual destructor.
   */
  tweak_app_destroy_context_proc destroy_context;
};

/**
 * @brief Initialize instance of the base class common for server and client.
 *
 * @param app_context pointer to preallocated memory buffer large enough
 * to contain sizeof(struct tweak_app_context_base) bytes.
 * @param queue_size size of queue containing delayed io requests
 *
 * @return true if base class has been initialized successfully.
 */
bool tweak_app_context_private_initialize_base(struct tweak_app_context_base* app_context, uint32_t queue_size);

/**
 * @brief Deinitialize instance of the base class common for server and client.
 * It won't free memory occupied by instance, it's up to subclass.
 *
 * @param app_context pointer to valid running instance of tweak_app_context_base class.
 */
void tweak_app_context_private_destroy_base(struct tweak_app_context_base* app_context);

/**
 * @brief Clone item's value to designated tweak_variant instance.
 *
 * @note this method is more selective than tweak_app_item_get_attributes.
 *
 * @param context an application context.
 * @param id tweak id.
 * @param value a pointer to instance. Preexisting data will be released with
 * tweak_variant_destroy() call.
 *
 * @return TWEAK_APP_SUCCESS if there wasn't any errors.
 */
tweak_app_error_code tweak_app_context_private_item_clone_current_value(tweak_app_context context,
  tweak_id id, tweak_variant* value);

/**
 * @brief Swap item's value with provided @p value and propagate new item's value
 * to the connected peer, it there's one.
 *
 * @note this is a template method operating on internal model and providing
 * common functionality for both client and server implementations of this context.
 * Shouldn't be used directly.
 *
 * @param context an application context.
 * @param id tweak id.
 * @param value a pointer to new value. After the call its contents will be replaced
 * by previous value of item. A user could repurpose the storage associated with
 * this tweak_variant instance or release it immediately with tweak_variant_destroy
 * call.
 *
 * @return TWEAK_APP_SUCCESS if there wasn't any errors.
 */
tweak_app_error_code tweak_app_context_private_item_replace_current_value(tweak_app_context context,
  tweak_id tweak_id, tweak_variant* value);

/**
 * @brief Set "connected" status for the instance. When connected, subclasses shall sync
 * their state with connected peers.
 *
 * @note this is a template method operating on internal model and providing
 * common functionality for both client and server implementations of this context.
 * Shouldn't be used directly.
 *
 * @param app_context pointer to valid running instance of tweak_app_context_base class.
 * @param arg true is application context is going online.
 */
void tweak_app_context_private_set_connected(struct tweak_app_context_base* app_context, bool arg);

/**
 * @brief Query "connected" status of the instance. When connected, subclasses shall sync
 * their state with connected peers.
 *
 * @param app_context pointer to valid running instance of tweak_app_context_base class.
 * @return true if remote peer is connected to the context.
 */
bool tweak_app_context_private_is_connected(struct tweak_app_context_base* app_context);

#endif
