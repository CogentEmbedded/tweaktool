/**
 * @file tweakcompat.c
 * @ingroup tweak-api
 * @brief part of tweak2 - tweak1 compatibility layer implementation.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak.h>
#include <tweak2/appserver.h>
#include <tweak2/log.h>
#include <tweak2/types.h>
#include <tweak2/variant.h>

#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

static tweak_app_server_context s_context = NULL;

#define MAX_BUFFER_SIZE 256

static pthread_mutex_t s_log_lock = { };

static pthread_mutex_t s_callback_lock = { };

static tweak_update_handler s_callback = NULL;

static void* s_callback_cookie = NULL;

static uint32_t s_layout_id = 0L;

static char* s_layout = NULL;

static void* s_item_change_listener_cookie = NULL;

static void print_event(FILE* file, const char* format, int line, const char* source, ...) {
  char message[MAX_BUFFER_SIZE] = { 0 };
  va_list args;
  va_start (args, source);
  int char_count = vsnprintf(message, sizeof(message), format, args);
  message[char_count] = '\0';
  va_end (args);
  pthread_mutex_lock(&s_log_lock);
  fprintf(file, "LOG_ERROR file: %s line: %d | %s\n", source, line, message);
  pthread_mutex_unlock(&s_log_lock);
}

void tweak_on_update(const char* name) {
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
  pthread_mutex_lock(&s_callback_lock);
  callback = s_callback;
  callback_cookie = s_callback_cookie;
  pthread_mutex_unlock(&s_callback_lock);
  if (callback) {
    tweak_app_item_snapshot* snapshot = tweak_app_item_get_snapshot(s_context, id);
    callback(tweak_variant_string_c_str(&snapshot->uri), callback_cookie);
    tweak_app_release_snapshot(s_context, snapshot);
  }
}

void tweak_set_update_handler(tweak_update_handler handler, void* cookie) {
  pthread_mutex_lock(&s_callback_lock);
  s_callback = handler;
  s_callback_cookie = cookie;
  pthread_mutex_unlock(&s_callback_lock);
}

int tweak_connect(void) {
  pthread_mutex_init(&s_log_lock, NULL);
  pthread_mutex_init(&s_callback_lock, NULL);

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
  "\"layout_id\"=%u,"
  "\"width\"=%u,"
  "\"horizontal_vertical\"=%u,"
  "\"layout_name=\"%s\""
  "}";

void tweak_add_layout(unsigned int width, unsigned int horizontal_vertical, const char* name) {
  char layout_buff[256] = {};
  ++s_layout_id;
  snprintf(layout_buff, sizeof(layout_buff) - 1, LAYOUT_META_TEMPLATE,
    s_layout_id, width, horizontal_vertical, name);
  char* old_layout = s_layout;
  s_layout = strdup(layout_buff);
  free(old_layout);
}

void tweak_close() {
  if (!s_context) {
    TWEAK_LOG_ERROR("Library hasn't been initialized correctly");
    return;
  }

  tweak_app_destroy_context(s_context);
  pthread_mutex_destroy(&s_callback_lock);
  pthread_mutex_destroy(&s_log_lock);

  s_context = NULL;

  free(s_layout);
  s_layout = NULL;
}

static const char* META_TEMPLATE_WITH_LAYOUT = "{"
  "\"control\": \"%s\","
  "\"min\": %f,"
  "\"max\": %f,"
  "\"readonly\": false,"
  "\"decimals\": %u,"
  "\"layout\": %s"
  "}";

static const char* META_TEMPLATE_WITHOUT_LAYOUT = "{"
  "\"control\": \"%s\","
  "\"min\": %f,"
  "\"max\": %f,"
  "\"readonly\": false,"
  "\"decimals\": %u"
  "}";

void tweak_add_slider(const char* name, double minv, double maxv, double def, unsigned int precision) {
  char meta_buff[256] = {};

  if (s_layout) {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      META_TEMPLATE_WITH_LAYOUT,
      "slider", minv, maxv, precision, s_layout);
  } else {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      META_TEMPLATE_WITHOUT_LAYOUT,
      "slider", minv, maxv, precision);
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_create_double(&value, def);

  tweak_id id = tweak_app_server_add_item(s_context,
    name, name, meta_buff, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

void tweak_add_spinbox(const char* name, double minv, double maxv, double def, unsigned int precision) {
  char meta_buff[256] = {};
  if (s_layout) {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      META_TEMPLATE_WITH_LAYOUT,
      "spinbox", minv, maxv, precision, s_layout);
  } else {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      META_TEMPLATE_WITHOUT_LAYOUT,
      "spinbox", minv, maxv, precision);
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_create_double(&value, def);

  tweak_id id = tweak_app_server_add_item(s_context,
    name, name, meta_buff, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

static const char* BOOL_META_TEMPLATE_WITH_LAYOUT = "{"
  "\"control\": \"%s\","
  "\"min\": false,"
  "\"max\": true,"
  "\"readonly\": false,"
  "\"layout\": %s"
  "}";

static const char* BOOL_META_TEMPLATE_WITHOUT_LAYOUT = "{"
  "\"control\": \"%s\","
  "\"min\": false,"
  "\"max\": true,"
  "\"readonly\": false"
  "}";

void tweak_add_checkbox(const char* name, int def_val) {
  char meta_buff[256] = {};
  if (s_layout) {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      BOOL_META_TEMPLATE_WITH_LAYOUT, "checkbox", s_layout);
  } else {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      BOOL_META_TEMPLATE_WITHOUT_LAYOUT, "checkbox");
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_create_bool(&value, def_val != 0);

  tweak_id id = tweak_app_server_add_item(s_context,
    name, name, meta_buff, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

void tweak_add_button(const char* name) {
  char meta_buff[256] = {};
  if (s_layout) {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      BOOL_META_TEMPLATE_WITH_LAYOUT, "button", s_layout);
  } else {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      BOOL_META_TEMPLATE_WITHOUT_LAYOUT, "button");
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_create_bool(&value, false);

  tweak_id id = tweak_app_server_add_item(s_context,
    name, name, meta_buff, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

static const char* RADIO_BUTTON_META_TEMPLATE_WITH_LAYOUT = "{"
  "\"control\": \"radio\","
  "\"desc\": \"%s\","
  "\"readonly\": false,"
  "\"layout\": %s"
  "}";

static const char* RADIO_BUTTON_META_TEMPLATE_WITHOUT_LAYOUT = "{"
  "\"control\": \"radio\","
  "\"desc\": \"%s\","
  "\"readonly\": false"
  "}";

void tweak_add_groupbox(const char* name, const char* desc, unsigned int def) {
  char meta_buff[256] = {};
  if (s_layout) {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      RADIO_BUTTON_META_TEMPLATE_WITH_LAYOUT, desc, s_layout);
  } else {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      RADIO_BUTTON_META_TEMPLATE_WITHOUT_LAYOUT, desc);
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_create_uint32(&value, def);

  tweak_id id = tweak_app_server_add_item(s_context,
    name, name, meta_buff, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

void tweak_add_widget(const char* name) {
  char meta_buff[256] = {};
  if (s_layout) {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      BOOL_META_TEMPLATE_WITH_LAYOUT, "widget", s_layout);
  } else {
    snprintf(meta_buff, sizeof(meta_buff) - 1,
      BOOL_META_TEMPLATE_WITHOUT_LAYOUT, "widget");
  }

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_create_bool(&value, false);

  tweak_id id = tweak_app_server_add_item(s_context,
    name, name, meta_buff, &value, NULL);

  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("Can't add control, duplicate name");
  }
}

double tweak_get(const char* name, double defval) {
  tweak_id id = tweak_app_find_id(s_context, name);
  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("No tweak with name: %s", name);
    return defval;
  }
  tweak_variant variant_value = {
    .type = TWEAK_VARIANT_TYPE_IS_NULL
  };
  double result = defval;
  tweak_app_error_code error_code = tweak_app_item_clone_current_value((tweak_app_context)s_context, id, &variant_value);
  if (error_code == TWEAK_APP_SUCCESS) {
    switch (variant_value.type) {
    case TWEAK_VARIANT_TYPE_BOOL:
      result = variant_value.value_bool;
      break;
    case TWEAK_VARIANT_TYPE_SINT8:
      result = variant_value.sint8;
      break;
    case TWEAK_VARIANT_TYPE_SINT16:
      result = variant_value.sint16;
      break;
    case TWEAK_VARIANT_TYPE_SINT32:
      result = variant_value.sint32;
      break;
    case TWEAK_VARIANT_TYPE_SINT64:
      result = variant_value.sint64;
      break;
    case TWEAK_VARIANT_TYPE_UINT8:
      result = variant_value.uint8;
      break;
    case TWEAK_VARIANT_TYPE_UINT16:
      result = variant_value.uint16;
      break;
    case TWEAK_VARIANT_TYPE_UINT32:
      result = variant_value.uint32;
      break;
    case TWEAK_VARIANT_TYPE_UINT64:
      result = variant_value.uint64;
      break;
    case TWEAK_VARIANT_TYPE_FLOAT:
      result = variant_value.fp32;
      break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
      result = variant_value.fp64;
      break;
    }
  } else {
    TWEAK_LOG_ERROR("tweak_app_item_clone_current_value returned 0x%x", result);
    return defval;
  }
  return result;
}

void tweak_set(const char* name, double val) {
  tweak_id id = tweak_app_find_id(s_context, name);
  if (id == TWEAK_INVALID_ID) {
    TWEAK_LOG_ERROR("No tweak with name: %s", name);
    return;
  }
  tweak_variant_type type = tweak_app_item_get_type(s_context, id);
  if (type != TWEAK_VARIANT_TYPE_IS_NULL) {
    tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
    switch (type) {
    case TWEAK_VARIANT_TYPE_BOOL:
      tweak_variant_create_bool(&value, val != 0);
      break;
    case TWEAK_VARIANT_TYPE_SINT8:
      tweak_variant_create_sint8(&value, (int8_t) val);
      break;
    case TWEAK_VARIANT_TYPE_SINT16:
      tweak_variant_create_sint16(&value, (int16_t) val);
      break;
    case TWEAK_VARIANT_TYPE_SINT32:
      tweak_variant_create_sint32(&value, (int32_t) val);
      break;
    case TWEAK_VARIANT_TYPE_SINT64:
      tweak_variant_create_sint64(&value, (int64_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT8:
      tweak_variant_create_uint8(&value, (uint8_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT16:
      tweak_variant_create_uint16(&value, (uint16_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT32:
      tweak_variant_create_uint32(&value, (uint32_t) val);
      break;
    case TWEAK_VARIANT_TYPE_UINT64:
      tweak_variant_create_uint64(&value, (uint64_t) val);
      break;
    case TWEAK_VARIANT_TYPE_FLOAT:
      tweak_variant_create_float(&value, (float) val);
      break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
      tweak_variant_create_double(&value, val);
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
  TWEAK_FATAL("tweak_get_string not implemented");
  return 0;
}


uint64_t tweak_fopen(const char* name, const char* mode) {
  (void) name;
  (void) mode;
  TWEAK_FATAL("tweak_get_string not implemented");
  return 0;
}

uint64_t tweak_fclose(uint64_t fd) {
  (void) fd;
  TWEAK_FATAL("tweak_fclose not implemented");
  return 0;
}

uint64_t tweak_ftell(uint64_t fd) {
  (void) fd;
  TWEAK_FATAL("tweak_ftell not implemented");
  return 0;
}

uint64_t tweak_fseek(uint64_t fd, int32_t offset, int32_t where){
  (void) fd;
  (void) offset;
  (void) where;
  TWEAK_FATAL("tweak_fseek not implemented");
  return 0;
}

uint64_t tweak_fwrite(uint64_t fd, uint32_t sz, void* p_data) {
  (void) fd;
  (void) sz;
  (void) p_data;
  TWEAK_FATAL("tweak_fwrite not implemented");
  return 0;
}

uint64_t tweak_fread(uint64_t fd, uint32_t sz, void* p_data) {
  (void) fd;
  (void) sz;
  (void) p_data;
  TWEAK_FATAL("tweak_fread not implemented");
  return 0;
}

uint64_t tweak_config_fopen(const char* name, const char* mode) {
  (void) name;
  (void) mode;
  TWEAK_FATAL("tweak_config_fopen not implemented");
  return 0;
}

uint64_t tweak_config_fclose(uint64_t fd) {
  (void) fd;
  TWEAK_FATAL("tweak_config_fclose not implemented");
  return 0;
}

uint64_t tweak_config_add(uint32_t sz, void* p_data) {
  (void) sz;
  (void) p_data;
  TWEAK_FATAL("tweak_config_add not implemented");
  return 0;
}

void tweak_get_file_path(char* dst, const char* mask, uint32_t read) {
  (void) dst;
  (void) mask;
  (void) read;
  TWEAK_FATAL("tweak_get_file_path not implemented");
}

uint32_t tweak_json_config_read(void* p_data, uint32_t max_sz, int cfg_enum, const char* cfg_name_ver, const char* path) {
  (void) p_data;
  (void) max_sz;
  (void) cfg_enum;
  (void) cfg_name_ver;
  (void) path;
  TWEAK_FATAL("tweak_json_config_read not implemented");
  return 0;
}

uint32_t tweak_json_config_write(void* p_data, uint32_t max_sz, int cfg_enum, const char* cfg_name_ver, const char* path) {
  (void) p_data;
  (void) max_sz;
  (void) cfg_enum;
  (void) cfg_name_ver;
  (void) path;
  TWEAK_FATAL("tweak_json_config_write  not implemented");
  return 0;
}
