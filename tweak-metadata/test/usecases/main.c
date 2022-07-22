/**
 * @file main.c
 * @ingroup tweak-api
 * @brief test suite for tweak metadata library.
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

#include <tweak2/metadata.h>

#include <acutest.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>

void test_1(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, NULL);
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_CHECKBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_destroy(metadata);
}

void test_1e_1(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_CHECKBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_destroy(metadata);
}

void test_1e_2(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "\"BoZo");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_CHECKBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_destroy(metadata);
}

void test_2(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "{ \"readonly\": true }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_CHECKBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == true);
  tweak_metadata_destroy(metadata);
}

void test_2_btn(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "{ \"control\": \"button\" }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_BUTTON);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  tweak_metadata_destroy(metadata);
}

void test_2e_1(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "{ \"BoZo\": true, \"readonly\": true }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_CHECKBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == true);
  tweak_metadata_destroy(metadata);
}

void test_2e_2(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "{ \"BoZo\": true, \"readonly\": true }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_CHECKBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == true);
  tweak_metadata_destroy(metadata);
}

bool compare_to_int(const tweak_variant* a, int b);

void check_nth_option(tweak_metadata_options options, size_t index, int value, const char* text) {
  const tweak_variant_string* text0 = tweak_metadata_get_enum_text(options, index);
  TEST_CHECK(strcmp(tweak_variant_string_c_str(text0), text) == 0);
  const tweak_variant* value0 = tweak_metadata_get_enum_value(options, index);
  TEST_CHECK(compare_to_int(value0, value));
}

void test_3(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "{ \"options\": [\"True\", \"False\"] }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_COMBOBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_NULL);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_NULL);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_options options = tweak_metadata_get_options(metadata);
  TEST_CHECK(options != NULL);
  TEST_CHECK(tweak_metadata_get_enum_size(options) == 2);
  check_nth_option(options, 0, 0, "True");
  check_nth_option(options, 1, 1, "False");
  tweak_metadata_destroy(metadata);
}

void test_3e_1(void) {
  /* Missing closing curly bracket for doc object */
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, 1, "{ \"options\": [\"True\", \"False\"]");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_CHECKBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(min->value.b == false);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_BOOL);
  TEST_CHECK(max->value.b == true);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_destroy(metadata);
}

void test_5(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_FLOAT, 1, NULL);
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_SLIDER);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_FLOAT);
  TEST_CHECK(min->value.fp32 == -FLT_MAX);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_FLOAT);
  TEST_CHECK(max->value.fp32 == FLT_MAX);
  uint32_t decimals = tweak_metadata_get_decimals(metadata);
  TEST_CHECK(decimals == 4);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  const tweak_variant* step = tweak_metadata_get_step(metadata);
  TEST_CHECK(step->type == TWEAK_VARIANT_TYPE_FLOAT);
  float step0 = step->value.fp32;
  float err = fabsf(step0 - .0001f); /* derived from decimals == 4*/
  TEST_CHECK(err < powf(10, -((float)decimals + 2.f)));
  tweak_metadata_destroy(metadata);
}

void test_6(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_FLOAT, 1, "{ \"min\": -1, \"max\": 1, \"decimals\": 6 }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_SLIDER);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_FLOAT);
  TEST_CHECK(min->value.fp32 == -1.f);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_FLOAT);
  TEST_CHECK(max->value.fp32 == 1.f);
  uint32_t decimals = tweak_metadata_get_decimals(metadata);
  TEST_CHECK(decimals == 6);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  const tweak_variant* step = tweak_metadata_get_step(metadata);
  TEST_CHECK(step->type == TWEAK_VARIANT_TYPE_FLOAT);
  float step0 = step->value.fp32;
  float err = fabsf(step0 - .0001f); /* derived from decimals == 4*/
  TEST_CHECK(err < powf(10, -((float)decimals + 2.0f)));
  tweak_metadata_destroy(metadata);
}

void test_7(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_UINT32, 1, "{ \"min\": 0, \"max\": 512 }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_SPINBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_UINT32);
  TEST_CHECK(min->value.uint32 == 0);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_UINT32);
  TEST_CHECK(max->value.uint32 == 512L);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_destroy(metadata);
}

void test_8(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, 1, "{ \"min\": -128, \"max\": 127, \"unit\": \"counter\" }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_SPINBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_SINT32);
  TEST_CHECK(min->value.sint32 == -128);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_SINT32);
  TEST_CHECK(max->value.sint32 == 127);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  TEST_CHECK(strcmp(tweak_variant_string_c_str(tweak_metadata_get_unit(metadata)), "counter") == 0);
  tweak_metadata_destroy(metadata);
}

