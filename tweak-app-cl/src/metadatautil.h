/**
 * @file metadatautil.h
 * @ingroup tweak-api
 *
 * @brief format and parse arrays with respect to metadata.
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

#ifndef TWEAK_APP_CL_METADATA_UTIL_INCLUDED
#define TWEAK_APP_CL_METADATA_UTIL_INCLUDED

#include <tweak2/variant.h>
#include <tweak2/string.h>

tweak_variant_string tweak_app_cl_metadata_aware_variant_to_string(const tweak_variant* value,
  const tweak_variant_string* metadata_str);

tweak_variant_type_conversion_result tweak_app_cl_metadata_aware_variant_from_string(const char* string,
  tweak_variant_type type, size_t item_count, const tweak_variant_string* metadata_str, tweak_variant* out);

#endif
