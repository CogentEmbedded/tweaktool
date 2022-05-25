/**
 * @file tweakappfeatures.h
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

#ifndef TWEAK_APP_FEATURES_H_INCLUDED
#define TWEAK_APP_FEATURES_H_INCLUDED

#include <tweak2/string.h>
#include <tweak2/types.h>

/**
 * @brief Structure encapsulating protocol capabilities of a peer.
 */
struct tweak_app_features {
  /**
   * @brief whether vector types allowed by a peer.
   * When false, this peer shall perform graceful degradation exposing of scalar items only.
   * No presence of tweaks having vector types shall be announced to connected peer.
   * Updates to these tweaks shall not be marshalled to connected peer as well.
   */
  bool vectors;
};

/**
 * @brief initialize minimal set of features supported by all builds.
 *
 * @param out_result features instance.
 */
void tweak_app_features_init_minimal(struct tweak_app_features* features);

/**
 * @brief initialize features for this build.
 *
 * @param out_result features instance.
 */
void tweak_app_features_init_default(struct tweak_app_features* features);

/**
 * @brief Parse features from JSON.
 *
 * @param json features serialized to a json string.
 * @param out_result output result parameter.
 *
 * @return true @p arg has been parsed successfully. false on json parse errors.
 */
bool tweak_app_features_from_json(const tweak_variant_string* json, struct tweak_app_features* out_result);

/**
 * @brief Serialize features to JSON string.
 *
 * @param arg features instance.
 *
 * @return string to send over wire.
 */
tweak_variant_string tweak_app_features_to_json(const struct tweak_app_features* arg);

/**
 * @brief Check if given @p type is supported by features.
 *
 * @param arg features instance.
 * @param type type to check.

 * @return true if type supported by remote peer exposing given features.
 */
bool tweak_app_features_check_type_compatibility(const struct tweak_app_features* arg, tweak_variant_type type);

#endif