void test_9(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, 1, NULL);
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_SPINBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_SINT32);
  TEST_CHECK(min->value.sint32 == INT_MIN);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_SINT32);
  TEST_CHECK(max->value.sint32 == INT_MAX);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_destroy(metadata);
}

void test_10(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, 1, "{\"options\": "
      "[{\"value\": 100, \"text\": \"Foo\"},"
      " {\"value\": 110, \"text\": \"Bar\"},"
      " {\"value\": 120, \"text\": \"Baz\"},"
      " \"Buff\"]}");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_COMBOBOX);

  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_options options = tweak_metadata_get_options(metadata);
  TEST_CHECK(options != NULL);
  TEST_CHECK(tweak_metadata_get_enum_size(options) == 4);
  check_nth_option(options, 0, 100, "Foo");
  check_nth_option(options, 1, 110, "Bar");
  check_nth_option(options, 2, 120, "Baz");
  check_nth_option(options, 3, 121, "Buff");
  tweak_metadata_destroy(metadata);
}

void test_11(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, 1, "{\"options\": [\"Single element\"]}");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_COMBOBOX);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_options options = tweak_metadata_get_options(metadata);
  TEST_CHECK(options != NULL);
  TEST_CHECK(tweak_metadata_get_enum_size(options) == 1);
  check_nth_option(options, 0, 0, "Single element");
  tweak_metadata_destroy(metadata);
}

void test_12(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_VECTOR_FLOAT, 10, "{}");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_TABLE);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_layout layout = tweak_metadata_get_layout(metadata);
  TEST_CHECK(layout != NULL);
  TEST_CHECK(tweak_metadata_layout_get_number_of_dimensions(layout) == 1);
  TEST_CHECK(tweak_metadata_layout_get_dimension(layout, 0) == 10);
  tweak_metadata_destroy(metadata);
}

void test_13(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_VECTOR_FLOAT, 10, "{\"layout\":{\"dimensions\":[5, 2]}}");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_TABLE);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_layout layout = tweak_metadata_get_layout(metadata);
  TEST_CHECK(layout != NULL);
  TEST_CHECK(tweak_metadata_layout_get_number_of_dimensions(layout) == 2);
  TEST_CHECK(tweak_metadata_layout_get_dimension(layout, 0) == 5);
  TEST_CHECK(tweak_metadata_layout_get_dimension(layout, 1) == 2);
  tweak_metadata_destroy(metadata);
}

void test_14(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_VECTOR_FLOAT, 10,
    "{\"layout\":{\"order\": \"column-major\", \"dimensions\": [5, 2]}}");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_TABLE);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_layout layout = tweak_metadata_get_layout(metadata);
  TEST_CHECK(layout != NULL);
  TEST_CHECK(tweak_metadata_layout_get_order(layout) == TWEAK_METADATA_LAYOUT_ORDER_COLUMN_MAJOR);
  TEST_CHECK(tweak_metadata_layout_get_number_of_dimensions(layout) == 2);
  TEST_CHECK(tweak_metadata_layout_get_dimension(layout, 0) == 5);
  TEST_CHECK(tweak_metadata_layout_get_dimension(layout, 1) == 2);
  tweak_metadata_destroy(metadata);
}

void test_15(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_VECTOR_FLOAT, 10,
    "{\"layout\":{\"order\": \"column-major\", \"dimensions\": [5, 2]}}");

  tweak_metadata metadata_copy = tweak_metadata_copy(metadata);
  tweak_metadata_destroy(metadata);

  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata_copy);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_TABLE);
  TEST_CHECK(tweak_metadata_get_readonly(metadata_copy) == false);
  tweak_metadata_layout layout = tweak_metadata_get_layout(metadata_copy);
  TEST_CHECK(layout != NULL);
  TEST_CHECK(tweak_metadata_layout_get_order(layout) == TWEAK_METADATA_LAYOUT_ORDER_COLUMN_MAJOR);
  TEST_CHECK(tweak_metadata_layout_get_number_of_dimensions(layout) == 2);
  TEST_CHECK(tweak_metadata_layout_get_dimension(layout, 0) == 5);
  TEST_CHECK(tweak_metadata_layout_get_dimension(layout, 1) == 2);

  tweak_metadata_destroy(metadata_copy);
}

