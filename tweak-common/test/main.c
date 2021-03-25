/**
 * @file main.c
 * @ingroup tweak-api
 * @brief test suite for common library.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/string.h>

#include <stdio.h>
#include <stdlib.h>

static const char *const large_sting_literal1 =
"/******************************************************************************* \n"
" * main.c                                                                        \n"
" *                                                                               \n"
" * Example test for Cogent Tweak Tool v2, transport layer (tweak-wire library).  \n"
" *                                                                               \n"
" * Copyright (c) 2020 Cogent Embedded Inc.                                       \n"
" * ALL RIGHTS RESERVED.                                                          \n"
" *                                                                               \n"
" * The source code contained or described herein and all documents related to    \n"
" *the source code(\"Software\") or their modified versions are owned by Cogent   \n"
" *Embedded Inc. or its affiliates.                                               \n"
" *                                                                               \n"
" * No part of the Software may be used, copied, reproduced, modified, published, \n"
" * uploaded, posted, transmitted, distributed, or disclosed in any way without   \n"
" * prior express written permission from Cogent Embedded Inc.                    \n"
" *                                                                               \n"
" * Cogent Embedded Inc. grants a nonexclusive, non-transferable, royalty-free    \n"
" * license to use the Software to Licensee without the right to sublicense.      \n"
" * Licensee agrees not to distribute the Software to any third-party without     \n"
" * the prior written permission of Cogent Embedded Inc.                          \n"
" *                                                                               \n"
" * Unless otherwise agreed by Cogent Embedded Inc. in writing, you may not       \n"
" *remove or alter this notice or any other notice embedded in Software in any    \n"
" *way.                                                                           \n"
" *                                                                               \n"
" * THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  \n"
" * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      \n"
" * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE  \n"
" * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        \n"
" * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, \n"
" * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     \n"
" * THE SOFTWARE.                                                                 \n"
" *******************************************************************************/";

