/**
 * @file tweakconfig.h
 * @ingroup tweak-api
 *
 * @brief configuration for compatibility layer.
 *
 * @note library must be recompiled if user decides to alter these settings.
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

#ifndef TWEAK_CONFIG_H_INCLUDED
#define TWEAK_CONFIG_H_INCLUDED

#define TWEAK_CONNECTION_TYPE "nng"

#define TWEAK_PARAMS "role=server"

#define TWEAK_URI "tcp://0.0.0.0:7777/"

#endif
