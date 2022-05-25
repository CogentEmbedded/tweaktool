/**
 * @file tweakappcommon.c
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
#include <errno.h>
#include <inttypes.h>

#include "tweakappinternal.h"

bool tweak_app_context_private_check_value_compatibility(const tweak_variant* sample,
  const tweak_variant* value)
{
  assert (sample != NULL);
  assert (value != NULL);

  if (sample == value) {
    return true;
  }

  if (sample->type != value->type) {
    TWEAK_LOG_WARN("Constraint violation: type mismatch. target type = %d, source type = %d. Value rejected");
    return false;
  }

  switch(sample->type) {
  case TWEAK_VARIANT_TYPE_NULL:
    return false;
  case TWEAK_VARIANT_TYPE_BOOL:
  case TWEAK_VARIANT_TYPE_SINT8:
  case TWEAK_VARIANT_TYPE_SINT16:
  case TWEAK_VARIANT_TYPE_SINT32:
  case TWEAK_VARIANT_TYPE_SINT64:
  case TWEAK_VARIANT_TYPE_UINT8:
  case TWEAK_VARIANT_TYPE_UINT16:
  case TWEAK_VARIANT_TYPE_UINT32:
  case TWEAK_VARIANT_TYPE_UINT64:
  case TWEAK_VARIANT_TYPE_FLOAT:
  case TWEAK_VARIANT_TYPE_DOUBLE:
  case TWEAK_VARIANT_TYPE_STRING:
    return true;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    if (sample->value.buffer.size == value->value.buffer.size) {
      return true;
    } else {
      TWEAK_LOG_WARN("Constraint violation: Buffer size mismatch. sample size = %zu, requested size = %zu",
        sample->value.buffer.size, value->value.buffer.size);
      return false;
    }
  }
  TWEAK_FATAL("Unknown type: %d", sample->type);
  return false;
}

static void* io_loop(void* arg) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_base* app_context = arg;
  bool end_of_loop = false;
  while (!end_of_loop) {
    struct pull_jobs_result jobs_batch = tweak_app_queue_pull(app_context->job_queue);
    if (jobs_batch.is_stopped) {
      end_of_loop = true;
      continue;
    }
    const struct job_array* job_array = jobs_batch.job_array;
    for (size_t ix = 0; ix < job_array->size; ++ix) {
      struct job* job = &job_array->jobs[ix];
      assert(job->job_proc != NULL);
      job->job_proc(job->tweak_id, job->cookie);
    }
  }
  return NULL;
}

bool tweak_app_context_private_initialize_base(struct tweak_app_context_base* app_context, uint32_t queue_size) {
  TWEAK_LOG_TRACE_ENTRY("app_context = %p, queue_size = %u", app_context, queue_size);
  tweak_common_mutex_init(&app_context->conn_state_lock);

  app_context->model_impl.model = tweak_model_create();
  if (!app_context->model_impl.model) {
    TWEAK_LOG_ERROR("tweak_model_create() failed");
    goto destroy_connection_state_lock;
  }

  app_context->model_impl.index = tweak_model_uri_to_tweak_id_index_create();
  if (!app_context->model_impl.index) {
    TWEAK_LOG_ERROR("tweak_model_uri_to_tweak_id_index_create() failed");
    goto destroy_model;
  }

  if (tweak_common_rwlock_init(&app_context->model_impl.model_lock) != TWEAK_COMMON_THREAD_SUCCESS) {
    TWEAK_LOG_ERROR("tweak_common_rwlock_init() failed");
    goto destroy_tweak_id_index;
  }

  app_context->job_queue = tweak_app_queue_create(queue_size);
  if (app_context->job_queue == NULL) {
    TWEAK_LOG_ERROR("tweak_app_queue_create() failed");
    goto destroy_rwlock;
  }

  if (tweak_common_thread_create(&app_context->worker_thread, io_loop, app_context) != TWEAK_COMMON_THREAD_SUCCESS) {
    TWEAK_LOG_ERROR("Platform specific threading error in tweak_common_thread_create()");
    goto destroy_app_queue;
  }

  return true;

destroy_app_queue:
  tweak_app_queue_destroy(app_context->job_queue);

destroy_rwlock:
  tweak_common_rwlock_destroy(&app_context->model_impl.model_lock);

destroy_tweak_id_index:
  tweak_model_uri_to_tweak_id_index_destroy(app_context->model_impl.index);

destroy_model:
  tweak_model_destroy(app_context->model_impl.model);

destroy_connection_state_lock:
  tweak_common_mutex_destroy(&app_context->conn_state_lock);

  return false;
}

tweak_id tweak_app_find_id(tweak_app_context context, const char* uri) {
  TWEAK_LOG_TRACE_ENTRY("context = %p, uri = %s", context, uri);
  tweak_id id = TWEAK_INVALID_ID;
  tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
  id = tweak_model_uri_to_tweak_id_index_lookup(context->model_impl.index, uri);
  tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_TRACE("Item with uri = %s not found", uri);
  } else {
    TWEAK_LOG_TRACE("Found item with uri = %s and tweak_id = %" PRId64 "", uri, id);
  }
  return id;
}

struct traverse_context {
  struct tweak_app_context_base* context;
  tweak_app_traverse_items_callback user_callback;
  void* user_cookie;
};

static bool index_traverse_proc(const char *uri, tweak_id id, void* cookie) {
  (void) uri;
  TWEAK_LOG_TRACE_ENTRY("uri = %s, id = %" PRIu64 ", cookie= %p", uri, id, cookie);
  struct traverse_context* traverse_context = cookie;
   struct tweak_app_context_base* context = traverse_context->context;
  tweak_item* item = NULL;
  item = tweak_model_find_item_by_id(context->model_impl.model, id);
  assert(item);
  tweak_app_item_snapshot snapshot = {
    .id = item->id,
    .uri = item->uri,
    .description = item->description,
    .meta = item->meta,
    .default_value = item->default_value,
    .current_value = item->current_value
  };
  return traverse_context->user_callback(&snapshot, traverse_context->user_cookie);
}

bool tweak_app_traverse_items(tweak_app_context context,
  tweak_app_traverse_items_callback callback, void* cookie)
{
  TWEAK_LOG_TRACE_ENTRY("context = %p, callback = %p, cookie= %p", context, callback, cookie);
  struct traverse_context traverse_context = {
    .context = context,
    .user_callback = callback,
    .user_cookie = cookie
  };
  bool result;
  tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
  result = tweak_model_uri_to_tweak_id_index_walk(context->model_impl.index,
    &index_traverse_proc, &traverse_context);
  tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
  return result;
}

tweak_app_item_snapshot* tweak_app_item_get_snapshot(tweak_app_context context,
  tweak_id id)
{
  TWEAK_LOG_TRACE_ENTRY("context = %p, id = %" PRIu64 "", context, id);
  tweak_item* item = NULL;
  tweak_app_item_snapshot* snapshot = NULL;
  tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
  item = tweak_model_find_item_by_id(context->model_impl.model, id);
  if (item != NULL) {
    snapshot = calloc(1, sizeof(*snapshot));
    if (snapshot) {
      snapshot->id = item->id;
      snapshot->uri = tweak_variant_string_copy(&item->uri);
      snapshot->description = tweak_variant_string_copy(&item->description);
      snapshot->meta = tweak_variant_string_copy(&item->meta);
      snapshot->default_value = tweak_variant_copy(&item->default_value);
      snapshot->current_value = tweak_variant_copy(&item->current_value);
    } else {
      TWEAK_FATAL("Can't allocate memory for item state snapshot");
    }
  }
  tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
  return snapshot;
}

tweak_variant_type tweak_app_item_get_type(tweak_app_context context, tweak_id id) {
  TWEAK_LOG_TRACE_ENTRY("context = %p, id = %" PRIu64 "", context, id);
  tweak_variant_type result = TWEAK_VARIANT_TYPE_NULL;
  tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
  tweak_item* item = tweak_model_find_item_by_id(context->model_impl.model, id);
  if (item != NULL) {
    result = item->default_value.type;
    TWEAK_LOG_TRACE("Item with tweak_id = %" PRIu64 " has type %d", id, result);
  } else {
    TWEAK_LOG_TRACE("Item with tweak_id = %" PRIu64 " isn't found", id);
  }
  tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
  return result;
}

void tweak_app_release_snapshot(tweak_app_context context,
  tweak_app_item_snapshot* snapshot)
{
  (void) context;
  TWEAK_LOG_TRACE_ENTRY("context = %p, snapshot = %p", context, snapshot);
  tweak_variant_destroy_string(&snapshot->uri);
  tweak_variant_destroy_string(&snapshot->description);
  tweak_variant_destroy_string(&snapshot->meta);
  tweak_variant_destroy(&snapshot->default_value);
  tweak_variant_destroy(&snapshot->current_value);
  free(snapshot);
}

tweak_app_error_code tweak_app_item_clone_current_value(tweak_app_context context,
  tweak_id id, tweak_variant* value)
{
  return context->clone_current_value_proc(context, id, value);
}

tweak_app_error_code tweak_app_item_replace_current_value(tweak_app_context context,
  tweak_id id, tweak_variant* value)
{
  return context->replace_current_value_proc(context, id, value);
}

tweak_app_error_code tweak_app_context_private_item_clone_current_value(tweak_app_context context,
  tweak_id id, tweak_variant* value)
{
  TWEAK_LOG_TRACE_ENTRY("context = %p, tweak_id =  %" PRIu64 ", value = %p", context, id, value);
  tweak_item* item = NULL;
  tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
  item = tweak_model_find_item_by_id(context->model_impl.model, id);
  if (item) {
    *value = tweak_variant_copy(&item->current_value);
  }
  tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
  if (item) {
    TWEAK_LOG_TRACE("Current value of item with tweak_id = %" PRIu64 " has been cloned", id);
    return TWEAK_APP_SUCCESS;
  } else {
    TWEAK_LOG_TRACE("Item with tweak_id = %" PRIu64 " hasn't been found, can't clone", id);
    return TWEAK_APP_ITEM_NOT_FOUND;
  }
}

tweak_app_error_code tweak_app_item_get_metadata(tweak_app_context context,
  tweak_id id, tweak_metadata* metadata)
{
  TWEAK_LOG_TRACE_ENTRY("context = %p, tweak_id =  %" PRIu64 ", metadata = %p", context, id, metadata);
  assert(metadata != NULL);
  tweak_app_error_code result;
  tweak_item* item = NULL;
  tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
  item = tweak_model_find_item_by_id(context->model_impl.model, id);
  if (item) {
    if (!item->metadata_initialized) {
      tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
      tweak_common_rwlock_write_lock(&context->model_impl.model_lock);
      if (!item->metadata_initialized) {
        item->metadata = tweak_metadata_create(item->current_value.type,
          tweak_variant_get_item_count(&item->current_value),
          tweak_variant_string_c_str(&item->meta));
        item->metadata_initialized = true;
      }
      tweak_common_rwlock_write_unlock(&context->model_impl.model_lock);
      tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
    }
    *metadata = tweak_metadata_copy(item->metadata);
    result = TWEAK_APP_SUCCESS;
  } else {
    TWEAK_LOG_ERROR("Item with tweak_id = %" PRIu64 " hasn't been found", id);
    result = TWEAK_APP_ITEM_NOT_FOUND;
  }
  tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
  return result;
}

tweak_app_error_code tweak_app_context_private_item_replace_current_value(tweak_app_context context,
  tweak_id tweak_id, tweak_variant* value)
{
  tweak_app_error_code result;
  tweak_item* item = NULL;
  bool should_push_change = false;
  tweak_common_rwlock_write_lock(&context->model_impl.model_lock);
  item = tweak_model_find_item_by_id(context->model_impl.model, tweak_id);
  if (item) {
    if (!tweak_variant_is_equal(&item->current_value, value)) {
      tweak_variant_swap(&item->current_value, value);
      bool item_is_compatible =
        tweak_app_features_check_type_compatibility(&context->remote_peer_features, value->type);
      should_push_change = item_is_compatible && tweak_app_context_private_is_connected(context);
    } else {
      TWEAK_LOG_TRACE("Omitting redundant value update");
    }
    tweak_variant_destroy(value);
    result = TWEAK_APP_SUCCESS;
  } else {
    result = TWEAK_APP_ITEM_NOT_FOUND;
  }
  tweak_common_rwlock_write_unlock(&context->model_impl.model_lock);
  if (should_push_change) {
    assert(context->push_changes_proc != NULL);
    context->push_changes_proc(context, tweak_id);
  }
  if (result == TWEAK_APP_SUCCESS) {
    TWEAK_LOG_TRACE("Current value of item with tweak_id = %" PRIu64 " has been updated", tweak_id);
  } else {
    TWEAK_LOG_TRACE("Item with tweak_id = %" PRIu64 " hasn't been found, can't update", tweak_id);
  }
  return result;
}

void tweak_app_context_private_set_connected(struct tweak_app_context_base* app_context, bool arg) {
  TWEAK_LOG_TRACE_ENTRY("app_context = %p, connected = %s", app_context, arg ? "true" : "false");
  tweak_common_mutex_lock(&app_context->conn_state_lock);
  app_context->connected = arg;
  tweak_common_mutex_unlock(&app_context->conn_state_lock);
}

bool tweak_app_context_private_is_connected(struct tweak_app_context_base* app_context) {
  TWEAK_LOG_TRACE_ENTRY("app_context = %p", app_context);
  bool result;
  tweak_common_mutex_lock(&app_context->conn_state_lock);
  result = app_context->connected;
  tweak_common_mutex_unlock(&app_context->conn_state_lock);
  TWEAK_LOG_TRACE("result = %s", result ? "true" : "false");
  return result;
}

void tweak_app_context_private_destroy_base(struct tweak_app_context_base* app_context) {
  TWEAK_LOG_TRACE_ENTRY("app_context = %p", app_context);
  tweak_common_thread_join(app_context->worker_thread, NULL);
  tweak_app_queue_destroy(app_context->job_queue);
  tweak_model_destroy(app_context->model_impl.model);
  tweak_model_uri_to_tweak_id_index_destroy(app_context->model_impl.index);
  tweak_common_rwlock_destroy(&app_context->model_impl.model_lock);
  tweak_common_mutex_destroy(&app_context->conn_state_lock);
}

void tweak_app_flush_queue(tweak_app_context context) {
  tweak_app_queue_wait_empty(context->job_queue);
}

void tweak_app_destroy_context(tweak_app_context context) {
  TWEAK_LOG_TRACE_ENTRY("context = %p", context);
  assert(context->destroy_context != NULL);
  context->destroy_context(context);
}
