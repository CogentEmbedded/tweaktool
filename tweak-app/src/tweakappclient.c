/**
 * @file tweakappclient.c
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

#include <tweak2/appclient.h>
#include <tweak2/log.h>
#include <tweak2/pickle_client.h>

#include "tweakappinternal.h"
#include "tweakappqueue.h"
#include "tweakmodel.h"
#include "tweakmodel_uri_to_tweak_id_index.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>

enum { TWEAK_APP_CLIENT_QUEUE_SIZE = 100 };

struct tweak_app_context_client_impl {
  struct tweak_app_context_base base;
  tweak_app_client_callbacks client_callbacks;
  tweak_pickle_client_endpoint rpc_endpoint;
};

static void io_loop_subscribe(tweak_id tweak_id, void* cookie) {
  TWEAK_LOG_TRACE_ENTRY("tweak_id = %" PRId64 " , cookie = %p", cookie);
  struct tweak_app_context_client_impl* client_impl = cookie;
  tweak_pickle_subscribe subscribe = {};
  tweak_pickle_call_result result = tweak_pickle_client_subscribe(client_impl->rpc_endpoint, &subscribe);
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

static void call_remove_callback_for_all_items(struct tweak_app_context_client_impl* client_impl) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_model_impl* model = &client_impl->base.model_impl;
  if (client_impl->client_callbacks.on_item_removed) {
    TWEAK_LOG_TRACE("Invoking on_item_removed for all items");
    tweak_model_uri_to_tweak_id_index_walk(model->index,
      &invoke_item_removed_callback, client_impl);
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

  call_remove_callback_for_all_items(client_impl);

  struct tweak_model_impl* model = &client_impl->base.model_impl;
  tweak_model old_model = model->model;
  tweak_model_uri_to_tweak_id_index old_index = model->index;

  pthread_rwlock_wrlock(&model->model_lock);
  model->model = new_model;
  model->index = new_index;
  pthread_rwlock_unlock(&model->model_lock);

  tweak_model_destroy(old_model);
  tweak_model_uri_to_tweak_id_index_destroy(old_index);
}

static void client_connection_state_pickle_impl(tweak_pickle_connection_state connection_state,
  void *cookie)
{
  TWEAK_LOG_TRACE_ENTRY("connection_state = %d, cookie = %p", connection_state, cookie);
  struct tweak_app_context_client_impl* client_impl = (struct tweak_app_context_client_impl*) cookie;
  switch (connection_state) {
  case TWEAK_PICKLE_CONNECTED:
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
}

static tweak_model_index_result update_index(tweak_model_uri_to_tweak_id_index index,
  const char *old_uri, const char *uri, tweak_id tweak_id)
{
  TWEAK_LOG_TRACE_ENTRY();
  uint64_t bound_id = tweak_model_uri_to_tweak_id_index_lookup(index, old_uri);
  tweak_model_index_result rv;
  if (bound_id == tweak_id) {
    if (strcmp(old_uri, uri) == 0) {
      rv = TWEAK_MODEL_INDEX_SUCCESS;
    } else {
      tweak_model_index_result index_result = tweak_model_uri_to_tweak_id_index_insert(index, uri, tweak_id);
      if (index_result == TWEAK_MODEL_INDEX_SUCCESS) {
        index_result = tweak_model_uri_to_tweak_id_index_remove(index, old_uri);
        assert(index_result == TWEAK_MODEL_INDEX_SUCCESS);
      }
      rv = index_result;
    }
  } else if (bound_id == TWEAK_INVALID_ID) {
    rv = tweak_model_uri_to_tweak_id_index_insert(index, uri, tweak_id);
  }
  if (rv == TWEAK_MODEL_INDEX_SUCCESS) {
    TWEAK_LOG_TRACE("Index entry for item with tweak_id = %" PRId64 " has been updated", tweak_id);
  } else {
    TWEAK_LOG_TRACE("Index entry for item with tweak_id = %" PRId64 " hasn't been updated,"
      " error_code = %d", tweak_id, rv);
  }
  return rv;
}

static tweak_id add_item_to_model(struct tweak_model_impl* model, tweak_pickle_add_item *add_item) {
  TWEAK_LOG_TRACE_ENTRY();
  tweak_model_error_code model_error_code = tweak_model_create_item(model->model,
    add_item->tweak_id, &add_item->uri, &add_item->description, &add_item->meta, &add_item->default_value,
    &add_item->current_value, NULL);

  if (model_error_code != TWEAK_MODEL_SUCCESS) {
    TWEAK_LOG_TRACE("Can't handle add_item request for tweak_id = %" PRIu64 "\n", add_item->tweak_id);
    return TWEAK_INVALID_ID;
  }

  if (tweak_model_uri_to_tweak_id_index_insert(model->index,
    tweak_variant_string_c_str(&add_item->uri), add_item->tweak_id) != TWEAK_MODEL_INDEX_SUCCESS)
  {
    TWEAK_LOG_TRACE("Can't handle add_item request for tweak_id = %" PRIu64 "\n", add_item->tweak_id);
    tweak_model_remove_item(model->model, add_item->tweak_id);
    return TWEAK_INVALID_ID;
  }
  return add_item->tweak_id;
}

static tweak_id update_item_in_model(struct tweak_model_impl* model,
  tweak_item* item, tweak_pickle_add_item *add_item)
{
  TWEAK_LOG_TRACE_ENTRY();
  tweak_model_index_result result = update_index(model->index,
    tweak_variant_string_c_str(&item->uri), tweak_variant_string_c_str(&add_item->uri),
    add_item->tweak_id);

  if (result == TWEAK_MODEL_INDEX_SUCCESS) {
    tweak_variant_swap_string(&item->uri, &add_item->uri);
    tweak_variant_swap_string(&item->description, &add_item->description);
    tweak_variant_swap_string(&item->meta, &add_item->meta);
    tweak_variant_swap(&item->current_value, &add_item->current_value);
    tweak_variant_swap(&item->default_value, &add_item->default_value);
    item->current_value.type = add_item->current_value.type;
    return add_item->tweak_id;
  } else {
    return TWEAK_INVALID_ID;
  }
}

static void client_add_item_pickle_impl(tweak_pickle_add_item *add_item, void *cookie) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = cookie;
  struct tweak_model_impl* model = &client_impl->base.model_impl;

  pthread_rwlock_wrlock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, add_item->tweak_id);
  tweak_id tweak_id = item
    ? update_item_in_model(model, item, add_item)
    : add_item_to_model(model, add_item);
  pthread_rwlock_unlock(&model->model_lock);

  if (tweak_id != TWEAK_INVALID_ID && client_impl->client_callbacks.on_new_item) {
    TWEAK_LOG_TRACE("Invoking client callback %p", client_impl->client_callbacks.on_new_item);
    client_impl->client_callbacks.on_new_item(&client_impl->base,
      tweak_id, client_impl->client_callbacks.cookie);
  } else {
    TWEAK_LOG_TRACE("on_new_item is NULL");
  }
}

static void client_change_item_pickle_impl(tweak_pickle_change_item *change, void *cookie) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = cookie;
  struct tweak_model_impl* model = &client_impl->base.model_impl;
  bool emit_change_event = false;
  tweak_id id;
  tweak_variant value = {};
  pthread_rwlock_wrlock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, change->tweak_id);
  if (item) {
    tweak_variant_swap(&item->current_value, &change->value);
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " updated", change->tweak_id);
    id = item->id;
    if (client_impl->client_callbacks.on_current_value_changed) {
      value = tweak_variant_copy(&item->current_value);
      emit_change_event = true;
    } else {
      TWEAK_LOG_TRACE("on_current_value_changed is NULL");
    }
  } else {
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " isn't found ", change->tweak_id);
  }
  pthread_rwlock_unlock(&model->model_lock);
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
  pthread_rwlock_wrlock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, remove_item->tweak_id);
  if (item != NULL) {
    tweak_model_uri_to_tweak_id_index_remove(model->index, tweak_variant_string_c_str(&item->uri));
    tweak_model_remove_item(model->model, remove_item->tweak_id);
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " removed", remove_item->tweak_id);
    item_removed = true;
  } else {
    TWEAK_LOG_TRACE("item with id = %" PRIu64 " isn't found ", remove_item->tweak_id);
  }
  pthread_rwlock_unlock(&model->model_lock);
  if (item_removed && client_impl->client_callbacks.on_item_removed) {
    TWEAK_LOG_TRACE("Invoking on_item_removed");
    client_impl->client_callbacks.on_item_removed(&client_impl->base, remove_item->tweak_id,
      client_impl->client_callbacks.cookie);
  }
}

static void io_loop_change(tweak_id tweak_id, void* cookie) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = cookie;
  struct tweak_model_impl* model = &client_impl->base.model_impl;
  tweak_variant value = {};
  bool should_push_change = false;
  pthread_rwlock_rdlock(&model->model_lock);
  tweak_item* item = tweak_model_find_item_by_id(model->model, tweak_id);
  if (item != NULL) {
    value = tweak_variant_copy(&item->current_value);
    should_push_change = tweak_app_context_private_is_connected(&client_impl->base);
  } else {
    TWEAK_LOG_WARN("io_loop_change: Unknown item id %" PRIu64 "\n", tweak_id);
  }
  pthread_rwlock_unlock(&model->model_lock);
  if (should_push_change) {
    TWEAK_LOG_TRACE("Pushing item change for tweak_id = %" PRIu64 " to server", tweak_id);
    tweak_pickle_change_item change = {
      .tweak_id = tweak_id,
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

static tweak_app_error_code check_connection_and_replace_current_value(tweak_app_context context,
  tweak_id tweak_id, tweak_variant* value)
{
  TWEAK_LOG_TRACE_ENTRY();
  return tweak_app_context_private_is_connected(context)
    ? tweak_app_context_private_item_replace_current_value(context, tweak_id, value)
    : TWEAK_APP_PEER_DISCONNECTED;
}

static void client_destroy_context(struct tweak_app_context_base* context) {
  TWEAK_LOG_TRACE_ENTRY();
  struct tweak_app_context_client_impl* client_impl = (struct tweak_app_context_client_impl*)context;
  tweak_app_queue_stop(client_impl->base.job_queue);
  call_remove_callback_for_all_items(client_impl);
  tweak_pickle_destroy_client_endpoint(client_impl->rpc_endpoint);
  tweak_app_context_private_destroy_base(&client_impl->base);
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

  tweak_pickle_client_descriptor client_descriptor = {
    .context_type = context_type,
    .params = params,
    .uri = uri,
    .skeleton = {
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
    goto free_client_context;
  }

  client_impl->base.clone_current_value_proc = &check_connection_and_clone_current_value;
  client_impl->base.replace_current_value_proc = &check_connection_and_replace_current_value;
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
   tweak_app_context_private_destroy_base(&client_impl->base);

free_client_context:
  free(client_impl);

  return NULL;
}
