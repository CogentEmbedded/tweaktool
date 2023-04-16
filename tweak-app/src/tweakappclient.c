/**
 * @file tweakappclient.c
 * @ingroup tweak-api
 *
 * @brief part of tweak2 application implementation.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#include <tweak2/appclient.h>
#include <tweak2/log.h>
#include <tweak2/pickle_client.h>
#include <tweak2/thread.h>

#include "tweakappinternal.h"
#include "tweakappqueue.h"
#include "tweakmodel.h"
#include "tweakmodel_uri_to_tweak_id_index.h"
#include "tweakappfeatures.h"

#include <inttypes.h>
#include <stdio.h>
#include <time.h>

enum { TWEAK_APP_CLIENT_QUEUE_SIZE = 100 };

struct wait_context {
  tweak_common_mutex lock;
  tweak_common_cond cond;
};

struct tweak_app_context_client_impl {
  struct tweak_app_context_base base;
  tweak_app_client_callbacks client_callbacks;
  tweak_pickle_client_endpoint rpc_endpoint;
  struct tweak_app_features remote_peer_features;
  struct wait_context wait_context;
};

static bool init_wait_context(struct wait_context* wait_context) {
  if (tweak_common_mutex_init(&wait_context->lock) != TWEAK_COMMON_THREAD_SUCCESS) {
    return false;
  }
  if (tweak_common_cond_init(&wait_context->cond) != TWEAK_COMMON_THREAD_SUCCESS) {
    tweak_common_mutex_destroy(&wait_context->lock);
    return false;
  }
  return true;
}

static void wait_context_notify_all(struct wait_context* wait_context) {
  tweak_common_mutex_lock(&wait_context->lock);
  tweak_common_cond_broadcast(&wait_context->cond);
  tweak_common_mutex_unlock(&wait_context->lock);
}

static void destroy_wait_context(struct wait_context* wait_context) {
  tweak_common_cond_destroy(&wait_context->cond);
  tweak_common_mutex_destroy(&wait_context->lock);
}

static void io_loop_subscribe(tweak_id tweak_id, void* cookie) {
  (void)tweak_id;
  TWEAK_LOG_TRACE_ENTRY("tweak_id = %" PRId64 " , cookie = %p", cookie);
  tweak_pickle_call_result result;
  struct tweak_app_context_client_impl* client_impl = cookie;

  struct tweak_app_features my_supported_features = { 0 };
  tweak_app_features_init_default(&my_supported_features);

  tweak_variant_string my_supported_features_json = tweak_app_features_to_json(&my_supported_features);
  tweak_pickle_features features = {
    .features = my_supported_features_json
  };

  result = tweak_pickle_client_announce_features(client_impl->rpc_endpoint, &features);
  if (result != TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_WARN("tweak_pickle_client_announce_features() returned %d", result);
  }
  tweak_variant_destroy_string(&my_supported_features_json);

  tweak_pickle_subscribe subscribe = { 0 };
  result = tweak_pickle_client_subscribe(client_impl->rpc_endpoint, &subscribe);
  if (result != TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_ERROR("tweak_pickle_client_subscribe() returned %d", result);
  }
}

static void push_subscribe(tweak_app_context context) {
  TWEAK_LOG_TRACE_ENTRY();
  struct job job = {
    .job_proc = &io_loop_subscribe,
    .cookie = context
  };
  tweak_app_queue_push(context->job_queue, &job);
}

static bool invoke_item_removed_callback(const char *uri, tweak_id id, void* cookie) {
  TWEAK_LOG_TRACE_ENTRY("uri = %s, tweak_id = %" PRId64 ", cookie = %p", uri, id, cookie);
  (void) uri;

  struct tweak_app_context_client_impl* client_impl =
    (struct tweak_app_context_client_impl*) cookie;

  client_impl->client_callbacks.on_item_removed(&client_impl->base,
    id, client_impl->client_callbacks.cookie);

  return true;
}

static void call_remove_callback_for_all_items(
		struct tweak_app_context_client_impl* client_impl,
		tweak_model_uri_to_tweak_id_index index) {
  TWEAK_LOG_TRACE_ENTRY();

  if (client_impl->client_callbacks.on_item_removed) {
    TWEAK_LOG_TRACE("Invoking on_item_removed for all items");
    tweak_model_uri_to_tweak_id_index_walk(index, &invoke_item_removed_callback, client_impl);
  } else {
    TWEAK_LOG_TRACE("on_item_removed is NULL");
  }
}

static void reset_model(struct tweak_app_context_client_impl* client_impl) {
  TWEAK_LOG_TRACE_ENTRY();
  tweak_model new_model = tweak_model_create();
  if (!new_model) {
    TWEAK_FATAL("Can't allocate new model upon reconnect");
  }

  tweak_model_uri_to_tweak_id_index new_index = tweak_model_uri_to_tweak_id_index_create();
  if (!new_index) {
    TWEAK_FATAL("Can't recreate new model index upon reconnect");
  }

  struct tweak_model_impl* model = &client_impl->base.model_impl;
  tweak_model old_model = model->model;
  tweak_model_uri_to_tweak_id_index old_index = model->index;

  tweak_common_rwlock_write_lock(&model->model_lock);
  model->model = new_model;
  model->index = new_index;
  tweak_common_rwlock_write_unlock(&model->model_lock);

  call_remove_callback_for_all_items(client_impl, old_index);

  tweak_model_destroy(old_model);
  tweak_model_uri_to_tweak_id_index_destroy(old_index);
}

static void announce_features_impl(tweak_pickle_features* announce_features,
  void *cookie)
{
  TWEAK_LOG_TRACE_ENTRY("announce_features = \"%s\", cookie = %p",
    tweak_variant_string_c_str(&announce_features->features), cookie);
  struct tweak_app_context_client_impl* client_impl = (struct tweak_app_context_client_impl*) cookie;
  struct tweak_app_features remote_peer_features;
  tweak_app_features_init_minimal(&remote_peer_features);
  if (!tweak_app_features_from_json(&announce_features->features, &remote_peer_features)) {
    TWEAK_LOG_WARN("Can't parse server features, using scalar tweaks only");
    return;
  }
  TWEAK_LOG_DEBUG("Remote side vector support: %s", remote_peer_features.vectors ? "yes" : "no");
  tweak_common_mutex_lock(&client_impl->base.conn_state_lock);
  client_impl->base.remote_peer_features = remote_peer_features;
  tweak_common_mutex_unlock(&client_impl->base.conn_state_lock);
}

static void client_connection_state_pickle_impl(tweak_pickle_connection_state connection_state,
  void *cookie)
{
  TWEAK_LOG_TRACE_ENTRY("connection_state = %d, cookie = %p", connection_state,
    cookie);
  struct tweak_app_context_client_impl* client_impl = (struct tweak_app_context_client_impl*) cookie;
  switch (connection_state) {
  case TWEAK_PICKLE_CONNECTED:
    tweak_common_mutex_lock(&client_impl->base.conn_state_lock);
    tweak_app_features_init_minimal(&client_impl->base.remote_peer_features);
    tweak_common_mutex_unlock(&client_impl->base.conn_state_lock);
    reset_model(client_impl);
    tweak_app_context_private_set_connected(&client_impl->base, true);
    if (client_impl->client_callbacks.on_connection_status_changed != NULL) {
      TWEAK_LOG_TRACE("Calling on_connection_status_changed(TWEAK_PICKLE_CONNECTED)");
      client_impl->client_callbacks.on_connection_status_changed(&client_impl->base,
        true, client_impl->client_callbacks.cookie);
    } else {
      TWEAK_LOG_TRACE("on_connection_status_changed is NULL");
    }
    push_subscribe(&client_impl->base);
    break;
  case TWEAK_PICKLE_DISCONNECTED:
    if (client_impl->client_callbacks.on_connection_status_changed != NULL) {
      TWEAK_LOG_TRACE("Calling on_connection_status_changed(TWEAK_PICKLE_DISCONNECTED)");
      client_impl->client_callbacks.on_connection_status_changed(&client_impl->base,
        false, client_impl->client_callbacks.cookie);
    } else {
      TWEAK_LOG_TRACE("on_connection_status_changed is NULL");
    }
    tweak_app_context_private_set_connected(&client_impl->base, false);
    break;
  }
  wait_context_notify_all(&client_impl->wait_context);
}

static tweak_id add_item_to_model(struct tweak_model_impl* model, tweak_pickle_add_item *add_item) {
  TWEAK_LOG_TRACE_ENTRY();

  tweak_model_error_code model_error_code = tweak_model_create_item(model->model,
    add_item->id, &add_item->uri, &add_item->description, &add_item->meta, &add_item->default_value,
    &add_item->current_value, NULL);

  if (model_error_code != TWEAK_MODEL_SUCCESS) {
    TWEAK_LOG_TRACE("Can't handle add_item request for tweak_id = %" PRIu64 "\n", add_item->id);
    return TWEAK_INVALID_ID;
  }

  if (tweak_model_uri_to_tweak_id_index_insert(model->index,
    tweak_variant_string_c_str(&add_item->uri), add_item->id) != TWEAK_MODEL_INDEX_SUCCESS)
  {
    TWEAK_LOG_TRACE("Can't handle add_item request for tweak_id = %" PRIu64 "\n", add_item->id);
    tweak_model_remove_item(model->model, add_item->id);
    return TWEAK_INVALID_ID;
  }
  return add_item->id;
}

bool is_add_item_correct(tweak_pickle_add_item *add_item, tweak_item* item)
{
  return (
      item->id == add_item->id &&
      tweak_variant_str_is_equal(&item->uri, &add_item->uri) &&
      tweak_variant_str_is_equal(&item->description, &add_item->description) &&
      tweak_variant_str_is_equal(&item->meta, &add_item->meta) &&
      tweak_variant_is_equal(&item->default_value, &add_item->default_value)
  );
}

static void client_add_item_pickle_impl(tweak_pickle_add_item *add_item, void *cookie) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = cookie;
  struct tweak_model_impl* model = &client_impl->base.model_impl;

  enum change_item_status {
    TWEAK_APP_ITEM_NO_CHANGE = 0,
    TWEAK_APP_ITEM_CREATED,
    TWEAK_APP_ITEM_VALUE_UPDATED,
  };

  tweak_common_rwlock_write_lock(&model->model_lock);

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  enum change_item_status status;
  tweak_id tweak_id = tweak_model_uri_to_tweak_id_index_lookup(model->index,
                          tweak_variant_string_c_str(&add_item->uri));

  if (tweak_id != TWEAK_INVALID_ID) {
    tweak_item* item = tweak_model_find_item_by_id(model->model, tweak_id);
    if (!is_add_item_correct(add_item, item)) {
      TWEAK_FATAL("Client model is inconsistent. uri = %s id = %" PRIu64,
        tweak_variant_string_c_str(&add_item->uri), add_item->id);
    }

    if (!tweak_variant_is_equal(&item->current_value, &add_item->current_value)) {
      tweak_variant_swap(&item->current_value, &add_item->current_value);
      value = tweak_variant_copy(&item->current_value);
      status = TWEAK_APP_ITEM_VALUE_UPDATED;
    } else {
      status = TWEAK_APP_ITEM_NO_CHANGE;
    }
  } else {
    tweak_id = add_item_to_model(model, add_item);
    if (tweak_id == TWEAK_INVALID_ID) {
      TWEAK_FATAL("Unable to add item %" PRIu64, add_item->id);
    }
    status = TWEAK_APP_ITEM_CREATED;
  }

  tweak_common_rwlock_write_unlock(&model->model_lock);
  wait_context_notify_all(&client_impl->wait_context);

  switch (status) {
  case TWEAK_APP_ITEM_CREATED:
    if (client_impl->client_callbacks.on_new_item) {
      TWEAK_LOG_TRACE("Invoking client callback %p", client_impl->client_callbacks.on_new_item);
      client_impl->client_callbacks.on_new_item(&client_impl->base,
        tweak_id, client_impl->client_callbacks.cookie);
    } else {
      TWEAK_LOG_TRACE("on_new_item is NULL");
    }
    break;
  case TWEAK_APP_ITEM_VALUE_UPDATED:
    TWEAK_LOG_TRACE("Invoking on_current_value_changed");
    if (client_impl->client_callbacks.on_current_value_changed) {
      client_impl->client_callbacks.on_current_value_changed(&client_impl->base,
        tweak_id, &value, client_impl->client_callbacks.cookie);
    } else {
      TWEAK_LOG_TRACE("on_current_value_changed is NULL");
    }
    tweak_variant_destroy(&value);
    break;
  case TWEAK_APP_ITEM_NO_CHANGE:
    break;
  }
}

static void client_change_item_pickle_impl(tweak_pickle_change_item *change, void *cookie) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = cookie;
  struct tweak_model_impl* model = &client_impl->base.model_impl;
  bool emit_change_event = false;
  tweak_id id = TWEAK_INVALID_ID;
  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_common_rwlock_write_lock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, change->id);
  if (item) {
    tweak_variant_swap(&item->current_value, &change->value);
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " updated", change->id);
    id = item->id;
    if (client_impl->client_callbacks.on_current_value_changed) {
      if (!tweak_variant_is_equal(&item->current_value, &change->value)) {
        value = tweak_variant_copy(&item->current_value);
        emit_change_event = true;
      }
    } else {
      TWEAK_LOG_TRACE("on_current_value_changed is NULL");
    }
  } else {
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " isn't found ", change->id);
  }
  tweak_common_rwlock_write_unlock(&model->model_lock);
  wait_context_notify_all(&client_impl->wait_context);
  if (emit_change_event) {
    TWEAK_LOG_TRACE("Invoking on_current_value_changed");
    client_impl->client_callbacks.on_current_value_changed(&client_impl->base,
      id, &value, client_impl->client_callbacks.cookie);
    tweak_variant_destroy(&value);
  }
}

static void client_remove_item_pickle_impl(tweak_pickle_remove_item* remove_item,
  void *cookie)
{
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = cookie;
  struct tweak_model_impl* model = &client_impl->base.model_impl;
  bool item_removed = false;
  tweak_common_rwlock_write_lock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, remove_item->id);
  if (item != NULL) {
    tweak_model_uri_to_tweak_id_index_remove(model->index, tweak_variant_string_c_str(&item->uri));
    tweak_model_remove_item(model->model, remove_item->id);
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " removed", remove_item->id);
    item_removed = true;
  } else {
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " isn't found ", remove_item->id);
  }
  tweak_common_rwlock_write_unlock(&model->model_lock);
  wait_context_notify_all(&client_impl->wait_context);
  if (item_removed && client_impl->client_callbacks.on_item_removed) {
    TWEAK_LOG_TRACE("Invoking on_item_removed");
    client_impl->client_callbacks.on_item_removed(&client_impl->base, remove_item->id,
      client_impl->client_callbacks.cookie);
  }
}

static void io_loop_change(tweak_id tweak_id, void* cookie) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = cookie;
  struct tweak_model_impl* model = &client_impl->base.model_impl;
  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  bool should_push_change = false;
  tweak_common_rwlock_read_lock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, tweak_id);
  if (item != NULL) {
    value = tweak_variant_copy(&item->current_value);
    should_push_change = tweak_app_context_private_is_connected(&client_impl->base);
  } else {
    TWEAK_LOG_WARN("io_loop_change: Unknown item id %" PRIu64 "\n", tweak_id);
  }
  tweak_common_rwlock_read_unlock(&model->model_lock);
  if (should_push_change) {
    TWEAK_LOG_TRACE("Pushing item change for tweak_id = %" PRIu64 " to server", tweak_id);
    tweak_pickle_change_item change = {
      .id = tweak_id,
      .value = value
    };
    tweak_pickle_call_result result =
      tweak_pickle_client_change_item(client_impl->rpc_endpoint, &change);
    if (result != TWEAK_PICKLE_SUCCESS) {
      TWEAK_LOG_TRACE("failed tweak_pickle_server_change_item RPC call on id = %" PRIu64 "\n", tweak_id);
    }
    tweak_variant_destroy(&value);
  }
}

static void client_push_changes(tweak_app_context context, tweak_id tweak_id) {
  TWEAK_LOG_TRACE_ENTRY();
  struct job job = {
    .job_proc = &io_loop_change,
    .tweak_id = tweak_id,
    .cookie = context
  };
  tweak_app_queue_push(context->job_queue, &job);
}

static tweak_app_error_code check_connection_and_clone_current_value(tweak_app_context context,
  tweak_id id, tweak_variant* value)
{
  TWEAK_LOG_TRACE_ENTRY();
  tweak_app_error_code error_code = tweak_app_context_private_item_clone_current_value(context, id, value);
  if (error_code == TWEAK_APP_SUCCESS && !tweak_app_context_private_is_connected(context))
    error_code = TWEAK_APP_SUCCESS_LAST_KNOWN_VALUE;
  return error_code;
}

static tweak_app_error_code replace_current_value(tweak_app_context context,
  tweak_id tweak_id, tweak_variant* value)
{
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_model_impl* model = &context->model_impl;
  bool value_allowed = false;
  tweak_common_rwlock_read_lock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, tweak_id);
  value_allowed = (item != NULL) && tweak_app_context_private_check_value_compatibility(&item->current_value, value);
  tweak_common_rwlock_read_unlock(&model->model_lock);
  if ((item != NULL) && value_allowed) {
    return tweak_app_context_private_is_connected(context)
      ? tweak_app_context_private_item_replace_current_value(context, tweak_id, value)
      : TWEAK_APP_PEER_DISCONNECTED;
  } else if ((item != NULL) && !value_allowed) {
    TWEAK_LOG_WARN("Type mismatch error while updating item with tweak_id = %" PRIu64 "", tweak_id);
    return TWEAK_APP_TYPE_MISMATCH;
  } else {
    TWEAK_LOG_WARN("item with id = %" PRIu64 " isn't found ", tweak_id);
    return TWEAK_APP_ITEM_NOT_FOUND;
  }
}

static void client_destroy_context(struct tweak_app_context_base* context) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = (struct tweak_app_context_client_impl*)context;
  if (client_impl->base.job_queue) {
    tweak_app_queue_stop(client_impl->base.job_queue);
  }

  if (client_impl->rpc_endpoint) {
    tweak_pickle_destroy_client_endpoint(client_impl->rpc_endpoint);
    client_impl->rpc_endpoint = NULL;
  }

  call_remove_callback_for_all_items(client_impl, client_impl->base.model_impl.index);

  tweak_app_context_private_destroy_base(&client_impl->base);
  destroy_wait_context(&client_impl->wait_context);
  free(context);
}

tweak_app_client_context tweak_app_create_client_context(const char *context_type, const char *params,
  const char *uri, const tweak_app_client_callbacks* client_callbacks)
{
  TWEAK_LOG_TRACE_ENTRY("context_type = %s, params = %s, uri = %s, client_callbacks = %p",
    context_type, params, uri, client_callbacks);
  struct tweak_app_context_client_impl* client_impl = calloc(1, sizeof(*client_impl));
  if (!client_impl) {
    TWEAK_LOG_ERROR("calloc() returned NULL");
    return NULL;
  }

  if (!init_wait_context(&client_impl->wait_context)) {
    TWEAK_LOG_ERROR("init_wait_context() returned error");
    free(client_impl);
    return NULL;
  }

  tweak_pickle_client_descriptor client_descriptor = {
    .context_type = context_type,
    .params = params,
    .uri = uri,
    .skeleton = {
      .announce_features_listener = {
        .callback = &announce_features_impl,
        .cookie = client_impl
      },
      .connection_state_listener = {
        .callback = &client_connection_state_pickle_impl,
        .cookie = client_impl
      },
      .add_item_listener = {
        .callback = &client_add_item_pickle_impl,
        .cookie = client_impl
      },
      .change_item_listener = {
        .callback = &client_change_item_pickle_impl,
        .cookie = client_impl
      },
      .remove_item_listener = {
        .callback = client_remove_item_pickle_impl,
        .cookie = client_impl
      }
    }
  };

  if (!tweak_app_context_private_initialize_base(&client_impl->base, TWEAK_APP_CLIENT_QUEUE_SIZE)) {
    TWEAK_LOG_ERROR("tweak_app_context_private_initialize_base() fail");
    goto destroy_context;
  }

  client_impl->base.clone_current_value_proc = &check_connection_and_clone_current_value;
  client_impl->base.replace_current_value_proc = &replace_current_value;
  client_impl->base.push_changes_proc = &client_push_changes;
  client_impl->base.destroy_context = &client_destroy_context;

  if (client_callbacks) {
    client_impl->client_callbacks = *client_callbacks;
  }

  client_impl->rpc_endpoint = tweak_pickle_create_client_endpoint(&client_descriptor);
  if (!client_impl->rpc_endpoint) {
    TWEAK_LOG_ERROR("tweak_pickle_create_client_endpoint() fail");
    goto destroy_context;
  }

  return &client_impl->base;

destroy_context:
  client_destroy_context(&client_impl->base);
  return NULL;
}

static bool check_wait_condition(struct tweak_app_context_client_impl* context_client_impl,
  const char** uris, tweak_id* tweak_ids, size_t uris_size)
{
  if (!tweak_app_context_private_is_connected(&context_client_impl->base)) {
    return false;
  }
  bool result = true;
  struct tweak_model_impl* model = &context_client_impl->base.model_impl;
  tweak_common_rwlock_read_lock(&model->model_lock);
  for (size_t ix = 0; ix < uris_size; ++ix) {
    tweak_id tweak_id = tweak_model_uri_to_tweak_id_index_lookup(model->index, uris[ix]);
    if (tweak_id == TWEAK_INVALID_ID) {
      result = false;
      break;
    }
    if (tweak_ids != NULL) {
      tweak_ids[ix] = tweak_id;
    }
  }
  tweak_common_rwlock_read_unlock(&model->model_lock);
  return result;
}

struct predicate_proc_context {
  struct tweak_app_context_client_impl* context_client_impl;
  const char** uris;
  size_t uris_size;
};

static bool predicate_proc(void* cookie) {
  struct predicate_proc_context* context = (struct predicate_proc_context*) cookie;
  return check_wait_condition(context->context_client_impl,
    context->uris, NULL, context->uris_size);
}

tweak_app_error_code tweak_app_client_wait_uris(tweak_app_client_context client_context,
  const char** uris, size_t uris_size, tweak_id* tweak_ids, uint64_t timeout_millis)
{
  if (!client_context) {
    TWEAK_FATAL("Invalid argument: client_context is NULL");
  }
  if (!uris) {
    TWEAK_FATAL("uris is NULL");
  }

  struct tweak_app_context_client_impl* context_client_impl = (struct tweak_app_context_client_impl*)client_context;

  struct predicate_proc_context predicate_proc_context = {
    .context_client_impl = context_client_impl,
    .uris = uris,
    .uris_size = uris_size
  };

  tweak_common_thread_error wait_result;
  tweak_common_mutex_lock(&context_client_impl->wait_context.lock);
  wait_result = tweak_common_cond_timed_wait_with_pred(&context_client_impl->wait_context.cond,
    &context_client_impl->wait_context.lock, timeout_millis,
    &predicate_proc, &predicate_proc_context);
  tweak_common_mutex_unlock(&context_client_impl->wait_context.lock);

  switch (wait_result) {
  case TWEAK_COMMON_THREAD_SUCCESS:
    return check_wait_condition(context_client_impl, uris, tweak_ids, uris_size)
      ? TWEAK_APP_SUCCESS
      : TWEAK_APP_TIMEOUT;
  default:
  case TWEAK_COMMON_THREAD_FAILURE:
    TWEAK_FATAL("Platform specific threading error in tweak_common_cond_timed_wait_with_pred()");
  case TWEAK_COMMON_THREAD_TIMEOUT:
    return TWEAK_APP_TIMEOUT;
  }
}
