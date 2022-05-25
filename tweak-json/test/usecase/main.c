/**
 * @file main.c
 * @ingroup tweak-api
 * @brief test suite for common library.
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

#include <tweak2/string.h>
#include <tweak2/json.h>

#include <acutest.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

static double convert_simple_to_double(const struct tweak_json_node* simple, bool* success)  {
  bool success0 = false;
  double d = NAN;
  if (tweak_json_get_type(simple) == TWEAK_JSON_NODE_TYPE_NUMBER) {
    char* endp;
    const char* p = tweak_json_node_as_c_str(simple);
    d = strtod(p, &endp);
    if (endp != p && *endp == '\0') {
        success0 = true;
    }
  }
  if (success != NULL) {
    *success = success0;
  }
  return success0 ? d : 0;
}

static int32_t convert_simple_to_int32(const struct tweak_json_node* simple, bool* success)  {
  bool success0 = false;
  int64_t i = 0;
  if (tweak_json_get_type(simple) == TWEAK_JSON_NODE_TYPE_NUMBER) {
    char* endp;
    const char* p = tweak_json_node_as_c_str(simple);
    i = strtol(p, &endp, 10);
    if (endp != p && *endp == '\0' && i >= INT_MIN && i <= INT_MAX) {
      success0 = true;
    }
  }
  if (success != NULL) {
    *success = success0;
  }
  return success0 ? (int32_t)i : 0;
}

static bool convert_simple_to_bool(const struct tweak_json_node* simple, bool* success)  {
  bool success0 = false;
  bool b = false;
  if (tweak_json_get_type(simple) == TWEAK_JSON_NODE_TYPE_BOOL) {
    const char* p = tweak_json_node_as_c_str(simple);
    if (strcmp("true", p) == 0) {
      b = true;
      success0 = true;
    } else if (strcmp("false", p) == 0) {
      b = false;
      success0 = true;
    }
  }
  if (success != NULL) {
    *success = success0;
  }
  return success0 ? b : false;
}

void test_json_1(void) {
    struct tweak_json_node* doc = tweak_json_parse(NULL);
    TEST_CHECK(doc == NULL);
}

void test_json_11(void) {
    struct tweak_json_node* doc = tweak_json_parse("\"BoZo\"");
    TEST_CHECK(doc != NULL);
    TEST_CHECK(tweak_json_get_type(doc) == TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(strcmp(tweak_json_node_as_c_str(doc), "BoZo") == 0);
    tweak_json_destroy(doc);
}

void test_json_12_check_X_string(struct tweak_json_node* array, int index, const char* value);

void test_json_12(void) {
    struct tweak_json_node* doc = tweak_json_parse("[\"Alpha\", \"Bravo\", \"Charlie\", \"Foxtrot\"]");
    TEST_CHECK(doc != NULL);
    TEST_CHECK(tweak_json_get_type(doc) == TWEAK_JSON_NODE_TYPE_ARRAY);
    size_t size = 0;
    tweak_json_get_array_size_status status = tweak_json_get_array_size(doc, &size);
    TEST_CHECK(status == TWEAK_JSON_GET_SIZE_SUCCESS);
    TEST_CHECK(size == 4);
    test_json_12_check_X_string(doc, 0, "Alpha");
    test_json_12_check_X_string(doc, 1, "Bravo");
    test_json_12_check_X_string(doc, 2, "Charlie");
    test_json_12_check_X_string(doc, 3, "Foxtrot");
    tweak_json_destroy(doc);
}

void test_json_12_check_X_string(struct tweak_json_node* array, int index, const char* value) {
    const struct tweak_json_node* node = tweak_json_get_array_item(array, index, TWEAK_JSON_NODE_TYPE_VALUE);
    TEST_CHECK(node != NULL);
    TEST_CHECK(tweak_json_get_type(node) == TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(strcmp(tweak_json_node_as_c_str(node), value) == 0);
}

/* Missing comma after first item in options array */
const char* JSON1e_1 = "{\"options\": [{\"value\": 100, \"text\": \"Foo\"}"
"                                      {\"value\": 110, \"text\": \"Bar\"},"
"                                      {\"value\": 120, \"text\":  \"Baz\"}]"
"};";

void test_json_1e_1(void) {
  struct tweak_json_node* doc = tweak_json_parse(JSON1e_1);
  TEST_CHECK(doc == NULL);
}

/* Missing closing square bracket of item's array */
const char* JSON1e_2 = "{\"options\": [{\"value\": 100, \"text\": \"Foo\"}};";

