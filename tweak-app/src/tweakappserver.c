/**
 * @file tweakappserver.c
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

#include <tweak2/appserver.h>
#include <tweak2/log.h>
#include <tweak2/util.h>
#include <tweak2/pickle_server.h>

#include "tweakappinternal.h"
#include "tweakappqueue.h"
#include "tweakmodel.h"
#include "tweakmodel_uri_to_tweak_id_index.h"
#include "tweakappfeatures.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

enum { TWEAK_APP_SERVER_QUEUE_SIZE = 100 };

static void server_push_changes(tweak_app_context context, tweak_id tweak_id);

struct tweak_app_context_server_impl {
  struct tweak_app_context_base base;
  tweak_app_server_callbacks server_callbacks;
  tweak_pickle_server_endpoint rpc_endpoint;
  bool features_announced;
};

static bool check_value_allowed(struct tweak_app_features* features, const tweak_variant* value) {
  switch (value->type) {
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
    return features->vectors;
  default:
    return true;
  }
}

static void server_destroy_context(struct tweak_app_context_base* context) {
  TWEAK_LOG_TRACE_ENTRY("context = %p", context);
  struct tweak_app_context_server_impl* server_impl = (struct tweak_app_context_server_impl*)context;
  if (server_impl->base.job_queue) {
    tweak_app_queue_stop(server_impl->base.job_queue);
  }
  if (server_impl->rpc_endpoint) {
    tweak_pickle_destroy_server_endpoint(server_impl->rpc_endpoint);
  }
  tweak_app_context_private_destroy_base(&server_impl->base);
  free(context);
}

static bool subscribe_walk_proc(const char *uri, tweak_id tweak_id, void* cookie) {
  (void)uri;
  TWEAK_LOG_TRACE_ENTRY("uri = %s, tweak_id = %" PRId64 ", cookie = %p", uri, tweak_id, cookie);
  assert(tweak_id != TWEAK_INVALID_ID);
  struct tweak_app_context_server_impl* server_impl = (struct tweak_app_context_server_impl*) cookie;
  tweak_item* item = tweak_model_find_item_by_id(server_impl->base.model_impl.model, tweak_id);
  bool value_allowed = check_value_allowed(&server_impl->base.remote_peer_features, &item->current_value);
  if (!value_allowed) {
    TWEAK_LOG_WARN("Remote endpoint doesn't support item : \"%s\"", tweak_variant_string_c_str(&item->uri));
  }
  if (item != NULL) {
    bool item_is_compatible =
      tweak_app_features_check_type_compatibility(&server_impl->base.remote_peer_features, item->variant_type);
    if (item_is_compatible) {
      tweak_pickle_add_item pickle_add_item = {
        .id = item->id,
        .uri = item->uri,
        .meta = item->meta,
        .description = item->description,
        .default_value = item->default_value,
        .current_value = item->current_value,
      };
      tweak_pickle_call_result call_result =
        tweak_pickle_server_add_item(server_impl->rpc_endpoint, &pickle_add_item);
      if (call_result == TWEAK_PICKLE_SUCCESS) {
        return true;
      }
      TWEAK_LOG_WARN("tweak_pickle_server_add_item: RPC call failed with code %d", call_result);
    } else {
      TWEAK_LOG_WARN("Skipping item with uri = \"%s\" with type %d not supported by remote peer",
        tweak_variant_string_c_str(&item->uri), item->variant_type);
      return true;
    }
  } else {
    TWEAK_LOG_WARN("model index is inconsistent, Unknown tweak_id = %" PRIu64 "", tweak_id);
  }
  return false;
}

static void io_loop_subscribe(tweak_id tweak_id, void* cookie) {
  (void) tweak_id;
  TWEAK_LOG_TRACE_ENTRY("tweak_id = %" PRId64 ", cookie = %p", tweak_id, cookie);
  struct tweak_app_context_server_impl* server_impl = cookie;
  if (server_impl->features_announced) {
    struct tweak_app_features default_features = { 0 };
    tweak_app_features_init_default(&default_features);

    tweak_variant_string session_features = tweak_app_features_to_json(&default_features);
    tweak_pickle_features features = { .features = session_features };

    tweak_pickle_call_result call_result =
      tweak_pickle_server_announce_features(server_impl->rpc_endpoint, &features);

    tweak_variant_destroy_string(&session_features);
    if (call_result != TWEAK_PICKLE_SUCCESS) {
      TWEAK_LOG_WARN("tweak_pickle_server_announce_features() returned error %d", call_result);
      return;
    }
    server_impl->features_announced = false;
  }

  tweak_app_context context = &server_impl->base;
  tweak_common_rwlock_read_lock(&context->model_impl.model_lock);
  bool walk_success = tweak_model_uri_to_tweak_id_index_walk(context->model_impl.index,
    &subscribe_walk_proc, context);
  if (walk_success) {
    tweak_app_context_private_set_connected(context, true);
    TWEAK_LOG_TRACE("Client connected. Updates shall be propagated to client.");
  } else {
    TWEAK_LOG_WARN("Can't handle subscribe request, status is still offline");
  }
  tweak_common_rwlock_read_unlock(&context->model_impl.model_lock);
}

static void push_subscribe(tweak_app_context context) {
  TWEAK_LOG_TRACE_ENTRY("context = %p", context);
  struct job job = {
    .job_proc = &io_loop_subscribe,
    .tweak_id = TWEAK_INVALID_ID,
    .cookie = context
  };
  tweak_app_queue_push(context->job_queue, &job);
}

static void log_remote_features_change(const struct tweak_app_features *old,
                                       const struct tweak_app_features *new)
{
  TWEAK_LOG_TRACE_ENTRY();
  if (old->vectors != new->vectors && !new->vectors) {
    TWEAK_LOG_WARN("Vectors option became disabled");
  }
}

static void announce_features_impl(tweak_pickle_features* announce_features,
  void *cookie)
{
  TWEAK_LOG_TRACE_ENTRY("announce_features = \"%s\", cookie = %p",
    tweak_variant_string_c_str(&announce_features->features), cookie);
  struct tweak_app_context_server_impl* server_impl = cookie;

  struct tweak_app_features features = { 0 };
  bool parse_success = tweak_app_features_from_json(&announce_features->features, &features);

  if (!parse_success) {
    tweak_app_features_init_default(&features);
    TWEAK_LOG_WARN("Can't parse server features, using scalar tweaks only");
  }

  tweak_common_mutex_lock(&server_impl->base.conn_state_lock);
  features = tweak_app_features_combine(
          &server_impl->base.remote_peer_features,
          &features);

  log_remote_features_change(&server_impl->base.remote_peer_features, &features);

  server_impl->base.remote_peer_features = features;
  server_impl->features_announced = parse_success;
  tweak_common_mutex_unlock(&server_impl->base.conn_state_lock);
}

static void subscribe_tweak_pickle_impl(tweak_pickle_subscribe* subscribe, void *cookie) {
  (void)subscribe;
  TWEAK_LOG_TRACE_ENTRY("subscribe = %p, cookie = %p", subscribe, cookie);
  push_subscribe(cookie);
}

static void change_item_pickle_impl(tweak_pickle_change_item *change, void *cookie) {
  TWEAK_LOG_TRACE_ENTRY("change = %p, cookie = %p", change, cookie);
  struct tweak_app_context_server_impl* server_impl = cookie;
  struct tweak_model_impl* model = &server_impl->base.model_impl;
  bool emit_change_event = false;
  tweak_id id = TWEAK_INVALID_ID;
  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_common_rwlock_write_lock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, change->id);
  if (item != NULL &&
    tweak_app_context_private_check_value_compatibility(&item->current_value, &change->value))
  {
    tweak_variant_swap(&item->current_value, &change->value);
    TWEAK_LOG_TRACE("Item with tweak_id = %" PRId64 " has been updated", change->id);
    id = item->id;

    bool changed = !tweak_variant_is_equal(&item->current_value, &change->value);

    if (changed && server_impl->server_callbacks.on_current_value_changed) {
      value = tweak_variant_copy(&item->current_value);
      emit_change_event = true;
    }
  } else {
    TWEAK_LOG_WARN("Ignored change request: Unknown tweak_id =  %" PRIu64 "", item->id);
  }
  tweak_common_rwlock_write_unlock(&model->model_lock);
  if (emit_change_event) {
    TWEAK_LOG_TRACE("Invoking on_current_value_changed callback on tweak_id = %" PRId64 "", id);
    server_impl->server_callbacks.on_current_value_changed(&server_impl->base,
      id, &value, server_impl->server_callbacks.cookie);
    tweak_variant_destroy(&value);
  }

  server_push_changes(&server_impl->base, id);
}

static void connection_state_pickle_impl(tweak_pickle_connection_state connection_state,
  void *cookie)
{
  TWEAK_LOG_TRACE_ENTRY("connection_state = %d, cookie = %p",
    connection_state, cookie);
  struct tweak_app_context_server_impl* server_impl = cookie;
  switch (connection_state) {
  case TWEAK_PICKLE_CONNECTED:
    TWEAK_LOG_TRACE("Client connected.");
    tweak_common_mutex_lock(&server_impl->base.conn_state_lock);
    tweak_app_features_init_default(&server_impl->base.remote_peer_features);
    tweak_common_mutex_unlock(&server_impl->base.conn_state_lock);
    break;
  case TWEAK_PICKLE_DISCONNECTED:
    TWEAK_LOG_TRACE("Client disconnected.");
    tweak_app_context_private_set_connected(&server_impl->base, false);
    break;
  default:
    TWEAK_LOG_ERROR("Unexpected enum value: %u", connection_state);
    break;
  }
}

static void clean_pickle_add_item(tweak_pickle_add_item* add_item) {
  TWEAK_LOG_TRACE_ENTRY("add_item = %p", add_item);
  tweak_variant_destroy_string(&add_item->uri);
  tweak_variant_destroy_string(&add_item->meta);
  tweak_variant_destroy_string(&add_item->description);
  tweak_variant_destroy(&add_item->default_value);
  tweak_variant_destroy(&add_item->current_value);
}

static void io_loop_append(tweak_id tweak_id, void* cookie) {
  TWEAK_LOG_TRACE_ENTRY("tweak_id = %" PRIu64 ", cookie = %p", tweak_id, cookie);
  struct tweak_app_context_server_impl* server_impl = cookie;
  struct tweak_model_impl* model = &server_impl->base.model_impl;
  tweak_item* item;
  tweak_pickle_add_item pickle_add_item = { 0 };
  tweak_common_rwlock_write_lock(&model->model_lock);
  item = tweak_model_find_item_by_id(model->model, tweak_id);
  if (item != NULL) {
    pickle_add_item.id = item->id;
    pickle_add_item.uri = tweak_variant_string_copy(&item->uri);
    pickle_add_item.meta = tweak_variant_string_copy(&item->meta);
    pickle_add_item.description = tweak_variant_string_copy(&item->description);
    pickle_add_item.default_value = tweak_variant_copy(&item->default_value);
    pickle_add_item.current_value = tweak_variant_copy(&item->current_value);
  } else {
    TWEAK_LOG_WARN("execute_add_item_task: Unknown tweak_id = %" PRIu64 "", tweak_id);
  }
  tweak_common_rwlock_write_unlock(&model->model_lock);
  if (item) {
    TWEAK_LOG_TRACE("Propagating add_item request to client = %" PRIu64 "", item->id);
    tweak_pickle_call_result call_result =
      tweak_pickle_server_add_item(server_impl->rpc_endpoint, &pickle_add_item);
    if (call_result != TWEAK_PICKLE_SUCCESS) {
       TWEAK_LOG_WARN("failed tweak_pickle_server_add_item RPC call on id = %" PRIu64 "", tweak_id);
    }
    clean_pickle_add_item(&pickle_add_item);
  }
}

static void push_append(tweak_app_context context, tweak_id tweak_id) {
  TWEAK_LOG_TRACE_ENTRY("context = %p, tweak_id = %" PRIu64 "", context, tweak_id);
  struct job job = {
    .tweak_id = tweak_id,
    .job_proc = &io_loop_append,
    .cookie = context,
  };
  tweak_app_queue_push(context->job_queue, &job);
}

static void io_loop_change(tweak_id tweak_id, void* cookie) {
  TWEAK_LOG_TRACE_ENTRY("tweak_id = %" PRIu64 ", cookie = %p", tweak_id, cookie);
  struct tweak_app_context_server_impl* server_impl = cookie;
  struct tweak_model_impl* model = &server_impl->base.model_impl;
  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  bool should_push_change = false;
  tweak_common_rwlock_read_lock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, tweak_id);
  if (item != NULL) {
    value = tweak_variant_copy(&item->current_value);
    should_push_change = tweak_app_context_private_is_connected(&server_impl->base);
  } else {
    TWEAK_LOG_WARN("change_item_callback: Unknown tweak_id = %" PRIu64 "\n", tweak_id);
  }
  tweak_common_rwlock_read_unlock(&model->model_lock);
  if (should_push_change) {
    TWEAK_LOG_TRACE("Propagating change_item request to client = %" PRIu64 "", item->id);
    tweak_pickle_change_item change = {
      .id = tweak_id,
      .value = value
    };
    tweak_pickle_call_result result =
      tweak_pickle_server_change_item(server_impl->rpc_endpoint, &change);
    if (result != TWEAK_PICKLE_SUCCESS) {
      TWEAK_LOG_WARN("failed tweak_pickle_server_change_item RPC call on id = %" PRIu64 "", tweak_id);
    }
  }
  tweak_variant_destroy(&value);
}

static void server_push_changes(tweak_app_context context, tweak_id tweak_id) {
  TWEAK_LOG_TRACE_ENTRY("context = %p, tweak_id = %" PRIu64 "", context, tweak_id);
  struct job job = {
    .job_proc = &io_loop_change,
    .tweak_id = tweak_id,
    .cookie = context
  };
  tweak_app_queue_push(context->job_queue, &job);
}

static void io_loop_remove(tweak_id tweak_id, void* cookie) {
  TWEAK_LOG_TRACE_ENTRY("tweak_id = %" PRIu64 ", cookie = %p", tweak_id, cookie);
  struct tweak_app_context_server_impl* server_impl = cookie;
  tweak_pickle_remove_item remove_item = {
    .id = tweak_id
  };
  tweak_pickle_call_result call_result =
    tweak_pickle_server_remove_item(server_impl->rpc_endpoint, &remove_item);
  if (call_result != TWEAK_PICKLE_SUCCESS) {
    TWEAK_LOG_WARN("failed tweak_pickle_server_add_item RPC call on id = %" PRIu64 "", tweak_id);
  }
}

static void push_remove(tweak_app_context context, tweak_id tweak_id) {
  TWEAK_LOG_TRACE_ENTRY("context = %p, tweak_id = %" PRIu64 "", context, tweak_id);
  struct job job = {
    .job_proc = &io_loop_remove,
    .tweak_id = tweak_id,
    .cookie = context
  };
  tweak_app_queue_push(context->job_queue, &job);
}

tweak_app_server_context tweak_app_create_server_context(const char *context_type, const char *params,
  const char *uri, const tweak_app_server_callbacks* server_callbacks)
{
  TWEAK_LOG_TRACE_ENTRY("context_type = %s, params = %s, uri = %s, server_callbacks = %p",
    context_type, params, uri, server_callbacks);

  struct tweak_app_context_server_impl* server_impl = calloc(1, sizeof(*server_impl));
  if (!server_impl) {
    return NULL;
  }

  tweak_pickle_server_descriptor server_descriptor = {
    .context_type = context_type,
    .params = params,
    .uri = uri,
    .skeleton = {
      .announce_features_listener = {
        .callback = &announce_features_impl,
        .cookie = server_impl
      },
      .subscribe_listener = {
        .callback = &subscribe_tweak_pickle_impl,
        .cookie = server_impl
      },
      .change_item_listener = {
        .callback = &change_item_pickle_impl,
        .cookie = server_impl
      },
      .connection_state_listener = {
        .callback = &connection_state_pickle_impl,
        .cookie = server_impl
      }
    }
  };

  if (!tweak_app_context_private_initialize_base(&server_impl->base, TWEAK_APP_SERVER_QUEUE_SIZE)) {
    goto destroy_context;
  }

  server_impl->base.clone_current_value_proc = &tweak_app_context_private_item_clone_current_value;
  server_impl->base.replace_current_value_proc = &tweak_app_context_private_item_replace_current_value;
  server_impl->base.push_changes_proc = &server_push_changes;
  server_impl->base.destroy_context = &server_destroy_context;

  if (server_callbacks) {
    server_impl->server_callbacks = *server_callbacks;
  }

  server_impl->rpc_endpoint = tweak_pickle_create_server_endpoint(&server_descriptor);
  if (!server_impl->rpc_endpoint) {
    goto destroy_context;
  }

  return &server_impl->base;

destroy_context:
  server_destroy_context(&server_impl->base);
  return NULL;
}

tweak_id tweak_app_server_add_item(tweak_app_server_context server_context,
  const char* uri, const char* description, const char* meta,
  tweak_variant* initial_value, void* item_cookie)
{
  TWEAK_LOG_TRACE_ENTRY("server_context = %p, uri = %s, description = %s, meta = %s,"
    " initial_value = %p, item_cookie = %p", server_context, uri, description, meta,
    initial_value, item_cookie);

  struct tweak_app_context_server_impl* server_impl =
    (struct tweak_app_context_server_impl*)server_context;
  struct tweak_model_impl* model = &server_impl->base.model_impl;

  tweak_variant_string uri0 = TWEAK_VARIANT_STRING_EMPTY;
  tweak_assign_string(&uri0, uri);
  tweak_variant_string description0 = TWEAK_VARIANT_STRING_EMPTY;
  tweak_assign_string(&description0, description);
  tweak_variant_string meta0 = TWEAK_VARIANT_STRING_EMPTY;
  tweak_assign_string(&meta0, meta);
  tweak_variant default_value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_swap(&default_value, initial_value);
  tweak_variant current_value = tweak_variant_copy(&default_value);
  bool item_is_compatible =
    tweak_app_features_check_type_compatibility(&server_context->remote_peer_features, current_value.type);

  bool should_push_change = false;
  tweak_id tweak_id = TWEAK_INVALID_ID;

  tweak_model_error_code model_error_code;
  tweak_model_index_result index_result;

  tweak_common_rwlock_write_lock(&model->model_lock);
  if (tweak_model_uri_to_tweak_id_index_lookup(model->index, uri) != TWEAK_INVALID_ID) {
    goto error;
  }

  tweak_id = tweak_common_genid();
  model_error_code = tweak_model_create_item(model->model,
    tweak_id, &uri0, &description0, &meta0, &default_value, &current_value, item_cookie);
  if (model_error_code != TWEAK_MODEL_SUCCESS) {
    TWEAK_LOG_ERROR("tweak_app_server_add_item: tweak_model_create_item failed with error code: %d",
      model_error_code);
    tweak_id = TWEAK_INVALID_ID;
    goto error;
  }

  index_result = tweak_model_uri_to_tweak_id_index_insert(model->index, uri, tweak_id);

  if (index_result != TWEAK_MODEL_INDEX_SUCCESS) {
    TWEAK_LOG_ERROR("tweak_app_server_add_item: tweak_model_uri_to_tweak_id_index_insert: %d",
      model_error_code);
    tweak_model_remove_item(model->model, tweak_id);
    tweak_id = TWEAK_INVALID_ID;
    goto error;
  }

  should_push_change = item_is_compatible && tweak_app_context_private_is_connected(server_context);
error:
  tweak_common_rwlock_write_unlock(&model->model_lock);

  tweak_variant_destroy_string(&uri0);
  tweak_variant_destroy_string(&description0);
  tweak_variant_destroy_string(&meta0);
  tweak_variant_destroy(&default_value);
  tweak_variant_destroy(&current_value);

  if (should_push_change) {
    TWEAK_LOG_TRACE("Pushing add_item request to client");
    push_append(server_context, tweak_id);
  }
  if (tweak_id == TWEAK_INVALID_ID) {
    TWEAK_LOG_TRACE("Can't add new item");
  } else {
    TWEAK_LOG_TRACE("tweak_app_server_add_item successful, new tweak_id = %" PRId64 "", tweak_id);
  }
  return tweak_id;
}

void* tweak_app_item_get_cookie(tweak_app_server_context server_context, tweak_id id) {
  TWEAK_LOG_TRACE_ENTRY("server_context = %p, tweak_id = %" PRIu64 "", server_context, id);
  void* item_cookie;
  tweak_item* item = NULL;
  tweak_common_rwlock_read_lock(&server_context->model_impl.model_lock);
  item = tweak_model_find_item_by_id(server_context->model_impl.model, id);
  item_cookie = item != NULL ? item->item_cookie : NULL;
  tweak_common_rwlock_read_unlock(&server_context->model_impl.model_lock);
  TWEAK_LOG_TRACE("Item with tweak_id = %" PRIu64 " has cookie = %p", id, item_cookie);
  return item_cookie;
}

bool tweak_app_server_remove_item(tweak_app_server_context server_context, tweak_id id) {
  TWEAK_LOG_TRACE_ENTRY("server_context = %p, tweak_id = %" PRIu64 "", server_context, id);
  bool result = false;
  struct tweak_app_context_server_impl* server_impl =
    (struct tweak_app_context_server_impl*)server_context;
  struct tweak_model_impl* model = &server_impl->base.model_impl;

  bool should_push_change = false;
  tweak_common_rwlock_write_lock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, id);
  if (item) {
    bool item_is_compatible =
      tweak_app_features_check_type_compatibility(&server_context->remote_peer_features, item->variant_type);
    tweak_model_uri_to_tweak_id_index_remove(model->index,
    tweak_variant_string_c_str(&item->uri));
    tweak_model_remove_item(model->model, id);
    result = true;
    should_push_change = item_is_compatible && tweak_app_context_private_is_connected(server_context);
  } else {
    TWEAK_LOG_WARN("change_item_callback: Unknown tweak_id = %" PRIu64 "\n", id);
  }
  tweak_common_rwlock_write_unlock(&model->model_lock);
  if (should_push_change) {
    TWEAK_LOG_TRACE("Pushing remove_item request to client");
    push_remove(server_context, id);
  }
  if (result) {
    TWEAK_LOG_TRACE("Removed item with tweak_id = %" PRIu64 "", id);
  } else {
    TWEAK_LOG_TRACE("Can't remove, item with tweak_id = %" PRIu64 " hasn't been found", id);
  }
  return result;
}
