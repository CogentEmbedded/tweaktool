/**
 * @file tweak_id_gen_atomic_add_value.c
 * @ingroup tweak-api
 *
 * @brief Utility method to generate tweak ids in multi thread env.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/types.h>
#include <atomic.h>

static volatile unsigned seed = 1UL;

tweak_id tweak_common_genid() {
    return atomic_add_value(&seed, 1);
}