void test_json_1e_2(void) {
  struct tweak_json_node* doc = tweak_json_parse(JSON1e_2);
  TEST_CHECK(doc == NULL);
}

/* Missing opening curly bracket */
const char* JSON1e_3 = "\"options\": [{\"value\": 100, \"text\": \"Foo\"}};";

void test_json_1e_3(void) {
  struct tweak_json_node* doc = tweak_json_parse(JSON1e_2);
  TEST_CHECK(doc == NULL);
}

/* Missing quote pair*/
const char* JSON1e_4 = "\"BoZo";

void test_json_1e_4(void) {
  struct tweak_json_node* doc = tweak_json_parse(JSON1e_3);
  TEST_CHECK(doc == NULL);
}

/* Missing quote pair*/
const char* JSON1e_5 = "{\"}";

void test_json_1e_5(void) {
  struct tweak_json_node* doc = tweak_json_parse(JSON1e_4);
  TEST_CHECK(doc == NULL);
}

const char* JSON2 = "{\"options\": [{\"value\": 100, \"text\": \"Foo\"},"
"                                   {\"value\": 110, \"text\": \"Bar\"},"
"                                   {\"value\": 120, \"text\": \"Baz\"}]"
"                    }";

static void test_json_2_node_X(const struct tweak_json_node* array, int index, int value, const char* text) {
    (void)text;
    const struct tweak_json_node* node = tweak_json_get_array_item(array, index, TWEAK_JSON_NODE_TYPE_ANY);
    TEST_CHECK(node != NULL);
    TEST_CHECK(tweak_json_get_type(node) == TWEAK_JSON_NODE_TYPE_OBJECT);
    const struct tweak_json_node* ill;
    ill = tweak_json_get_object_field(node, "value", TWEAK_JSON_NODE_TYPE_BOOL);
    TEST_CHECK(!ill);
    const struct tweak_json_node* value_field = tweak_json_get_object_field(node, "value", TWEAK_JSON_NODE_TYPE_NUMBER);
    TEST_CHECK(tweak_json_get_type(value_field) == TWEAK_JSON_NODE_TYPE_NUMBER);
    bool success;
    int value0 = convert_simple_to_int32(value_field, &success);
    TEST_CHECK(success);
    TEST_CHECK(value == value0);
    ill = tweak_json_get_object_field(node, "text", TWEAK_JSON_NODE_TYPE_BOOL);
    TEST_CHECK(!ill);
    const struct tweak_json_node* text_field = tweak_json_get_object_field(node, "text", TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(tweak_json_get_type(text_field) == TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(strcmp(tweak_json_node_as_c_str(text_field), text) == 0);
}

void test_json_2(void) {
    struct tweak_json_node* doc = tweak_json_parse(JSON2);
    TEST_CHECK(doc != NULL);
    TEST_CHECK(tweak_json_get_type(doc) == TWEAK_JSON_NODE_TYPE_OBJECT);
    TEST_CHECK(tweak_json_get_object_field(doc, "opt1ons", TWEAK_JSON_NODE_TYPE_ANY) == NULL);
    TEST_CHECK(tweak_json_get_object_field(doc, "options", TWEAK_JSON_NODE_TYPE_OBJECT) == NULL);
    const struct tweak_json_node* options_field =
      tweak_json_get_object_field(doc, "options", TWEAK_JSON_NODE_TYPE_COMPOUND);
    TEST_CHECK(options_field != NULL);
    TEST_CHECK(tweak_json_get_type(options_field) == TWEAK_JSON_NODE_TYPE_ARRAY);

    size_t size = SIZE_MAX;
    tweak_json_get_array_size_status status;
    status = tweak_json_get_array_size(NULL, &size);
    TEST_CHECK(status == TWEAK_JSON_GET_SIZE_INVALID_ARG);

    status = tweak_json_get_array_size(options_field, NULL);
    TEST_CHECK(status == TWEAK_JSON_GET_SIZE_INVALID_ARG);

    status = tweak_json_get_array_size(options_field, &size);
    TEST_CHECK(status == TWEAK_JSON_GET_SIZE_SUCCESS);

    TEST_CHECK(size == 3);
    test_json_2_node_X(options_field, 0, 100, "Foo");
    test_json_2_node_X(options_field, 1, 110, "Bar");
    test_json_2_node_X(options_field, 2, 120, "Baz");
    TEST_CHECK(tweak_json_get_array_item(options_field, 3, TWEAK_JSON_NODE_TYPE_ANY) == NULL);
    tweak_json_destroy(doc);
}

static void check_string_field(struct tweak_json_node* obj,
    const char* field, const char* expected_value)
{
    const struct tweak_json_node* field_node;
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_NUMBER | TWEAK_JSON_NODE_TYPE_BOOL);
    TEST_CHECK(field_node == NULL);
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(field_node != NULL);
    TEST_CHECK(tweak_json_get_type(field_node) == TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(strcmp(tweak_json_node_as_c_str(field_node), expected_value) == 0);
}

static void check_double_field(struct tweak_json_node* obj,
    const char* field, double expected_value)
{
    const struct tweak_json_node* field_node;
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_STRING | TWEAK_JSON_NODE_TYPE_BOOL);
    TEST_CHECK(field_node == NULL);
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_NUMBER);
    TEST_CHECK(field_node != NULL);
    TEST_CHECK(tweak_json_get_type(field_node) == TWEAK_JSON_NODE_TYPE_NUMBER);
    bool coerce_success;
    double value =  convert_simple_to_double(field_node, &coerce_success);
    TEST_CHECK(coerce_success);
    TEST_CHECK(value == expected_value);
}