static const char *const large_sting_literal2 =
"/*******1******2******3********************************************************* \n"
" * main.1      2      3                                                          \n"
" *      1      2      3                                                          \n"
" * Examp1e test2for Co3ent Tweak Tool v2, transport layer (tweak-wire library).  \n"
" *      1      2      3                                                          \n"
" * Copyr1ght (c2 2020 3ogent Embedded Inc.                                       \n"
" * ALL R1GHTS R2SERVED3                                                          \n"
" *      1      2      3                                                          \n"
" * The s1urce c2de con3ained or described herein and all documents related to    \n"
" *the so1rce co2e(\"So3tware\") or their modified versions are owned by Cogent   \n"
" *Embedd1d Inc.2or its3affiliates.                                               \n"
" *      1      2      3                                                          \n"
" * No pa1t of t2e Soft3are may be used, copied, reproduced, modified, published, \n"
" * uploa1ed, po2ted, t3ansmitted, distributed, or disclosed in any way without   \n"
" * prior1expres2 writt3n permission from Cogent Embedded Inc.                    \n"
" *      1      2      3                                                          \n"
" * Cogen1 Embed2ed Inc3 grants a nonexclusive, non-transferable, royalty-free    \n"
" * licen1e to u2e the 3oftware to Licensee without the right to sublicense.      \n"
" * Licen1ee agr2es not3to distribute the Software to any third-party without     \n"
" * the p1ior wr2tten p3rmission of Cogent Embedded Inc.                          \n"
" *      1      2      3                                                          \n"
" * Unles1 other2ise ag3eed by Cogent Embedded Inc. in writing, you may not       \n"
" *remove1or alt2r this3notice or any other notice embedded in Software in any    \n"
" *way.  1      2      3                                                          \n"
" *      1      2      3                                                          \n"
"/*******1******2******3********************************************************* \n"
" * main.1      2      3                                                          \n"
" *      1      2      3                                                          \n"
" * Examp1e test2for Co3ent Tweak Tool v2, transport layer (tweak-wire library).  \n"
" *      1      2      3                                                          \n"
" * Copyr1ght (c2 2020 3ogent Embedded Inc.                                       \n"
" * ALL R1GHTS R2SERVED3                                                          \n"
" *      1      2      3                                                          \n"
" * The s1urce c2de con3ained or described herein and all documents related to    \n"
" *the so1rce co2e(\"So3tware\") or their modified versions are owned by Cogent   \n"
" *Embedd1d Inc.2or its3affiliates.                                               \n"
" *      1      2      3                                                          \n"
" * No pa1t of t2e Soft3are may be used, copied, reproduced, modified, published, \n"
" * uploa1ed, po2ted, t3ansmitted, distributed, or disclosed in any way without   \n"
" * prior1expres2 writt3n permission from Cogent Embedded Inc.                    \n"
" *      1      2      3                                                          \n"
" * Cogen1 Embed2ed Inc3 grants a nonexclusive, non-transferable, royalty-free    \n"
" * licen1e to u2e the 3oftware to Licensee without the right to sublicense.      \n"
" * Licen1ee agr2es not3to distribute the Software to any third-party without     \n"
" * the p1ior wr2tten p3rmission of Cogent Embedded Inc.                          \n"
" *      1      2      3                                                          \n"
" * Unles1 other2ise ag3eed by Cogent Embedded Inc. in writing, you may not       \n"
" *remove1or alt2r this3notice or any other notice embedded in Software in any    \n"
" *way.  1      2      3                                                          \n"
" *      1      2      3                                                          \n"
"/*******1******2******3********************************************************* \n"
" * main.1      2      3                                                          \n"
" *      1      2      3                                                          \n"
" * Examp1e test2for Co3ent Tweak Tool v2, transport layer (tweak-wire library).  \n"
" *      1      2      3                                                          \n"
" * Copyr1ght (c2 2020 3ogent Embedded Inc.                                       \n"
" * ALL R1GHTS R2SERVED3                                                          \n"
" *      1      2      3                                                          \n"
" * The s1urce c2de con3ained or described herein and all documents related to    \n"
" *the so1rce co2e(\"So3tware\") or their modified versions are owned by Cogent   \n"
" *Embedd1d Inc.2or its3affiliates.                                               \n"
" *      1      2      3                                                          \n"
" * No pa1t of t2e Soft3are may be used, copied, reproduced, modified, published, \n"
" * uploa1ed, po2ted, t3ansmitted, distributed, or disclosed in any way without   \n"
" * prior1expres2 writt3n permission from Cogent Embedded Inc.                    \n"
" *      1      2      3                                                          \n"
" * Cogen1 Embed2ed Inc3 grants a nonexclusive, non-transferable, royalty-free    \n"
" * licen1e to u2e the 3oftware to Licensee without the right to sublicense.      \n"
" * Licen1ee agr2es not3to distribute the Software to any third-party without     \n"
" * the p1ior wr2tten p3rmission of Cogent Embedded Inc.                          \n"
" *      1      2      3                                                          \n"
" * Unles1 other2ise ag3eed by Cogent Embedded Inc. in writing, you may not       \n"
" *remove1or alt2r this3notice or any other notice embedded in Software in any    \n"
" *way.  1      2      3                                                          \n"
" *      1      2      3                                                          \n"
"/*******1******2******3********************************************************* \n"
" * main.1      2      3                                                          \n"
" *      1      2      3                                                          \n"
" * Examp1e test2for Co3ent Tweak Tool v2, transport layer (tweak-wire library).  \n"
" *      1      2      3                                                          \n"
" * Copyr1ght (c2 2020 3ogent Embedded Inc.                                       \n"
" * ALL R1GHTS R2SERVED3                                                          \n"
" *      1      2      3                                                          \n"
" * The s1urce c2de con3ained or described herein and all documents related to    \n"
" *the so1rce co2e(\"So3tware\") or their modified versions are owned by Cogent   \n"
" *Embedd1d Inc.2or its3affiliates.                                               \n"
" *      1      2      3                                                          \n"
" * No pa1t of t2e Soft3are may be used, copied, reproduced, modified, published, \n"
" * uploa1ed, po2ted, t3ansmitted, distributed, or disclosed in any way without   \n"
" * prior1expres2 writt3n permission from Cogent Embedded Inc.                    \n"
" *      1      2      3                                                          \n"
" * Cogen1 Embed2ed Inc3 grants a nonexclusive, non-transferable, royalty-free    \n"
" * licen1e to u2e the 3oftware to Licensee without the right to sublicense.      \n"
" * Licen1ee agr2es not3to distribute the Software to any third-party without     \n"
" * the p1ior wr2tten p3rmission of Cogent Embedded Inc.                          \n"
" *      1      2      3                                                          \n"
" * Unles1 other2ise ag3eed by Cogent Embedded Inc. in writing, you may not       \n"
" *remove1or alt2r this3notice or any other notice embedded in Software in any    \n"
" *way.  1      2      3                                                          \n"
" *      1      2      3                                                          \n"
" *******1******2******3*********************************************************/";

