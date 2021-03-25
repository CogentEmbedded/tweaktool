/**
 * @file tweakvariant.c
 * @ingroup tweak-api
 *
 * @brief Tweak variant type, data conversion helpers.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-app-implementation Implementation of tweak-app component.
 */

#include <tweak2/log.h>
#include <tweak2/variant.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static tweak_variant_type_conversion_result
  parse_bool(const char* arg, tweak_variant* out);

static tweak_variant_type_conversion_result
  parse_signed_int64(const char *s, int64_t* out);

static tweak_variant_type_conversion_result
  parse_generic_int( const char* arg,
                     tweak_variant_type target_type,
                     tweak_variant* out);

static tweak_variant_type_conversion_result
  parse_unsigned_int64(const char *s, uint64_t* out);

static tweak_variant_type_conversion_result
  parse_generic_uint(const char* arg,
                     tweak_variant_type target_type,
                     tweak_variant* out);

static tweak_variant_type_conversion_result
  parse_floating_point(const char *s, double* out);

static tweak_variant_type_conversion_result
  parse_generic_float(const char* arg,
                      tweak_variant_type target_type,
                      tweak_variant* out);

tweak_variant_type_conversion_result tweak_variant_from_string(const char* arg,
  tweak_variant_type target_type, tweak_variant* out)
{
  assert(out->type == TWEAK_VARIANT_TYPE_IS_NULL);
  switch(target_type) {
  case TWEAK_VARIANT_TYPE_BOOL:
    return parse_bool(arg, out);
  case TWEAK_VARIANT_TYPE_SINT8:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT8, out);
  case TWEAK_VARIANT_TYPE_SINT16:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT16, out);
  case TWEAK_VARIANT_TYPE_SINT32:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT32, out);
  case TWEAK_VARIANT_TYPE_SINT64:
    return parse_generic_int(arg, TWEAK_VARIANT_TYPE_SINT64, out);
  case TWEAK_VARIANT_TYPE_UINT8:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT8, out);
  case TWEAK_VARIANT_TYPE_UINT16:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT16, out);
  case TWEAK_VARIANT_TYPE_UINT32:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT32, out);
  case TWEAK_VARIANT_TYPE_UINT64:
    return parse_generic_uint(arg, TWEAK_VARIANT_TYPE_UINT64, out);
  case TWEAK_VARIANT_TYPE_FLOAT:
    return parse_generic_float(arg, TWEAK_VARIANT_TYPE_FLOAT, out);
  case TWEAK_VARIANT_TYPE_DOUBLE:
    return parse_generic_float(arg, TWEAK_VARIANT_TYPE_DOUBLE, out);
  default:
    break; /* return null variant */
  }
  return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
}

tweak_variant_string tweak_variant_to_string(const tweak_variant* arg) {
  tweak_variant_string result = {};
  char buff[128];
  switch (arg->type) {
  case TWEAK_VARIANT_TYPE_IS_NULL:
    tweak_variant_assign_string(&result, "null");
    break;
  case TWEAK_VARIANT_TYPE_BOOL:
    tweak_variant_assign_string(&result, arg->value_bool ? "true" : "false");
    break;
  case TWEAK_VARIANT_TYPE_SINT8:
    sprintf(buff, "%d", (int)arg->sint8);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_SINT16:
    sprintf(buff, "%d", (int)arg->sint16);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    sprintf(buff, "%d", (int)arg->sint32);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_SINT64:
    sprintf(buff, "%" PRIi64 "", arg->sint64);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT8:
    sprintf(buff, "%u", (unsigned)arg->uint8);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT16:
    sprintf(buff, "%u", (unsigned)arg->uint16);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT32:
    sprintf(buff, "%u", (unsigned)arg->uint32);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT64:
    sprintf(buff, "%" PRIu64 "", arg->uint64);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_FLOAT:
    sprintf(buff, "%f", arg->fp32);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_DOUBLE:
    sprintf(buff, "%f", arg->fp64);
    tweak_variant_assign_string(&result, buff);
    break;
  default:
    sprintf(buff, "{invalid_type_%d:???}", arg->type);
    tweak_variant_assign_string(&result, buff);
    break;
  }
  return result;
}