static void check_bool_field(const struct tweak_json_node* obj,
    const char* field, bool expected_value)
{
    const struct tweak_json_node* field_node;
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_STRING | TWEAK_JSON_NODE_TYPE_NUMBER);
    TEST_CHECK(field_node == NULL);
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_BOOL);
    TEST_CHECK(field_node != NULL);
    TEST_CHECK(tweak_json_get_type(field_node) == TWEAK_JSON_NODE_TYPE_BOOL);
    bool coerce_success;
    bool value = convert_simple_to_bool(field_node, &coerce_success);
    TEST_CHECK(coerce_success);
    TEST_CHECK(value == expected_value);
}

static void check_int32_field(const struct tweak_json_node* obj,
    const char* field, int expected_value)
{
    const struct tweak_json_node* field_node;
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_STRING | TWEAK_JSON_NODE_TYPE_BOOL);
    TEST_CHECK(field_node == NULL);
    field_node = tweak_json_get_object_field(obj, field, TWEAK_JSON_NODE_TYPE_NUMBER);
    TEST_CHECK(field_node != NULL);
    TEST_CHECK(tweak_json_get_type(field_node) == TWEAK_JSON_NODE_TYPE_NUMBER);
    bool coerce_success;
    int value = convert_simple_to_int32(field_node, &coerce_success);
    TEST_CHECK(coerce_success);
    TEST_CHECK(value == expected_value);
}

const char* JSON3 = ""
"{"
"    \"control\": \"slider\","
"    \"min\": 55.6,"
"    \"max\": 100.4,"
"    \"readonly\": false,"
"    \"decimals\": 3,"
"    \"step\": 0.1"
"}";

void test_json_3(void) {
    struct tweak_json_node* doc = tweak_json_parse(JSON3);
    TEST_CHECK(doc != NULL);
    TEST_CHECK(tweak_json_get_type(doc) == TWEAK_JSON_NODE_TYPE_OBJECT);
    TEST_CHECK(tweak_json_get_object_field(doc, "options", TWEAK_JSON_NODE_TYPE_ANY) == NULL);
    check_string_field(doc, "control", "slider");
    check_double_field(doc, "min", 55.6);
    check_double_field(doc, "max", 100.4);
    check_bool_field(doc, "readonly", false);
    check_int32_field(doc, "decimals", 3);
    check_double_field(doc, "step", 0.1);
    tweak_json_destroy(doc);
}

const char* JSON4 = ""
"{"
"    \"options\": [{\"value\": false, \"text\": \"On\"},"
"                  {\"value\": true, \"text\": \"Off\"}]"
"}";

static void test_json_4_node_X(const struct tweak_json_node* array, int index, bool value, const char* text);

