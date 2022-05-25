/**
 * @file tweakcompat.c
 * @ingroup tweak-api
 * @brief part of tweak2 - tweak1 compatibility layer implementation.
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

#include <tweak.h>
#include <tweak2/appserver.h>
#include <tweak2/log.h>
#include <tweak2/thread.h>
#include <tweak2/types.h>
#include <tweak2/variant.h>

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <uthash.h>
#include <stdarg.h>

static tweak_app_server_context s_context = NULL;

static tweak_common_mutex s_callback_lock = { 0 };

static tweak_update_handler s_callback = NULL;

static void* s_callback_cookie = NULL;

static uint32_t s_layout_id = 0L;

static char* s_current_widget_name = NULL;

static char* s_layout_meta = NULL;

#define MAX_BUFFER_SIZE (4 * 1024)

#define MAX_NAME_LENGTH 256

#define MAX_OPTIONS_STRING_SIZE 512

static char s_buffer[MAX_BUFFER_SIZE];

struct name_uri_pair {
  char* name;
  char* uri;
  UT_hash_handle hh;
};

static tweak_common_rwlock s_name_uri_lock = { 0 };

struct name_uri_pair* s_name_uri_pairs = NULL;

struct name_uri_pair* s_uri_name_pairs = NULL;

static char s_uri_concat_buffer[MAX_BUFFER_SIZE];

static char* add_uri_mapping(const char* name, const char* uri) {
  struct name_uri_pair* pair = NULL;
  HASH_FIND_STR(s_name_uri_pairs, name, pair);
  if (pair != NULL) {
    TWEAK_FATAL("duplicate name");
  }
  pair = malloc(sizeof(*pair));
  if (pair == NULL) {
    TWEAK_FATAL("malloc() returned NULL");
  }
  size_t length = strlen(name);
  char* name_dup = strdup(name);
  char* uri_dup = strdup(uri);
  pair->name = name_dup;
  pair->uri = uri_dup;
  HASH_ADD_KEYPTR(hh, s_name_uri_pairs, name_dup, length, pair);
  return uri_dup;
}

static char* add_name_mapping(const char* uri, const char* name) {
  struct name_uri_pair* pair = NULL;
  HASH_FIND_STR(s_uri_name_pairs, uri, pair);
  if (pair != NULL) {
    TWEAK_FATAL("duplicate name");
  }
  pair = malloc(sizeof(*pair));
  if (pair == NULL) {
    TWEAK_FATAL("malloc() returned NULL");
  }
  char* name_dup = strdup(name);
  size_t length = strlen(uri);
  char* uri_dup = strdup(uri);
  pair->name = name_dup;
  pair->uri = uri_dup;
  HASH_ADD_KEYPTR(hh, s_uri_name_pairs, uri_dup, length, pair);
  return name_dup;
}

static char* get_uri(const char* name);

static const char* map_uri(const char* layout_name, const char* name) {
  if (!name) {
    TWEAK_FATAL("Name parameter is NULL");
    return NULL;
  }
  if (strlen(name) >= MAX_NAME_LENGTH) {
    TWEAK_FATAL("name is too long");
    return NULL;
  }
  if (layout_name && strlen(layout_name) >= MAX_NAME_LENGTH) {
    TWEAK_FATAL("layout name is too long");
    return NULL;
  }

  const char* uri = NULL;
  tweak_common_rwlock_write_lock(&s_name_uri_lock);
  s_uri_concat_buffer[0] = '\0';
  if (layout_name != NULL) {
    if (layout_name[0] != '/') {
      strcat(s_uri_concat_buffer, "/");
    }
    strcat(s_uri_concat_buffer, layout_name);
    strcat(s_uri_concat_buffer, "/");
    strcat(s_uri_concat_buffer, name);
  } else {
    if (name[0] != '/') {
      strcat(s_uri_concat_buffer, "/");
    }
    strcat(s_uri_concat_buffer, name);
  }
  uri = add_uri_mapping(name, s_uri_concat_buffer);
  add_name_mapping(s_uri_concat_buffer, name);
  tweak_common_rwlock_write_unlock(&s_name_uri_lock);
  return uri;
}

static char* get_uri(const char* name) {
  if (!name) {
    TWEAK_FATAL("name is NULL");
  }
  struct name_uri_pair* pair = NULL;
  tweak_common_rwlock_read_lock(&s_name_uri_lock);
  HASH_FIND_STR(s_name_uri_pairs, name, pair);
  tweak_common_rwlock_write_unlock(&s_name_uri_lock);
  if (pair == NULL) {
    return NULL;
  }
  return pair->uri;
}

static char* get_name(const char* uri) {
  if (!uri) {
    TWEAK_FATAL("uri is NULL");
  }
  struct name_uri_pair* pair = NULL;
  tweak_common_rwlock_read_lock(&s_name_uri_lock);
  HASH_FIND_STR(s_uri_name_pairs, uri, pair);
  tweak_common_rwlock_read_unlock(&s_name_uri_lock);
  if (pair == NULL) {
    return NULL;
  }
  return pair->name;
}

void tweak_on_update(const char* name) {
  (void) name;
  TWEAK_FATAL("Not supported in TWEAK 2."
              " Use tweak_set_update_handler");
}

static void on_current_value_changed(tweak_app_context context,
  tweak_id id, tweak_variant* value, void *cookie)
{
  (void)context;
  (void)value;
  (void)cookie;
  tweak_update_handler callback;
  void* callback_cookie;
  tweak_common_mutex_lock(&s_callback_lock);
  callback = s_callback;
  callback_cookie = s_callback_cookie;
  tweak_common_mutex_unlock(&s_callback_lock);
  if (callback) {
    tweak_app_item_snapshot* snapshot = tweak_app_item_get_snapshot(s_context, id);
    const char *name = get_name(tweak_variant_string_c_str(&snapshot->uri));
    callback(name, callback_cookie);
    tweak_app_release_snapshot(s_context, snapshot);
  }
}

void tweak_set_update_handler(tweak_update_handler handler, void* cookie) {
  tweak_common_mutex_lock(&s_callback_lock);
  s_callback = handler;
  s_callback_cookie = cookie;
  tweak_common_mutex_unlock(&s_callback_lock);
}

int tweak_connect(void) {
  tweak_common_mutex_init(&s_callback_lock);
  tweak_common_rwlock_init(&s_name_uri_lock);

  tweak_app_server_callbacks callbacks = {
    .on_current_value_changed = &on_current_value_changed
  };

  s_context = tweak_app_create_server_context(TWEAK_CONNECTION_TYPE, TWEAK_PARAMS, TWEAK_URI, &callbacks);
  if (!s_context) {
    TWEAK_LOG_ERROR("Library hasn't been initialized correctly");
    return 0;
  }

  return 1;
}

static const char* LAYOUT_META_TEMPLATE = "{"
  "\"layout_id\": %u,"
  "\"width\": %u,"
  "\"horizontal_vertical\": %u,"
  "\"layout_name\": \"%s\""
  "}";

void tweak_add_layout(unsigned int width, unsigned int horizontal_vertical, const char* name) {
  ++s_layout_id;
  int result_size;

  result_size = snprintf(s_buffer, sizeof(s_buffer),
    LAYOUT_META_TEMPLATE, s_layout_id, width,
    horizontal_vertical, name);

  if (result_size >= MAX_BUFFER_SIZE) {
    TWEAK_FATAL("Buffer to format meta string is too small");
  }

  char* old_layout_meta = s_layout_meta;
  s_layout_meta = strdup(s_buffer);
  if (!s_layout_meta) {
    TWEAK_FATAL("strdup() returned NULL");
  }

  free(old_layout_meta);
}

void tweak_close() {
  if (!s_context) {
    TWEAK_LOG_ERROR("Library hasn't been initialized correctly");
    return;
  }

  tweak_common_rwlock_write_lock(&s_name_uri_lock);
  struct name_uri_pair *pair = NULL;
  struct name_uri_pair *tmp = NULL;
  HASH_ITER(hh, s_name_uri_pairs, pair, tmp) {
    HASH_DEL(s_name_uri_pairs, pair);
    free(pair->name);
    free(pair->uri);
    free(pair);
  }
  s_name_uri_pairs = NULL;
  HASH_ITER(hh, s_uri_name_pairs, pair, tmp) {
    HASH_DEL(s_uri_name_pairs, pair);
    free(pair->name);
    free(pair->uri);
    free(pair);
  }
  s_uri_name_pairs = NULL;
  tweak_common_rwlock_write_unlock(&s_name_uri_lock);

  tweak_app_destroy_context(s_context);
  tweak_common_rwlock_destroy(&s_name_uri_lock);
  tweak_common_mutex_destroy(&s_callback_lock);

  s_context = NULL;

  free(s_layout_meta);
  s_layout_meta = NULL;
}

static const char* META_TEMPLATE_WITH_LAYOUT = "{"
  "\"type\": \"double\","
  "\"control\": \"%s\","
  "\"min\": %f,"
  "\"max\": %f,"
  "\"readonly\": false,"
  "\"decimals\": %u,"
  "\"layout\": %s"
  "}";

static const char* META_TEMPLATE_WITHOUT_LAYOUT = "{"
  "\"type\": \"double\","
  "\"control\": \"%s\","
  "\"min\": %f,"
  "\"max\": %f,"
  "\"readonly\": false,"
  "\"decimals\": %u"
  "}";

void tweak_add_slider(const char* name, double minv, double maxv, double def, unsigned int precision) {
  int result_size;

  const char* uri = map_uri(s_current_widget_name, name);
  if (s_layout_meta) {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      META_TEMPLATE_WITH_LAYOUT,
      "slider", minv, maxv, precision, s_layout_meta);
  } else {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      META_TEMPLATE_WITHOUT_LAYOUT,
      "slider", minv, maxv, precision);
  }
  if (result_size >= MAX_BUFFER_SIZE) {
    TWEAK_FATAL("Buffer to format meta string is too small");
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_double(&value, def);

  tweak_id id = tweak_app_server_add_item(s_context,
    uri, name, s_buffer, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

void tweak_add_spinbox(const char* name, double minv, double maxv, double def, unsigned int precision) {
  int result_size;

  const char* uri = map_uri(s_current_widget_name, name);
  if (s_layout_meta) {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      META_TEMPLATE_WITH_LAYOUT,
      "spinbox", minv, maxv, precision, s_layout_meta);
  } else {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      META_TEMPLATE_WITHOUT_LAYOUT,
      "spinbox", minv, maxv, precision);
  }
  if (result_size >= MAX_BUFFER_SIZE) {
    TWEAK_FATAL("Buffer to format meta string is too small");
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_double(&value, def);

  tweak_id id = tweak_app_server_add_item(s_context,
    uri, name, s_buffer, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

static const char* BOOL_META_TEMPLATE_WITH_LAYOUT = "{"
  "\"type\": \"bool\","
  "\"control\": \"%s\","
  "\"min\": false,"
  "\"max\": true,"
  "\"readonly\": false,"
  "\"layout\": %s"
  "}";

static const char* BOOL_META_TEMPLATE_WITHOUT_LAYOUT = "{"
  "\"type\": \"bool\","
  "\"control\": \"%s\","
  "\"min\": false,"
  "\"max\": true,"
  "\"readonly\": false"
  "}";

void tweak_add_checkbox(const char* name, int def_val) {
  int result_size;

  const char* uri = map_uri(s_current_widget_name, name);
  if (s_layout_meta) {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      BOOL_META_TEMPLATE_WITH_LAYOUT, "checkbox", s_layout_meta);
  } else {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      BOOL_META_TEMPLATE_WITHOUT_LAYOUT, "checkbox");
  }
  if (result_size >= MAX_BUFFER_SIZE) {
    TWEAK_FATAL("Buffer to format meta string is too small");
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_bool(&value, def_val != 0);

  tweak_id id = tweak_app_server_add_item(s_context,
    uri, name, s_buffer, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

void tweak_add_button(const char* name) {
  int result_size;

  const char* uri = map_uri(s_current_widget_name, name);
  if (s_layout_meta) {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      BOOL_META_TEMPLATE_WITH_LAYOUT, "button", s_layout_meta);
  } else {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      BOOL_META_TEMPLATE_WITHOUT_LAYOUT, "button");
  }
  if (result_size >= MAX_BUFFER_SIZE) {
    TWEAK_FATAL("Buffer to format meta string is too small");
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_bool(&value, false);

  tweak_id id = tweak_app_server_add_item(s_context,
    uri, name, s_buffer, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

static const char* RADIO_BUTTON_META_TEMPLATE_WITH_LAYOUT = "{"
  "\"type\": \"enum\","
  "\"control\": \"radio\","
  "\"options\": [%s],"
  "\"readonly\": false,"
  "\"layout\": %s"
  "}";

static const char* RADIO_BUTTON_META_TEMPLATE_WITHOUT_LAYOUT = "{"
  "\"type\": \"enum\","
  "\"control\": \"radio\","
  "\"options\": [%s],"
  "\"readonly\": false"
  "}";

void tweak_add_groupbox(const char* name, const char* options, unsigned int def) {
  assert(options != NULL && strlen(options) < MAX_OPTIONS_STRING_SIZE);
  const char* uri = map_uri(s_current_widget_name, name);

  char* token;
  char* options_dup = strdup(options);
  char* rest = options_dup;

  s_buffer[0] = '\0';
  token = strtok(rest, ";");
  while (token) {
    strcat(s_buffer, "\"");
    strcat(s_buffer, token);
    strcat(s_buffer, "\"");
    token = strtok(NULL, ";");
    if (token) {
      strcat(s_buffer, ",");
    }
  }
  free(options_dup);
  char* options_json = strdup(s_buffer);

  int result_size;
  if (s_layout_meta) {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      RADIO_BUTTON_META_TEMPLATE_WITH_LAYOUT, options_json, s_layout_meta);
  } else {
    result_size = snprintf(s_buffer, sizeof(s_buffer),
      RADIO_BUTTON_META_TEMPLATE_WITHOUT_LAYOUT, options_json);
  }
  if (result_size >= MAX_BUFFER_SIZE) {
    TWEAK_FATAL("Buffer to format meta string is too small");
  }
  free(options_json);

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_uint32(&value, def);

  tweak_id id = tweak_app_server_add_item(s_context,
    uri, name, s_buffer, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

void tweak_add_widget(const char* name) {
  char* old_widget_name = s_current_widget_name;
  s_current_widget_name = strdup(name);
  if (!s_current_widget_name) {
    TWEAK_FATAL("strdup() returned NULL");
  }
  free(old_widget_name);
}

double tweak_get(const char* name, double defval) {
  const char* uri = get_uri(name);
  tweak_id id = tweak_app_find_id(s_context, uri);
  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("No tweak with name: %s", name);
    return defval;
  }
  tweak_variant variant_value = TWEAK_VARIANT_INIT_EMPTY;
  double result = defval;
  tweak_app_error_code error_code = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);
  if (error_code == TWEAK_APP_SUCCESS) {
    switch (variant_value.type) {
    case TWEAK_VARIANT_TYPE_BOOL:
      result = variant_value.value.b;
      break;
    case TWEAK_VARIANT_TYPE_SINT8:
      result = variant_value.value.sint8;
      break;
    case TWEAK_VARIANT_TYPE_SINT16:
      result = variant_value.value.sint16;
      break;
    case TWEAK_VARIANT_TYPE_SINT32:
      result = variant_value.value.sint32;
      break;
    case TWEAK_VARIANT_TYPE_SINT64:
      result = (double)variant_value.value.sint64;
      break;
    case TWEAK_VARIANT_TYPE_UINT8:
      result = variant_value.value.uint8;
      break;
    case TWEAK_VARIANT_TYPE_UINT16:
      result = variant_value.value.uint16;
      break;
    case TWEAK_VARIANT_TYPE_UINT32:
      result = variant_value.value.uint32;
      break;
    case TWEAK_VARIANT_TYPE_UINT64:
      result = (double)variant_value.value.uint64;
      break;
    case TWEAK_VARIANT_TYPE_FLOAT:
      result = variant_value.value.fp32;
      break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
      result = variant_value.value.fp64;
      break;
    default:
      break;
    }
  } else {
    TWEAK_LOG_ERROR("tweak_app_item_clone_current_value returned 0x%x", result);
    return defval;
  }
  return result;
}

void tweak_set(const char* name, double val) {
  const char* uri = get_uri(name);
  tweak_id id = tweak_app_find_id(s_context, uri);
  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("No tweak with name: %s", name);
    return;
  }
  tweak_variant_type type = tweak_app_item_get_type(s_context, id);
  if (type != TWEAK_VARIANT_TYPE_NULL) {
    tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
    switch (type) {
    case TWEAK_VARIANT_TYPE_BOOL:
      tweak_variant_assign_bool(&value, val != 0);
      break;
    case TWEAK_VARIANT_TYPE_SINT8:
      tweak_variant_assign_sint8(&value, (int8_t) val);
      break;
    case TWEAK_VARIANT_TYPE_SINT16:
      tweak_variant_assign_sint16(&value, (int16_t) val);
      break;
    case TWEAK_VARIANT_TYPE_SINT32:
      tweak_variant_assign_sint32(&value, (int32_t) val);
      break;
    case TWEAK_VARIANT_TYPE_SINT64:
      tweak_variant_assign_sint64(&value, (int64_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT8:
      tweak_variant_assign_uint8(&value, (uint8_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT16:
      tweak_variant_assign_uint16(&value, (uint16_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT32:
      tweak_variant_assign_uint32(&value, (uint32_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT64:
      tweak_variant_assign_uint64(&value, (uint64_t) val);
      break;
    case TWEAK_VARIANT_TYPE_FLOAT:
      tweak_variant_assign_float(&value, (float) val);
      break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
      tweak_variant_assign_double(&value, val);
      break;
    default:
      TWEAK_LOG_ERROR("Unsupported tweak type 0x%x", type);
      break;
    }
    tweak_app_error_code result = tweak_app_item_replace_current_value(s_context, id, &value);
    if (result != TWEAK_APP_SUCCESS) {
      TWEAK_LOG_ERROR("tweak_app_item_replace_current_value returned 0x%x", result);
    }
  } else {
    TWEAK_LOG_ERROR("Item %s has null type", name);
  }
}

double tweak_get_string(const char* name, double defval) {
  (void) name;
  (void) defval;
  TWEAK_LOG_WARN("tweak_get_string not implemented");
  return 0;
}

uint64_t tweak_fopen(const char* name, const char* mode) {
  (void) name;
  (void) mode;
  TWEAK_LOG_WARN("tweak_get_string not implemented");
  return 0;
}

uint64_t tweak_fclose(uint64_t fd) {
  (void) fd;
  TWEAK_LOG_WARN("tweak_fclose not implemented");
  return 0;
}

uint64_t tweak_ftell(uint64_t fd) {
  (void) fd;
  TWEAK_LOG_WARN("tweak_ftell not implemented");
  return 0;
}

uint64_t tweak_fseek(uint64_t fd, int32_t offset, int32_t where){
  (void) fd;
  (void) offset;
  (void) where;
  TWEAK_LOG_WARN("tweak_fseek not implemented");
  return 0;
}

uint64_t tweak_fwrite(uint64_t fd, uint32_t sz, void* p_data) {
  (void) fd;
  (void) sz;
  (void) p_data;
  TWEAK_LOG_WARN("tweak_fwrite not implemented");
  return 0;
}

uint64_t tweak_fread(uint64_t fd, uint32_t sz, void* p_data) {
  (void) fd;
  (void) sz;
  (void) p_data;
  TWEAK_LOG_WARN("tweak_fread not implemented");
  return 0;
}

uint64_t tweak_config_fopen(const char* name, const char* mode) {
  (void) name;
  (void) mode;
  TWEAK_LOG_WARN("tweak_config_fopen not implemented");
  return 0;
}

uint64_t tweak_config_fclose(uint64_t fd) {
  (void) fd;
  TWEAK_LOG_WARN("tweak_config_fclose not implemented");
  return 0;
}

uint64_t tweak_config_add(uint32_t sz, void* p_data) {
  (void) sz;
  (void) p_data;
  TWEAK_LOG_WARN("tweak_config_add not implemented");
  return 0;
}

void tweak_get_file_path(char* dst, const char* mask, uint32_t read) {
  (void) dst;
  (void) mask;
  (void) read;
  TWEAK_LOG_WARN("tweak_get_file_path not implemented");
}

uint32_t tweak_json_config_read(void* p_data, uint32_t max_sz, int cfg_enum, const char* cfg_name_ver, const char* path) {
  (void) p_data;
  (void) max_sz;
  (void) cfg_enum;
  (void) cfg_name_ver;
  (void) path;
  TWEAK_LOG_WARN("tweak_json_config_read not implemented");
  return 0;
}

uint32_t tweak_json_config_write(void* p_data, uint32_t max_sz, int cfg_enum, const char* cfg_name_ver, const char* path) {
  (void) p_data;
  (void) max_sz;
  (void) cfg_enum;
  (void) cfg_name_ver;
  (void) path;
  TWEAK_LOG_WARN("tweak_json_config_write  not implemented");
  return 0;
}
