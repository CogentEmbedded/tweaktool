/**
 * @file tweak2.c
 * @ingroup tweak-api
 *
 * @brief implementation of simplified server side tweak2 public API.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2.h>
#include <tweak2/appserver.h>
#include <tweak2/log.h>
#include <tweak2/variant.h>

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

static tweak_app_server_context s_context = NULL;

static inline bool invalid_context() {
  return s_context == NULL;
}

#define MAX_BUFFER_SIZE 256

static pthread_mutex_t s_callback_lock = { };

static tweak_item_change_listener s_item_change_listener = NULL;

static void* s_item_change_listener_cookie = NULL;

struct client_item_changed_listener {
  tweak_item_change_listener client_callback;
  void* client_cookie;
};

static void on_current_value_changed(tweak_app_context context,
  tweak_id id, tweak_variant* value, void *cookie)
{
  (void)value;                  /* This parameter would be quite useful if existing design allowed to pass */
                                /* updated value straight to the callback removing the necessity to */
                                /* invoke tweak_get_* method to access updated value. Hovewer, this */
                                /* decision would imply to drag tweakvariant.h stuff into public API */
                                /* of the library, whilst it is designed to be as simple as possible. */

  (void)cookie;                 /* This parameter would be quite useful if underlying tweak_app API */
                                /* allowed to alter this cookie at runtime, and by that allowing this */
                                /* library to embed item_change_listener and its cookie in user context */
                                /* and remove the necessity of global variables such as s_item_change_listener. */
                                /* However, it is only specified only upon creation and tweak_set_item_change_listener */
                                /* can't change it.*/

  void* item_cookie = tweak_app_item_get_cookie((tweak_app_server_context)context, id);
  if (item_cookie != NULL) {
    struct client_item_changed_listener* client_listener = (struct client_item_changed_listener*) item_cookie;
    assert(client_listener->client_callback);
    client_listener->client_callback(id, client_listener->client_cookie);
  } else {
    tweak_item_change_listener item_change_listener;
    void* item_change_listener_cookie;
    pthread_mutex_lock(&s_callback_lock);
    item_change_listener = s_item_change_listener;
    item_change_listener_cookie = s_item_change_listener_cookie;
    pthread_mutex_unlock(&s_callback_lock);
    if (item_change_listener != NULL) {
      item_change_listener(id, item_change_listener_cookie);
    }
  }
}

void tweak_initialize_library(const char *context_type, const char *params,
  const char *uri)
{
  pthread_mutex_init(&s_callback_lock, NULL);

  tweak_app_server_callbacks callbacks = {
    .on_current_value_changed = &on_current_value_changed
  };

  s_context = tweak_app_create_server_context(context_type, params, uri, &callbacks);
  if (invalid_context()) {
    TWEAK_LOG_ERROR("Library hasn't been initialized correctly");
  }
}

void tweak_set_item_change_listener(tweak_item_change_listener item_change_listener, void* cookie) {
  pthread_mutex_lock(&s_callback_lock);
  s_item_change_listener = item_change_listener;
  s_item_change_listener_cookie = cookie;
  pthread_mutex_unlock(&s_callback_lock);
}