void test_json_4(void) {
    struct tweak_json_node* doc = tweak_json_parse(JSON4);
    TEST_CHECK(doc != NULL);
    TEST_CHECK(tweak_json_get_type(doc) == TWEAK_JSON_NODE_TYPE_OBJECT);
    TEST_CHECK(tweak_json_get_object_field(doc, "opt1ons", TWEAK_JSON_NODE_TYPE_ANY) == NULL);
    const struct tweak_json_node* options_field =
      tweak_json_get_object_field(doc, "options", TWEAK_JSON_NODE_TYPE_COMPOUND);
    TEST_CHECK(options_field != NULL);
    TEST_CHECK(tweak_json_get_type(options_field) == TWEAK_JSON_NODE_TYPE_ARRAY);
    size_t size;
    tweak_json_get_array_size_status status = tweak_json_get_array_size(options_field, &size);
    TEST_CHECK(status == TWEAK_JSON_GET_SIZE_SUCCESS);
    TEST_CHECK(size == 2);
    test_json_4_node_X(options_field, 0, false, "On");
    test_json_4_node_X(options_field, 1, true, "Off");
    TEST_CHECK(tweak_json_get_array_item(options_field, 3, TWEAK_JSON_NODE_TYPE_ANY) == NULL);
    tweak_json_destroy(doc);
}

static void test_json_4_node_X(const struct tweak_json_node* array, int index, bool value, const char* text) {
    (void)text;
    const struct tweak_json_node* node;
    node = tweak_json_get_array_item(array, index, TWEAK_JSON_NODE_TYPE_NULL | TWEAK_JSON_NODE_TYPE_ARRAY
        | TWEAK_JSON_NODE_TYPE_STRING | TWEAK_JSON_NODE_TYPE_BOOL | TWEAK_JSON_NODE_TYPE_NUMBER);
    TEST_CHECK(node == NULL);
    node = tweak_json_get_array_item(array, index, TWEAK_JSON_NODE_TYPE_OBJECT);
    TEST_CHECK(node != NULL);
    TEST_CHECK(tweak_json_get_type(node) == TWEAK_JSON_NODE_TYPE_OBJECT);
    const struct tweak_json_node* value_field = tweak_json_get_object_field(node, "value", TWEAK_JSON_NODE_TYPE_BOOL);
    TEST_CHECK(tweak_json_get_type(value_field) == TWEAK_JSON_NODE_TYPE_BOOL);
    bool coerce_success;
    bool value0 = convert_simple_to_bool(value_field, &coerce_success);
    TEST_CHECK(coerce_success);
    TEST_CHECK(value == value0);
    const struct tweak_json_node* text_field = tweak_json_get_object_field(node, "text", TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(tweak_json_get_type(text_field) == TWEAK_JSON_NODE_TYPE_STRING);
    TEST_CHECK(strcmp(tweak_json_node_as_c_str(text_field), text) == 0);
}

const char* JSON5 = ""
"{"
"    \"control\": \"slider\","
"    \"min\": 55.6,"
"    \"max\": 100.4,"
"    \"readonly\": true,"
"    \"decimals\": 3,"
"    \"options\": null,"
"    \"step\": 0.1"
"}";

void test_json_5(void) {
    struct tweak_json_node* doc = tweak_json_parse(JSON5);
    TEST_CHECK(doc != NULL);
    TEST_CHECK(tweak_json_get_type(doc) == TWEAK_JSON_NODE_TYPE_OBJECT);
    check_string_field(doc, "control", "slider");
    check_double_field(doc, "min", 55.6);
    check_double_field(doc, "max", 100.4);
    check_bool_field(doc, "readonly", true);
    check_int32_field(doc, "decimals", 3);
    TEST_CHECK(tweak_json_get_object_field(doc, "options", TWEAK_JSON_NODE_TYPE_ARRAY) == NULL);
    TEST_CHECK(tweak_json_get_object_field(doc, "options", TWEAK_JSON_NODE_TYPE_ANY) != NULL);
    check_double_field(doc, "step", 0.1);
    tweak_json_destroy(doc);
}

void test_json_null_1(void) {
    struct tweak_json_node* doc = tweak_json_parse("null");
    TEST_CHECK(doc != NULL);
    TEST_CHECK(tweak_json_get_type(doc) == TWEAK_JSON_NODE_TYPE_NULL);
    tweak_json_destroy(doc);
}

TEST_LIST = {
   { "test_json_1", test_json_1 },
   { "test_json_1e_1", test_json_1e_1},
   { "test_json_1e_2", test_json_1e_2},
   { "test_json_1e_3", test_json_1e_3},
   { "test_json_1e_4", test_json_1e_4},
   { "test_json_1e_5", test_json_1e_5},
   { "test_json_11", test_json_11 },
   { "test_json_12", test_json_12 },
   { "test_json_2", test_json_2 },
   { "test_json_3", test_json_3 },
   { "test_json_4", test_json_4 },
   { "test_json_5", test_json_5 },
   { "test_json_null_1", test_json_null_1 },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};
