/**
 * @file tweakappfeatures.c
 * @ingroup tweak-internal
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
#include <tweak2/json.h>
#include <errno.h>
#include <inttypes.h>

#include "tweakappfeatures.h"

void tweak_app_features_init_minimal(struct tweak_app_features* features) {
  features->vectors = false;
}

void tweak_app_features_init_default(struct tweak_app_features* features) {
  features->vectors = true;
}

bool tweak_app_features_from_json(const tweak_variant_string* json, struct tweak_app_features* out_result) {
  assert(json);
  assert(out_result);

  bool result = false;
  struct tweak_json_node* doc = tweak_json_parse(tweak_variant_string_c_str(json));
  const struct tweak_json_node* vector_node;

  if (tweak_json_get_type(doc) != TWEAK_JSON_NODE_TYPE_OBJECT) {
    TWEAK_LOG_WARN("Can't parse json snippet: %s", tweak_variant_string_c_str(json));
    goto error;
  }

  tweak_app_features_init_default(out_result);
  vector_node = tweak_json_get_object_field(doc, "vectors", TWEAK_JSON_NODE_TYPE_BOOL);
  if (vector_node) {
    out_result->vectors = (strcmp("true", tweak_json_node_as_c_str(vector_node)) == 0);
  }

  result = true;
error:
  tweak_json_destroy(doc);
  return result;
}

tweak_variant_string tweak_app_features_to_json(const struct tweak_app_features* arg) {
  tweak_variant_string result = TWEAK_VARIANT_STRING_EMPTY;
  char buff[128];
  snprintf(buff, sizeof(buff), "{\"vectors\": %s}", arg->vectors ? "true" : "false");
  tweak_assign_string(&result, buff);
  return result;
}

bool tweak_app_features_check_type_compatibility(const struct tweak_app_features* arg, tweak_variant_type type) {
  switch (type){
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
    return true;
  case TWEAK_VARIANT_TYPE_STRING:
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
    return arg->vectors;
  }
  TWEAK_FATAL("Unknown type: %d", type);
  return false;
}
