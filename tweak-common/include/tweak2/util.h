/**
 * @file util.h
 * @ingroup tweak-api
 *
 * @brief Auxiliary utility functions to handle primitives from commons library.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_UTIL_H_INCLUDED
#define TWEAK_UTIL_H_INCLUDED

#include <tweak2/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generate new unique tweak identifier.
 *
 * @return New tweak identifier.
 */
tweak_id tweak_common_genid();

#ifdef __cplusplus
}
#endif

#endif
