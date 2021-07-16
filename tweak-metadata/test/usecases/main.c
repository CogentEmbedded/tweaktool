/**
 * @file main.c
 * @ingroup tweak-api
 * @brief test suite for tweak metadata library.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/metadata.h>

#include <acutest.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>

void test_1() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, NULL);
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

void test_1e_1() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "");
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

void test_1e_2() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "\"BoZo");
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

void test_2() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "{ \"readonly\": true }");
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

void test_2_btn() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "{ \"control\": \"button\" }");
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

void test_2e_1() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "{ \"BoZo\": true, \"readonly\": true }");
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

void test_2e_2() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "{ \"BoZo\": true, \"readonly\": true }");
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

void test_3() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "{ \"options\": [\"True\", \"False\"] }");
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

void test_3e_1() {
  /* Missing closing curly bracket for doc object */
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_BOOL, "{ \"options\": [\"True\", \"False\"]");
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

void test_5() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_FLOAT, NULL);
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
  float err = fabs(step0 - .0001f); /* derived from decimals == 4*/
  TEST_CHECK(err < powf(10, -(decimals + 2)));
  tweak_metadata_destroy(metadata);
}

void test_6() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_FLOAT, "{ \"min\": -1, \"max\": 1, \"decimals\": 6 }");
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
  float err = fabs(step0 - .0001f); /* derived from decimals == 4*/
  TEST_CHECK(err < powf(10, -(decimals + 2)));
  tweak_metadata_destroy(metadata);
}

void test_7() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_UINT32, "{ \"min\": 0, \"max\": 512 }");
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

void test_8() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, "{ \"min\": -128, \"max\": 127 }");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_SPINBOX);
  const tweak_variant* min = tweak_metadata_get_min(metadata);
  TEST_CHECK(min->type == TWEAK_VARIANT_TYPE_SINT32);
  TEST_CHECK(min->value.sint32 == -128);
  const tweak_variant* max = tweak_metadata_get_max(metadata);
  TEST_CHECK(max->type == TWEAK_VARIANT_TYPE_SINT32);
  TEST_CHECK(max->value.sint32 == 127);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_destroy(metadata);
}

void test_9() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, NULL);
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

void test_10() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, "{\"options\": "
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

void test_11() {
  tweak_metadata metadata = tweak_metadata_create(TWEAK_VARIANT_TYPE_SINT32, "{\"options\": [\"Single element\"]}");
  tweak_metadata_control_type control = tweak_metadata_get_control_type(metadata);
  TEST_CHECK(control == TWEAK_METADATA_CONTROL_COMBOBOX);
  TEST_CHECK(tweak_metadata_get_readonly(metadata) == false);
  tweak_metadata_options options = tweak_metadata_get_options(metadata);
  TEST_CHECK(options != NULL);
  TEST_CHECK(tweak_metadata_get_enum_size(options) == 1);
  check_nth_option(options, 0, 0, "Single element");
  tweak_metadata_destroy(metadata);
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