tweak_variant_string tweak_variant_to_json(const tweak_variant* arg) {
  tweak_variant_string result = {};
  char buff[128];
  switch (arg->type) {
  case TWEAK_VARIANT_TYPE_IS_NULL:
    tweak_variant_assign_string(&result, "null");
    break;
  case TWEAK_VARIANT_TYPE_BOOL:
    tweak_variant_assign_string(&result,
      arg->value_bool ? "{\"bool\":true}" : "{\"bool\":false}");
    break;
  case TWEAK_VARIANT_TYPE_SINT8:
    sprintf(buff, "{\"sint8\":%d}", (int)arg->sint8);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_SINT16:
    sprintf(buff, "{\"sint16\":%d}", (int)arg->sint16);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    sprintf(buff, "{\"sint32\":%d}", (int)arg->sint32);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_SINT64:
    sprintf(buff, "{\"sint64\":%" PRIi64 "}", arg->sint64);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT8:
    sprintf(buff, "{\"uint8\":%u}", (unsigned)arg->uint8);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT16:
    sprintf(buff, "{\"uint16\":%u}", (unsigned)arg->uint16);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT32:
    sprintf(buff, "{\"uint32\":%u}", (unsigned)arg->uint32);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_UINT64:
    sprintf(buff, "{\"uint64\":%" PRIu64 "}", arg->uint64);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_FLOAT:
    sprintf(buff, "{\"float\":%f}", arg->fp32);
    tweak_variant_assign_string(&result, buff);
    break;
  case TWEAK_VARIANT_TYPE_DOUBLE:
    sprintf(buff, "{\"double\":%f}", arg->fp64);
    tweak_variant_assign_string(&result, buff);
    break;
  default:
    sprintf(buff, "{\"invalid_type\":%d}", arg->type);
    tweak_variant_assign_string(&result, buff);
    break;
  }
  return result;
}