static size_t recv_n_bytes_from_io(char* buff, size_t length) {
  strncpy(buff, large_sting_literal1, length);
  return length;
}

int main(int argc, const char **argv) {
  tweak_variant_string test1 = { 0 };

  puts("tweak_variant_string_c_str(NULL) == NULL");
  assert(tweak_variant_string_c_str(NULL) == NULL);

  puts("Run tests for uninitialized string ...");
  assert(tweak_variant_string_is_empty(&test1));
  assert(tweak_variant_is_small_string(&test1));
  assert(tweak_variant_string_c_str(&test1) != NULL);

  puts("Run tests for zero length string ...");
  tweak_variant_assign_string(&test1, "");
  assert(tweak_variant_string_is_empty(&test1));
  assert(tweak_variant_is_small_string(&test1));
  assert(tweak_variant_string_c_str(&test1) != NULL);
  assert(strcmp(tweak_variant_string_c_str(&test1), "") == 0);
  tweak_variant_destroy_string(&test1);
  assert(tweak_variant_string_is_empty(&test1));
  assert(tweak_variant_is_small_string(&test1));

  puts("Run tests for small initialized string ...");
  tweak_variant_assign_string(&test1, "ABCD");
  assert(!tweak_variant_string_is_empty(&test1));
  assert(tweak_variant_is_small_string(&test1));
  assert(tweak_variant_string_c_str(&test1) != NULL);
  assert(strcmp(tweak_variant_string_c_str(&test1), "ABCD") == 0);
  tweak_variant_destroy_string(&test1);
  assert(tweak_variant_string_is_empty(&test1));
  assert(tweak_variant_is_small_string(&test1));

  puts("Run tests for large initialized string ...");
  tweak_variant_assign_string(&test1, large_sting_literal1);
  assert(!tweak_variant_string_is_empty(&test1));
  assert(!tweak_variant_is_small_string(&test1));
  assert(tweak_variant_string_c_str(&test1) != NULL);
  assert(strcmp(tweak_variant_string_c_str(&test1), large_sting_literal1) == 0);
  char * old_ptr = test1.large_buffer;
  size_t old_capacity = test1.capacity;
  tweak_variant_assign_string(&test1, large_sting_literal2);
  assert(!tweak_variant_string_is_empty(&test1));
  assert(!tweak_variant_is_small_string(&test1));
  assert(tweak_variant_string_c_str(&test1) != NULL);
  assert(strcmp(tweak_variant_string_c_str(&test1), large_sting_literal2) == 0);

  // large_sting_literal2 is noticeably larger.
  // Expect reallocation.
  assert(old_capacity != test1.capacity);

  old_ptr = test1.large_buffer;
  old_capacity = test1.capacity;
  tweak_variant_assign_string(&test1, large_sting_literal1);
  assert(!tweak_variant_string_is_empty(&test1));
  assert(!tweak_variant_is_small_string(&test1));
  assert(tweak_variant_string_c_str(&test1) != NULL);
  assert(strcmp(tweak_variant_string_c_str(&test1), large_sting_literal1) == 0);

  // large_sting_literal1 has lesser size.
  // Reallocation shouldn't happen here.
  assert(old_ptr == test1.large_buffer);
  assert(old_capacity == test1.capacity);

  tweak_variant_destroy_string(&test1);
  assert(tweak_variant_is_small_string(&test1));

  puts("Run tests for tweak_variant_string_swap ...");
  tweak_variant_assign_string(&test1, large_sting_literal1);
  {
    tweak_variant_string test2 = { 0 };
    tweak_variant_swap_string(&test1, &test2);
    assert(!tweak_variant_is_small_string(&test2));
    assert(tweak_variant_string_c_str(&test2) != NULL);
    assert(strcmp(tweak_variant_string_c_str(&test2), large_sting_literal1) == 0);
    tweak_variant_destroy_string(&test2);
  }
  assert(tweak_variant_is_small_string(&test1));
  tweak_variant_destroy_string(&test1); /* Does nothing,
                                           But if client of the code modelled
                                           by the nested block above
                                           didn't claim ownership of the string,
                                           real deallocation would happen here.
                                        */

  return 0;
}
