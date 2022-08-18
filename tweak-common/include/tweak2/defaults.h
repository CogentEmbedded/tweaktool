/**
 * @file defaults.h
 * @ingroup tweak-api
 *
 * @brief Project wide constants.
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
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_DEFAULTS_H_INCLUDED
#define TWEAK_DEFAULTS_H_INCLUDED

#if defined(_MSC_BUILD) || defined(__APPLE__)
#define TWEAK_DEFAULT_ENDPOINT_TEMPLATE "tcp://127.0.0.1:%d/"
#define TWEAK_DEFAULT_ENDPOINT "tcp://127.0.0.1:7777/"
#else
#define TWEAK_DEFAULT_ENDPOINT_TEMPLATE "tcp://0.0.0.0:%d/"
#define TWEAK_DEFAULT_ENDPOINT "tcp://0.0.0.0:7777/"
#endif


#endif