static tweak_variant_type_conversion_result
  parse_bool(const char* arg, tweak_variant* out)
{
  if (strcasecmp(arg, "true") == 0 || strcmp(arg, "1") == 0) {
    out->type = TWEAK_VARIANT_TYPE_BOOL;
    out->value_bool = true;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else if (strcasecmp(arg, "false") == 0 || strcmp(arg, "0") == 0) {
    out->type = TWEAK_VARIANT_TYPE_BOOL;
    out->value_bool = false;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

static tweak_variant_type_conversion_result
  parse_signed_int64(const char *s, int64_t* out)
{
  int64_t i;
  char* endp;
  i = strtol(s, &endp, 10);
  if (endp != s && *endp == '\0') {
    *out = i;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

static inline bool can_handle_conversion_result(
  tweak_variant_type_conversion_result conversion_result)
{
  return conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS
      || conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;
}

#define COERCE_INT(VARIANT_FIELD, VARIANT_TYPE, VARIANT_TYPE_MIN, VARIANT_TYPE_MAX) \
  if (val >= VARIANT_TYPE_MIN && val <= VARIANT_TYPE_MAX) {                         \
    result.VARIANT_FIELD = (VARIANT_TYPE) val;                                      \
  } else if (val < VARIANT_TYPE_MIN) {                                              \
    result.VARIANT_FIELD = VARIANT_TYPE_MIN;                                        \
    conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;       \
  } else if (val > VARIANT_TYPE_MAX) {                                              \
    result.VARIANT_FIELD = VARIANT_TYPE_MAX;                                        \
    conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;       \
  }

static tweak_variant_type_conversion_result
  parse_generic_int(const char* arg,
                    tweak_variant_type target_type,
                    tweak_variant* out)
{
  int64_t val;
  tweak_variant_type_conversion_result conversion_result;
  tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
  conversion_result = parse_signed_int64(arg, &val);
  if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
    switch (target_type) {
    case TWEAK_VARIANT_TYPE_SINT8:
      result.type = TWEAK_VARIANT_TYPE_SINT8;
      COERCE_INT(sint8, int8_t, INT8_MIN, INT8_MAX);
      break;
    case TWEAK_VARIANT_TYPE_SINT16:
      result.type = TWEAK_VARIANT_TYPE_SINT16;
      COERCE_INT(sint16, int16_t, INT16_MIN, INT16_MAX);
      break;
    case TWEAK_VARIANT_TYPE_SINT32:
      result.type = TWEAK_VARIANT_TYPE_SINT32;
      COERCE_INT(sint32, int32_t, INT32_MIN, INT32_MAX);
      break;
    case TWEAK_VARIANT_TYPE_SINT64:
      result.type = TWEAK_VARIANT_TYPE_SINT64;
      result.sint64 = val;
      break;
    default:
      TWEAK_FATAL("Unknown signed integer type: %d", target_type);
      break;
    }
  }
  if (can_handle_conversion_result(conversion_result)) {
    *out = result;
  }
  return conversion_result;
}

static tweak_variant_type_conversion_result
  parse_unsigned_int64(const char *s, uint64_t* out)
{
  uint64_t u;
  char* endp;
  u = strtoul(s, &endp, 10);
  if (endp != s && *endp == '\0') {
    *out = u;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

#define COERCE_UINT(VARIANT_FIELD, VARIANT_TYPE, VARIANT_TYPE_MAX)                  \
  if (val <= VARIANT_TYPE_MAX)  {                                                   \
    result.VARIANT_FIELD = (VARIANT_TYPE) val;                                      \
  } else  {                                                                         \
    result.VARIANT_FIELD = VARIANT_TYPE_MAX;                                        \
    conversion_result = TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED;       \
  }

static tweak_variant_type_conversion_result
  parse_generic_uint(const char* arg,
                     tweak_variant_type target_type,
                     tweak_variant* out)
{
  uint64_t val;
  tweak_variant_type_conversion_result conversion_result;
  tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
  conversion_result = parse_unsigned_int64(arg, &val);
  if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
    switch (target_type) {
    case TWEAK_VARIANT_TYPE_UINT8:
      result.type = TWEAK_VARIANT_TYPE_UINT8;
      COERCE_UINT(uint8, uint8_t, UINT8_MAX);
      break;
    case TWEAK_VARIANT_TYPE_UINT16:
      result.type = TWEAK_VARIANT_TYPE_UINT16;
      COERCE_UINT(uint16, uint16_t, UINT16_MAX);
      break;
    case TWEAK_VARIANT_TYPE_UINT32:
      result.type = TWEAK_VARIANT_TYPE_UINT32;
      COERCE_UINT(uint32, uint32_t, UINT32_MAX);
      break;
    case TWEAK_VARIANT_TYPE_UINT64:
      result.type = TWEAK_VARIANT_TYPE_UINT64;
      result.uint64 = val;
      break;
    default:
      TWEAK_FATAL("Unknown signed integer type: %d", target_type);
      break;
    }
  }
  if (can_handle_conversion_result(conversion_result)) {
    *out = result;
  }
  return conversion_result;
}

static tweak_variant_type_conversion_result
  parse_floating_point(const char *s, double* out)
{
  double d;
  char* endp;
  d = strtod(s, &endp);
  if (endp != s && *endp == '\0') {
    *out = d;
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS;
  } else {
    return TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_FAIL;
  }
}

static tweak_variant_type_conversion_result
  parse_generic_float(const char* arg,
                      tweak_variant_type target_type,
                      tweak_variant* out)
{
  double val;
  tweak_variant_type_conversion_result conversion_result;
  tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
  conversion_result = parse_floating_point(arg, &val);
  if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
    switch (target_type) {
    case TWEAK_VARIANT_TYPE_FLOAT:
      result.type = TWEAK_VARIANT_TYPE_FLOAT;
      result.fp32 = (float) val;
      break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
      result.type = TWEAK_VARIANT_TYPE_DOUBLE;
      result.fp64 = val;
      break;
    default:
      TWEAK_FATAL("Unknown signed integer type: %d", target_type);
      break;
    }
  }
  if (can_handle_conversion_result(conversion_result)) {
    *out = result;
  }
  return conversion_result;
}
