/**
 * @file tweak2.c
 * @ingroup tweak-api
 *
 * @brief implementation of simplified server side tweak2 public API.
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

#include <tweak2/tweak2.h>
#include <tweak2/appserver.h>
#include <tweak2/log.h>
#include <tweak2/buffer.h>
#include <tweak2/variant.h>
#include <tweak2/thread.h>

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

struct tweak_default_client_item_changed_listener {
  tweak_item_change_listener client_callback;
  void* client_cookie;
};

tweak_default_client_item_changed_listener_t tweak_create_default_client_item_changed_listener(
  tweak_item_change_listener client_callback,
  void* client_cookie)
{
  tweak_default_client_item_changed_listener_t result = malloc(sizeof(*result));
  if (result == NULL) {
    return NULL;
  }

  result->client_callback = client_callback;
  result->client_cookie = client_cookie;
  return result;
}

static tweak_app_server_context s_context = NULL;

static inline bool invalid_context() {
  return s_context == NULL;
}

#define MAX_BUFFER_SIZE 256

static tweak_common_mutex s_callback_lock = { 0 };

static tweak_item_change_listener s_item_change_listener = NULL;

static void* s_item_change_listener_cookie = NULL;

static char *s_context_type = NULL;

static char *s_params = NULL;

static char *s_uri = NULL;

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
    tweak_default_client_item_changed_listener_t client_listener = (tweak_default_client_item_changed_listener_t) item_cookie;
    assert(client_listener->client_callback);
    client_listener->client_callback(id, client_listener->client_cookie);
  } else {
    tweak_item_change_listener item_change_listener;
    void* item_change_listener_cookie;
    tweak_common_mutex_lock(&s_callback_lock);
    item_change_listener = s_item_change_listener;
    item_change_listener_cookie = s_item_change_listener_cookie;
    tweak_common_mutex_unlock(&s_callback_lock);
    if (item_change_listener != NULL) {
      item_change_listener(id, item_change_listener_cookie);
    }
  }
}

static bool strings_are_equal(const char *arg1, const char *arg2) {
  bool both_null = !arg1 && !arg2;
  bool both_non_null = arg1 != NULL && arg2 != NULL;
  bool non_null_and_equal = both_non_null && (strcmp(arg1, arg2) == 0);
  return both_null || non_null_and_equal;
}

static void check_finalized(void) {
  if (s_context != NULL) {
    TWEAK_LOG_WARN("tweak_finalize_library() hasn't been called prior to exit");
  }
}

void tweak_initialize_library(const char *context_type, const char *params,
  const char *uri)
{
  if (s_context != NULL) {
    if ((strings_are_equal(s_context_type, context_type) &&
         strings_are_equal(s_params, params) &&
         strings_are_equal(s_uri, uri)))
    {
      TWEAK_LOG_DEBUG("Context already initalized");
      return;
    } else {
      TWEAK_FATAL("Configuration mismatch");
    }
  }

  if (!context_type) {
    TWEAK_FATAL("context_type is NULL. Try setting this to \"nng\" on generic POSIX/x86 platform");
  }

  tweak_common_mutex_init(&s_callback_lock);
  atexit(&check_finalized);

  tweak_app_server_callbacks callbacks = {
    .on_current_value_changed = &on_current_value_changed
  };

  s_context_type = context_type ? strdup(context_type) : NULL;
  s_params = params ? strdup(params) : NULL;
  s_uri = uri ? strdup(uri) : NULL;

  s_context = tweak_app_create_server_context(s_context_type, s_params, s_uri, &callbacks);
  if (invalid_context()) {
    free(s_context_type);
    s_context_type = NULL;
    free(s_params);
    s_params = NULL;
    free(s_uri);
    s_uri = NULL;
    TWEAK_LOG_ERROR("Library hasn't been initialized correctly");
  }
}

void tweak_set_item_change_listener(tweak_item_change_listener item_change_listener, void* cookie) {
  tweak_common_mutex_lock(&s_callback_lock);
  s_item_change_listener = item_change_listener;
  s_item_change_listener_cookie = cookie;
  tweak_common_mutex_unlock(&s_callback_lock);
}

#define TWEAK2_IMPLEMENT_SCALAR_TYPE(SUFFIX, T, VARIANT_TYPE_DESC, VARIANT_FIELD, TYPE, DEFAULT_VALUE)                              \
  tweak_id tweak_add_##SUFFIX##_##T##_ex(const struct tweak_add_item_ex_desc* desc, TYPE initial_value)                             \
  {                                                                                                                                 \
    if (invalid_context()) {                                                                                                        \
      TWEAK_LOG_ERROR("%s: Library hasn't been initialized correctly", __func__);                                                   \
      return TWEAK_INVALID_ID;                                                                                                      \
    }                                                                                                                               \
                                                                                                                                    \
    tweak_variant variant_value = {                                                                                                 \
      .type = VARIANT_TYPE_DESC,                                                                                                    \
      .value = {                                                                                                                    \
        .VARIANT_FIELD = initial_value                                                                                              \
      }                                                                                                                             \
    };                                                                                                                              \
                                                                                                                                    \
    tweak_default_client_item_changed_listener_t client_listener = NULL;                                                            \
    if (desc->item_change_listener != NULL)                                                                                         \
    {                                                                                                                               \
      client_listener = tweak_create_default_client_item_changed_listener(desc->item_change_listener, desc->cookie);                \
      if (!client_listener)                                                                                                         \
      {                                                                                                                             \
        TWEAK_FATAL("Can't allocate memory");                                                                                       \
      }                                                                                                                             \
    }                                                                                                                               \
    return tweak_app_server_add_item(s_context, desc->uri, desc->description, desc->meta, &variant_value, client_listener);         \
  }                                                                                                                                 \
                                                                                                                                    \
  tweak_id tweak_add_##SUFFIX##_##T(const char* uri, const char* description, const char* meta, TYPE initial_value)                 \
  {                                                                                                                                 \
    if (invalid_context()) {                                                                                                        \
      TWEAK_LOG_ERROR("%s: Library hasn't been initialized correctly", __func__);                                                   \
      return TWEAK_INVALID_ID;                                                                                                      \
    }                                                                                                                               \
                                                                                                                                    \
    tweak_variant variant_value = {                                                                                                 \
      .type = VARIANT_TYPE_DESC,                                                                                                    \
      .value = {                                                                                                                    \
        .VARIANT_FIELD = initial_value                                                                                              \
      }                                                                                                                             \
    };                                                                                                                              \
                                                                                                                                    \
    return tweak_app_server_add_item(s_context, uri, description, meta, &variant_value, NULL);                                      \
  }                                                                                                                                 \
                                                                                                                                    \
  void tweak_set_##SUFFIX##_##T(tweak_id id, TYPE value) {                                                                          \
    if (invalid_context()) {                                                                                                        \
      TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);                                                  \
    }                                                                                                                               \
                                                                                                                                    \
    tweak_variant variant_value = {                                                                                                 \
      .type = VARIANT_TYPE_DESC,                                                                                                    \
      .value = {                                                                                                                    \
        .VARIANT_FIELD = value                                                                                                      \
      }                                                                                                                             \
    };                                                                                                                              \
                                                                                                                                    \
    tweak_app_error_code result = tweak_app_item_replace_current_value((tweak_app_context)s_context, id, &variant_value);           \
    if (result != TWEAK_APP_SUCCESS) {                                                                                              \
      TWEAK_LOG_ERROR("%s : tweak_app_item_replace_current_value returned 0x%x", __func__, result);                                 \
    }                                                                                                                               \
    tweak_variant_destroy(&variant_value);                                                                                          \
  }                                                                                                                                 \
                                                                                                                                    \
  TYPE tweak_get_##SUFFIX##_##T(tweak_id id) {                                                                                      \
    if (invalid_context()) {                                                                                                        \
      TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);                                                  \
      return DEFAULT_VALUE;                                                                                                         \
    }                                                                                                                               \
    tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;                                                                         \
    tweak_app_error_code result = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);             \
    if (result == TWEAK_APP_SUCCESS) {                                                                                              \
      if (variant_value.type == VARIANT_TYPE_DESC) {                                                                                \
        return variant_value.value.VARIANT_FIELD;                                                                                   \
      } else {                                                                                                                      \
        TWEAK_LOG_ERROR("%s : Type mismatch: item has type 0x%x, expected 0x%x", __func__, variant_value.type, VARIANT_TYPE_DESC);  \
        return DEFAULT_VALUE;                                                                                                       \
      }                                                                                                                             \
    } else {                                                                                                                        \
      TWEAK_LOG_ERROR("%s : tweak_app_item_clone_current_value returned 0x%x", __func__, result);                                   \
      return DEFAULT_VALUE;                                                                                                         \
    }                                                                                                                               \
  }

TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, float, TWEAK_VARIANT_TYPE_FLOAT, fp32, float, NAN)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, double, TWEAK_VARIANT_TYPE_DOUBLE, fp64, double, NAN)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, bool, TWEAK_VARIANT_TYPE_BOOL, b, bool, false)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int8, TWEAK_VARIANT_TYPE_SINT8, sint8, int8_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int16, TWEAK_VARIANT_TYPE_SINT16, sint16, int16_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int32, TWEAK_VARIANT_TYPE_SINT32, sint32, int32_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, int64, TWEAK_VARIANT_TYPE_SINT64, sint64, int64_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint8, TWEAK_VARIANT_TYPE_UINT8, uint8, uint8_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint16, TWEAK_VARIANT_TYPE_UINT16, uint16, uint16_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint32, TWEAK_VARIANT_TYPE_UINT32, uint32, uint32_t, 0)
TWEAK2_IMPLEMENT_SCALAR_TYPE(scalar, uint64, TWEAK_VARIANT_TYPE_UINT64, uint64, uint64_t, 0)

#define TWEAK2_IMPLEMENT_VECTOR_TYPE(SUFFIX, T, VARIANT_TYPE_DESC)                                                                  \
  tweak_id tweak_create_vector_##SUFFIX(const struct tweak_add_item_ex_desc *desc,                                                  \
    const T* initial_value, size_t count)                                                                                           \
  {                                                                                                                                 \
    if (invalid_context())                                                                                                          \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s: Library hasn't been initialized correctly", __func__);                                                   \
      return TWEAK_INVALID_ID;                                                                                                      \
    }                                                                                                                               \
                                                                                                                                    \
    tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;                                                                         \
    tweak_variant_assign_##SUFFIX##_vector(&variant_value, initial_value, count);                                                   \
    tweak_default_client_item_changed_listener_t client_listener = NULL;                                                            \
    if (desc->item_change_listener != NULL)                                                                                         \
    {                                                                                                                               \
      client_listener = tweak_create_default_client_item_changed_listener(desc->item_change_listener, desc->cookie);                \
      if (!client_listener)                                                                                                         \
      {                                                                                                                             \
        TWEAK_FATAL("Can't allocate memory");                                                                                       \
      }                                                                                                                             \
    }                                                                                                                               \
    return tweak_app_server_add_item(s_context, desc->uri, desc->description,                                                       \
      desc->meta, &variant_value, client_listener);                                                                                 \
  }                                                                                                                                 \
                                                                                                                                    \
  void tweak_set_vector_##SUFFIX(tweak_id id, const T* buffer)                                                                      \
  {                                                                                                                                 \
    tweak_app_error_code result;                                                                                                    \
    if (invalid_context())                                                                                                          \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);                                                  \
      return;                                                                                                                       \
    }                                                                                                                               \
                                                                                                                                    \
    tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;                                                                         \
    result = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);                                  \
    if (result != TWEAK_APP_SUCCESS)                                                                                                \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s : tweak_app_item_clone_current_value returned 0x%x", __func__, result);                                   \
      return;                                                                                                                       \
    }                                                                                                                               \
                                                                                                                                    \
    if (variant_value.type != VARIANT_TYPE_DESC)                                                                                    \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s : item has invalid type %d, expected %d", __func__, variant_value.type, VARIANT_TYPE_DESC);               \
      goto exit;                                                                                                                    \
    }                                                                                                                               \
                                                                                                                                    \
    memcpy(tweak_buffer_get_data(&variant_value.value.buffer),                                                                      \
      buffer, tweak_buffer_get_size(&variant_value.value.buffer));                                                                  \
    result = tweak_app_item_replace_current_value((tweak_app_context)s_context, id, &variant_value);                                \
                                                                                                                                    \
    if (result != TWEAK_APP_SUCCESS)                                                                                                \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s : tweak_app_item_replace_current_value returned 0x%x", __func__, result);                                 \
      goto exit;                                                                                                                    \
    }                                                                                                                               \
                                                                                                                                    \
  exit:                                                                                                                             \
    tweak_variant_destroy(&variant_value);                                                                                          \
  }                                                                                                                                 \
                                                                                                                                    \
  void tweak_get_vector_##SUFFIX(tweak_id id, T* buffer) {                                                                          \
    tweak_app_error_code result;                                                                                                    \
    if (invalid_context())                                                                                                          \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);                                                  \
      return;                                                                                                                       \
    }                                                                                                                               \
                                                                                                                                    \
    tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;                                                                         \
    result = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);                                  \
    if (result != TWEAK_APP_SUCCESS)                                                                                                \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s : tweak_app_item_clone_current_value returned 0x%x", __func__, result);                                   \
      goto exit;                                                                                                                    \
    }                                                                                                                               \
                                                                                                                                    \
    if (variant_value.type != VARIANT_TYPE_DESC)                                                                                    \
    {                                                                                                                               \
      TWEAK_LOG_ERROR("%s : item has invalid type %d, expected %d", __func__, variant_value.type, VARIANT_TYPE_DESC);               \
      goto exit;                                                                                                                    \
    }                                                                                                                               \
                                                                                                                                    \
    memcpy(buffer, tweak_buffer_get_data(&variant_value.value.buffer),                                                              \
      tweak_buffer_get_size(&variant_value.value.buffer));                                                                          \
                                                                                                                                    \
  exit:                                                                                                                             \
    tweak_variant_destroy(&variant_value);                                                                                          \
  }

TWEAK2_IMPLEMENT_VECTOR_TYPE(sint8, int8_t, TWEAK_VARIANT_TYPE_VECTOR_SINT8)
TWEAK2_IMPLEMENT_VECTOR_TYPE(sint16, int16_t, TWEAK_VARIANT_TYPE_VECTOR_SINT16)
TWEAK2_IMPLEMENT_VECTOR_TYPE(sint32, int32_t, TWEAK_VARIANT_TYPE_VECTOR_SINT32)
TWEAK2_IMPLEMENT_VECTOR_TYPE(sint64, int64_t, TWEAK_VARIANT_TYPE_VECTOR_SINT64)
TWEAK2_IMPLEMENT_VECTOR_TYPE(uint8, uint8_t, TWEAK_VARIANT_TYPE_VECTOR_UINT8)
TWEAK2_IMPLEMENT_VECTOR_TYPE(uint16, uint16_t, TWEAK_VARIANT_TYPE_VECTOR_UINT16)
TWEAK2_IMPLEMENT_VECTOR_TYPE(uint32, uint32_t, TWEAK_VARIANT_TYPE_VECTOR_UINT32)
TWEAK2_IMPLEMENT_VECTOR_TYPE(uint64, uint64_t, TWEAK_VARIANT_TYPE_VECTOR_UINT64)
TWEAK2_IMPLEMENT_VECTOR_TYPE(float, float, TWEAK_VARIANT_TYPE_VECTOR_FLOAT)
TWEAK2_IMPLEMENT_VECTOR_TYPE(double, double, TWEAK_VARIANT_TYPE_VECTOR_DOUBLE)

size_t tweak_get_vector_item_count(tweak_id id)
{
  size_t result = 0;
  tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;

  if (invalid_context())
  {
    TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);
    goto exit;
  }

  result = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);
  if (result != TWEAK_APP_SUCCESS)
  {
    TWEAK_LOG_ERROR("%s : tweak_app_item_clone_current_value returned 0x%x", __func__, result);
    goto exit;
  }

  result = tweak_variant_get_item_count(&variant_value);

exit:
  tweak_variant_destroy(&variant_value);
  return result;
}

tweak_id tweak_create_string(const struct tweak_add_item_ex_desc *desc, const char *initial_value)
{
  if (invalid_context())
  {
    TWEAK_LOG_ERROR("%s: Library hasn't been initialized correctly", __func__);
    return TWEAK_INVALID_ID;
  }

  tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_string(&variant_value, initial_value);

  tweak_default_client_item_changed_listener_t client_listener = tweak_create_default_client_item_changed_listener(desc->item_change_listener, desc->cookie);
  if (!client_listener)
  {
    TWEAK_FATAL("Can't allocate memory");
  }

  return tweak_app_server_add_item(s_context, desc->uri, desc->description,
                                   desc->meta, &variant_value, client_listener);
}

void tweak_set_string(tweak_id id, const char *string)
{
  if (invalid_context())
  {
    TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);
    return;
  }

  tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_string(&variant_value, string);

  tweak_app_error_code result = tweak_app_item_replace_current_value((tweak_app_context)s_context, id, &variant_value);
  if (result != TWEAK_APP_SUCCESS)
  {
    TWEAK_LOG_ERROR("%s : tweak_app_item_replace_current_value returned 0x%x", __func__, result);
  }
  tweak_variant_destroy(&variant_value);
}

size_t tweak_get_string(tweak_id id, char *buffer, size_t size)
{
  if (invalid_context())
  {
    TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);
    return 0UL;
  }
  tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_app_error_code result = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);
  if (result == TWEAK_APP_SUCCESS)
  {
    if (variant_value.type == TWEAK_VARIANT_TYPE_STRING)
    {
      size_t string_length = variant_value.value.string.length;
      strncpy(buffer, tweak_variant_string_c_str(&variant_value.value.string), size);
      if (string_length >= size) {
        buffer[size - 4] = '.';
        buffer[size - 3] = '.';
        buffer[size - 2] = '.';
        buffer[size - 1] = '\0';
      }
      tweak_variant_destroy(&variant_value);
      return string_length;
    }
    else
    {
      TWEAK_LOG_ERROR("%s : Type mismatch: item has type 0x%x, expected 0x%x",
        __func__, variant_value.type, TWEAK_VARIANT_TYPE_STRING);
      tweak_variant_destroy(&variant_value);
      return 0UL;
    }
  }
  else
  {
    TWEAK_LOG_ERROR("%s : tweak_app_item_clone_current_value returned 0x%x", __func__, result);
    return 0UL;
  }
}

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

static bool free_attachment(const tweak_app_item_snapshot* snapshot,
  void* cookie)
{
  (void)cookie;
  free(tweak_app_item_get_cookie(s_context, snapshot->id));
  return true;
}

void tweak_finalize_library() {
  if (invalid_context()) {
    TWEAK_LOG_ERROR("%s : Library hasn't been initialized correctly", __func__);
    return;
  }

  free(s_context_type);
  s_context_type = NULL;
  free(s_params);
  s_params = NULL;
  free(s_uri);
  s_uri = NULL;
  (void)tweak_app_traverse_items(s_context, free_attachment, NULL);
  tweak_app_destroy_context(s_context);
  tweak_common_mutex_destroy(&s_callback_lock);
  s_context = NULL;
}

struct tweak_app_context_base* tweak_get_default_server_instance() {
  return s_context;
}