#define TWEAK2_IMPLEMENT_SCALAR_TYPE(Suffix, T, VariantTypeDesc, VariantField, Type, DefaultValue)                               \
  tweak_id tweak_add_##Suffix##_##T##_ex(const struct tweak_add_item_ex_desc* desc, Type initial_value)                          \
  {                                                                                                                              \
    if (invalid_context()) {                                                                                                     \
      TWEAK_LOG_ERROR("%s: Library hasn't been initialized correctly", __func__);                                                \
      return TWEAK_INVALID_ID;                                                                                                   \
    }                                                                                                                            \
                                                                                                                                 \
    tweak_variant variant_value = {                                                                                              \
      .type = VariantTypeDesc,                                                                                                   \
      .VariantField = initial_value                                                                                              \
    };                                                                                                                           \
                                                                                                                                 \
    struct client_item_changed_listener* client_listener = calloc(1, sizeof(*client_listener));                                  \
    if (!client_listener) {                                                                                                      \
      TWEAK_FATAL("Can't allocate memory");                                                                                      \
    }                                                                                                                            \
                                                                                                                                 \
    client_listener->client_callback = desc->item_change_listener;                                                               \
    client_listener->client_cookie = desc->cookie;                                                                               \
                                                                                                                                 \
    return tweak_app_server_add_item(s_context, desc->uri, desc->description, desc->meta, &variant_value, client_listener);      \
  }                                                                                                                              \
                                                                                                                                 \
  tweak_id tweak_add_##Suffix##_##T(const char* uri, const char* description, const char* meta, Type initial_value)              \
  {                                                                                                                              \
    if (invalid_context()) {                                                                                                     \
      TWEAK_LOG_ERROR("%s: Library hasn't been initialized correctly", __func__);                                                \
      return TWEAK_INVALID_ID;                                                                                                   \
    }                                                                                                                            \
                                                                                                                                 \
    tweak_variant variant_value = {                                                                                              \
      .type = VariantTypeDesc,                                                                                                   \
      .VariantField = initial_value                                                                                              \
    };                                                                                                                           \
                                                                                                                                 \
    return tweak_app_server_add_item(s_context, uri, description, meta, &variant_value, NULL);                                   \
  }                                                                                                                              \
                                                                                                                                 \
  void tweak_set_##Suffix##_##T(tweak_id id, Type value) {                                                                       \
    if (invalid_context()) {                                                                                                     \
      TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);                                               \
    }                                                                                                                            \
                                                                                                                                 \
    tweak_variant variant_value = {                                                                                              \
      .type = VariantTypeDesc,                                                                                                   \
      .VariantField = value                                                                                                      \
    };                                                                                                                           \
                                                                                                                                 \
    tweak_app_error_code result = tweak_app_item_replace_current_value((tweak_app_context)s_context, id, &variant_value);        \
    if (result == TWEAK_APP_SUCCESS) {                                                                                           \
      tweak_variant_destroy(&variant_value);                                                                                     \
    } else {                                                                                                                     \
      TWEAK_LOG_ERROR("%s : tweak_app_item_replace_current_value returned 0x%x", __func__, result);                              \
    }                                                                                                                            \
  }                                                                                                                              \
                                                                                                                                 \
  Type tweak_get_##Suffix##_##T(tweak_id id) {                                                                                   \
    if (invalid_context()) {                                                                                                     \
      TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);                                               \
      return DefaultValue;                                                                                                       \
    }                                                                                                                            \
                                                                                                                                 \
    tweak_variant variant_value = {                                                                                              \
      .type = TWEAK_VARIANT_TYPE_IS_NULL                                                                                         \
    };                                                                                                                           \
                                                                                                                                 \
    tweak_app_error_code result = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);          \
    if (result == TWEAK_APP_SUCCESS) {                                                                                           \
      if (variant_value.type == VariantTypeDesc) {                                                                               \
        return variant_value.VariantField;                                                                                       \
      } else {                                                                                                                   \
        TWEAK_LOG_ERROR("%s : type mismatch: item has type 0x%x, expected 0x%x", __func__, variant_value.type, VariantTypeDesc); \
        return DefaultValue;                                                                                                     \
      }                                                                                                                          \
    } else {                                                                                                                     \
      TWEAK_LOG_ERROR("%s : tweak_app_item_clone_current_value returned 0x%x", __func__, result);                                \
      return DefaultValue;                                                                                                       \
    }                                                                                                                            \
  }

TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, float, TWEAK_VARIANT_TYPE_FLOAT, fp32, float, NAN)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, double, TWEAK_VARIANT_TYPE_DOUBLE, fp64, double, NAN)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, bool, TWEAK_VARIANT_TYPE_BOOL, value_bool, bool, false)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int8, TWEAK_VARIANT_TYPE_SINT8, sint8, int8_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int16, TWEAK_VARIANT_TYPE_SINT16, sint16, int16_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int32, TWEAK_VARIANT_TYPE_SINT32, sint32, int32_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int64, TWEAK_VARIANT_TYPE_SINT64, sint64, int64_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint8, TWEAK_VARIANT_TYPE_UINT8, uint8, uint8_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint16, TWEAK_VARIANT_TYPE_UINT16, uint16, uint16_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint32, TWEAK_VARIANT_TYPE_UINT32, uint32, uint32_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint64, TWEAK_VARIANT_TYPE_UINT64, uint64, uint64_t, 0)

tweak_id tweak_find_id(const char* uri) {
  if (invalid_context()) {
    TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);
    return TWEAK_INVALID_ID;
  }

  return tweak_app_find_id(s_context, uri);
}

void tweak_remove(tweak_id id) {
  if (invalid_context()) {
    TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);
    return;
  }

  free(tweak_app_item_get_cookie(s_context, id));
  tweak_app_server_remove_item((tweak_app_server_context)s_context, id);
}

void tweak_finalize_library() {
  if (invalid_context()) {
    TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);
    return;
  }

  tweak_app_destroy_context(s_context);
  pthread_mutex_destroy(&s_callback_lock);
}
