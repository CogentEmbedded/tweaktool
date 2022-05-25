/**
 * @file test-features.c
 * @ingroup tweak-app-implementation-test
 *
 * @brief part of test suite to test tweak2 application implementation.
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

/**
 * @defgroup tweak-app-implementation-test Test implementation for tweak-app internal interfaces.
 */

#include <tweak2/string.h>
#include <tweak2/variant.h>

#include "tweakappfeatures.h"

#include <acutest.h>

#define VECTORS_SUPPORTED_JSON "{\"vectors\": true}"
#define VECTORS_NOT_SUPPORTED_JSON "{\"vectors\": false}"

void test_features(void) {
    tweak_variant_string vectors_supported_vs = TWEAK_VARIANT_STRING_EMPTY;
    tweak_assign_string(&vectors_supported_vs, VECTORS_SUPPORTED_JSON);
    tweak_variant_string vectors_not_supported_vs = TWEAK_VARIANT_STRING_EMPTY;
    tweak_assign_string(&vectors_not_supported_vs, VECTORS_NOT_SUPPORTED_JSON);

    struct tweak_app_features vectors_supported = { 0 };
    struct tweak_app_features vectors_not_supported = { 0 };

    TEST_CHECK(tweak_app_features_from_json(&vectors_supported_vs, &vectors_supported));
    TEST_CHECK(tweak_app_features_from_json(&vectors_not_supported_vs, &vectors_not_supported));

    tweak_variant_string serialized_f = tweak_app_features_to_json(&vectors_not_supported);
    tweak_variant_string serialized_t = tweak_app_features_to_json(&vectors_supported);

    struct tweak_app_features deserialized_f = { 0 };
    struct tweak_app_features deserialized_t = { 0 };

    TEST_CHECK(tweak_app_features_from_json(&serialized_f, &deserialized_f));
    TEST_CHECK(tweak_app_features_from_json(&serialized_t, &deserialized_t));

    TEST_CHECK(!deserialized_f.vectors);
    TEST_CHECK(deserialized_t.vectors);
}

TEST_LIST = {
   { "test_features", test_features },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};