void test_16(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, 1, "{\"options\": "
      "[{\"value\": 100, \"text\": \"Foo\"},"
      " {\"value\": 110, \"text\": \"Bar\"},"
      " {\"value\": 120, \"text\": \"Baz\"},"
      " \"Buff\"]}");
  tweak_metadata metadata_copy = tweak_metadata_copy(metadata);
  tweak_metadata_destroy(metadata);

  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata_copy);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_COMBOBOX);

  TEST_CHECK(tweak_metadata_get_readonly(metadata_copy) == false);
  tweak_metadata_options options = tweak_metadata_get_options(metadata_copy);
  TEST_CHECK(options != NULL);
  TEST_CHECK(tweak_metadata_get_enum_size(options) == 4);
  check_nth_option(options, 0, 100, "Foo");
  check_nth_option(options, 1, 110, "Bar");
  check_nth_option(options, 2, 120, "Baz");
  check_nth_option(options, 3, 121, "Buff");
  tweak_metadata_destroy(metadata_copy);
}

void test_17(void) {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_UINT8, 1,
  "{\"max\":7,\"options\":["
    "{\"value\": 0, \"text\": \"Error\"},"
    "{\"value\": 1, \"text\": \"Warning\"},"
    "{\"value\": 2, \"text\": \"Info\"},"
    "{\"value\": 3, \"text\": \"Performance\"},"
    "{\"value\": 4, \"text\": \"Event\"},"
    "{\"value\": 5, \"text\": \"Debug\"},"
    "{\"value\": 6, \"text\": \"Trace\"},"
    "{\"value\": 7, \"text\": \"Dump\"}"
  "]}");
  tweak_metadata metadata_copy = tweak_metadata_copy(metadata);
  tweak_metadata_destroy(metadata);

  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata_copy);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_COMBOBOX); /* Options are present */

  TEST_CHECK(tweak_metadata_get_readonly(metadata_copy) == false);
  const tweak_variant *max = tweak_metadata_get_max(metadata_copy);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_NULL);
  tweak_metadata_options options = tweak_metadata_get_options(metadata_copy);
  TEST_CHECK(options != NULL);
  TEST_CHECK(tweak_metadata_get_enum_size(options) == 8);
  check_nth_option(options, 0, 0, "Error");
  check_nth_option(options, 1, 1, "Warning");
  check_nth_option(options, 2, 2, "Info");
  check_nth_option(options, 3, 3, "Performance");
  check_nth_option(options, 4, 4, "Event");
  check_nth_option(options, 5, 5, "Debug");
  check_nth_option(options, 6, 6, "Trace");
  check_nth_option(options, 7, 7, "Dump");
  tweak_metadata_destroy(metadata_copy);
}

TEST_LIST = {
  { "test_1", test_1 },
  { "test_1e_1", test_1e_1 },
  { "test_1e_2", test_1e_2 },
  { "test_2", test_2 },
  { "test_2_btn", test_2_btn },
  { "test_2e_1", test_2e_1 },
  { "test_2e_2", test_2e_2 },
  { "test_3", test_3 },
  { "test_3e_1", test_3e_1 },
  { "test_5", test_5 },
  { "test_6", test_6 },
  { "test_7", test_7 },
  { "test_8", test_8 },
  { "test_9", test_9 },
  { "test_10", test_10 },
  { "test_11", test_11 },
  { "test_12", test_12 },
  { "test_13", test_13 },
  { "test_14", test_14 },
  { "test_15", test_15 },
  { "test_16", test_16 },
  { "test_17", test_17 },
  { NULL, NULL }     /* zeroed record marking the end of the list */
};


bool compare_to_int(const tweak_variant* a, int b) {
    switch (a->type) {
    case TWEAK_VARIANT_TYPE_BOOL:
        return a->value.b == b;
    case TWEAK_VARIANT_TYPE_SINT8:
        return a->value.sint8 == b;
    case TWEAK_VARIANT_TYPE_SINT16:
        return a->value.sint16 == b;
    case TWEAK_VARIANT_TYPE_SINT32:
        return a->value.sint32 == b;
    case TWEAK_VARIANT_TYPE_SINT64:
        return a->value.sint64 == b;
    case TWEAK_VARIANT_TYPE_UINT8:
        return a->value.uint8 == b;
    case TWEAK_VARIANT_TYPE_UINT16:
        return a->value.uint16 == b;
    case TWEAK_VARIANT_TYPE_UINT32:
        return (int)a->value.uint32 == b;
    case TWEAK_VARIANT_TYPE_UINT64:
        return (int)a->value.uint64 == b;
    case TWEAK_VARIANT_TYPE_FLOAT:
        return (int)a->value.fp32 == b;
    case TWEAK_VARIANT_TYPE_DOUBLE:
        return (int)a->value.fp64 == b;
    default:
        break;
    }
    return false;
}
