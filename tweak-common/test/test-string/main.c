/**
 * @file main.c
 * @ingroup tweak-api
 * @brief test suite for common library.
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

#include <tweak2/string.h>
#include <tweak2/variant.h>

#include <acutest.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const large_sting_literal1 =
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque sed turpis nec     \n"
"dolor accumsan aliquet sed nec massa. Cras ultricies pellentesque urna, at pulvinar      \n"
"metus posuere ac. Etiam felis nulla, lacinia quis dapibus a, ultrices ac purus.          \n"
"Donec at purus nec sem suscipit fermentum. Donec finibus ex eget dolor ornare,           \n"
"sed fringilla enim imperdiet. Sed tincidunt mollis eros. Proin dapibus magna eu          \n"
"mauris gravida, a molestie libero consequat. Nunc eleifend imperdiet interdum.           \n"
"Sed imperdiet eu enim eget vehicula. Pellentesque habitant morbi tristique               \n"
"senectus et netus et malesuada fames ac turpis egestas. Ut mattis nunc nec libero        \n"
"efficitur lacinia. Praesent eleifend quam augue, eu tempus lectus hendrerit eu.          \n"
"Nunc imperdiet mollis laoreet. Nunc quis nisl turpis.                                    \n"
"                                                                                         \n"
"Praesent turpis mauris, aliquet eu porta non, malesuada ullamcorper lorem.               \n"
"Donec feugiat, nibh sit amet laoreet sollicitudin, nisl ligula porttitor sapien,         \n"
"id fringilla lorem ex vitae leo. Mauris eget vehicula quam. Suspendisse interdum         \n"
"vehicula pharetra. Nunc porta ante id libero volutpat, id fringilla diam porta.          \n"
"Sed tristique congue elit et vulputate. Donec pretium lacus eu ex convallis porttitor.   \n"
"Nam volutpat sapien ante, quis varius ex egestas in. Maecenas ut ultrices libero.        \n"
"Etiam sagittis diam non tempor eleifend. Cras commodo leo nisl.                          \n"
"                                                                                         \n"
"Suspendisse elementum pulvinar quam, nec aliquet nulla rhoncus vel. Nullam consequat     \n"
"risus at mauris elementum vehicula. Curabitur placerat enim massa. Curabitur eget        \n"
"est congue, elementum mi sed, suscipit justo. Aenean consectetur aliquam sollicitudin.   \n"
"Cras vel porttitor velit. Donec commodo mattis faucibus. Duis eleifend id velit          \n"
"vitae ornare. Etiam et fermentum erat.                                                   \n"
"                                                                                         \n"
"Curabitur id molestie enim. In elementum massa eu libero fermentum, non cursus           \n"
"diam fringilla. Phasellus quis ante nec nunc tempus gravida. Aenean ultrices,            \n"
"erat eget porta elementum, justo tortor aliquam mauris, in hendrerit ex sapien           \n"
"nec mauris. Sed a massa volutpat, dignissim justo eget, rutrum lectus.                   \n"
"Fusce congue, quam eu convallis placerat, orci sem euismod ex, nec efficitur             \n"
"magna sapien ut diam. Phasellus interdum lorem eget sem mollis, in venenatis             \n"
"elit gravida. Aenean et ultrices odio, at tempus lectus. Mauris ex nisl, laoreet         \n"
"in lacus eget, convallis ultricies nisi. Aliquam dui arcu, cursus vitae bibendum et,     \n"
"dapibus sed dui. Donec tellus quam, mollis vitae porta a, posuere eget nunc.             \n"
"Ut a eros in nunc elementum dapibus. Integer turpis purus, lacinia eget porttitor        \n"
"tristique, accumsan et magna.                                                            \n"
"                                                                                         \n"
"Suspendisse accumsan quis lorem eget varius. Interdum et malesuada fames ac ante         \n"
"ipsum primis in faucibus. Suspendisse rutrum leo eu ligula hendrerit, nec rutrum         \n"
"nunc finibus. Vestibulum porttitor venenatis quam, quis tincidunt dui venenatis et.      \n"
"Mauris auctor molestie venenatis. Nulla eu tortor vitae leo sagittis euismod eget        \n"
"et elit. Nullam non nibh ex. Curabitur eget ante tortor. Donec interdum viverra          \n"
"turpis tempor hendrerit. Fusce rhoncus accumsan orci, sed placerat diam consectetur et.  \n";

static const char *const large_sting_literal2 =
"Lorem 1psum dolor sit am2t, consectetur a3ipiscing eli4. Pellentesq5e sed turpis nec     \n"
"dolor 1ccumsan aliquet s2d nec massa. Cra3 ultricies p4llentesque u5na, at pulvinar      \n"
"metus 1osuere ac. Etiam 2elis nulla, laci3ia quis dapi4us a, ultric5s ac purus.          \n"
"Donec 1t purus nec sem s2scipit fermentum3 Donec finib4s ex eget do5or ornare,           \n"
"sed fr1ngilla enim imper2iet. Sed tincidu3t mollis ero4. Proin dapi5us magna eu          \n"
"mauris1gravida, a molest2e libero consequ3t. Nunc elei4end imperdie5 interdum.           \n"
"Sed im1erdiet eu enim eg2t vehicula. Pell3ntesque habi4ant morbi tr5stique               \n"
"senect1s et netus et mal2suada fames ac t3rpis egestas4 Ut mattis n5nc nec libero        \n"
"effici1ur lacinia. Praes2nt eleifend quam3augue, eu te4pus lectus h5ndrerit eu.          \n"
"Nunc i1perdiet mollis la2reet. Nunc quis 3isl turpis. 4            5                     \n"
"      1                 2                3            4            5                     \n"
"Praese1t turpis mauris, 2liquet eu porta 3on, malesuad4 ullamcorper5lorem.               \n"
"Donec 1eugiat, nibh sit 2met laoreet soll3citudin, nis4 ligula port5itor sapien,         \n"
"id fri1gilla lorem ex vi2ae leo. Mauris e3et vehicula 4uam. Suspend5sse interdum         \n"
"vehicu1a pharetra. Nunc 2orta ante id lib3ro volutpat,4id fringilla5diam porta.          \n"
"Sed tr1stique congue eli2 et vulputate. D3nec pretium 4acus eu ex c5nvallis porttitor.   \n"
"Nam vo1utpat sapien ante2 quis varius ex 3gestas in. M4ecenas ut ul5rices libero.        \n"
"Etiam 1agittis diam non 2empor eleifend. 3ras commodo 4eo nisl.    5                     \n"
"      1                 2                3            4            5                     \n"
"Suspen1isse elementum pu2vinar quam, nec 3liquet nulla4rhoncus vel.5Nullam consequat     \n"
"risus 1t mauris elementu2 vehicula. Curab3tur placerat4enim massa. 5urabitur eget        \n"
"est co1gue, elementum mi2sed, suscipit ju3to. Aenean c4nsectetur al5quam sollicitudin.   \n"
"Cras v1l porttitor velit2 Donec commodo m3ttis faucibu4. Duis eleif5nd id velit          \n"
"vitae 1rnare. Etiam et f2rmentum erat.   3            4            5                     \n"
"      1                 2                3            4            5                     \n"
"Curabi1ur id molestie en2m. In elementum 3assa eu libe4o fermentum,5non cursus           \n"
"diam f1ingilla. Phasellu2 quis ante nec n3nc tempus gr4vida. Aenean5ultrices,            \n"
"erat e1et porta elementu2, justo tortor a3iquam mauris4 in hendreri5 ex sapien           \n"
"nec ma1ris. Sed a massa 2olutpat, digniss3m justo eget4 rutrum lect5s.                   \n"
"Fusce 1ongue, quam eu co2vallis placerat,3orci sem eui4mod ex, nec 5fficitur             \n"
"magna 1apien ut diam. Ph2sellus interdum 3orem eget se4 mollis, in 5enenatis             \n"
"elit g1avida. Aenean et 2ltrices odio, at3tempus lectu4. Mauris ex 5isl, laoreet         \n"
"in lac1s eget, convallis2ultricies nisi. 3liquam dui a4cu, cursus v5tae bibendum et,     \n"
"dapibu1 sed dui. Donec t2llus quam, molli3 vitae porta4a, posuere e5et nunc.             \n"
"Ut a e1os in nunc elemen2um dapibus. Inte3er turpis pu4us, lacinia 5get porttitor        \n"
"tristi1ue, accumsan et m2gna.            3            4            5                     \n"
"      1                 2                3            4            5                     \n"
"Suspen1isse accumsan qui2 lorem eget vari3s. Interdum 4t malesuada 5ames ac ante         \n"
"ipsum 1rimis in faucibus2 Suspendisse rut3um leo eu li4ula hendreri5, nec rutrum         \n"
"nunc f1nibus. Vestibulum2porttitor venena3is quam, qui4 tincidunt d5i venenatis et.      \n"
"Mauris1auctor molestie v2nenatis. Nulla e3 tortor vita4 leo sagitti5 euismod eget        \n"
"et eli1. Nullam non nibh2ex. Curabitur eg3t ante torto4. Donec inte5dum viverra          \n"
"turpis1tempor hendrerit.2Fusce rhoncus ac3umsan orci, 4ed placerat 5iam consectetur et.  \n";

void test_common() {
  tweak_variant_string test1 = { 0 };

  puts("tweak_variant_string_c_str(NULL) == NULL");
  TEST_CHECK(tweak_variant_string_c_str(NULL) == NULL);

  puts("Run tests for uninitialized string ...");
  TEST_CHECK(tweak_variant_string_is_empty(&test1));
  TEST_CHECK(tweak_variant_is_small_string(&test1));
  TEST_CHECK(tweak_variant_string_c_str(&test1) != NULL);

  puts("Run tests for zero length string ...");
  tweak_assign_string(&test1, "");
  TEST_CHECK(tweak_variant_string_is_empty(&test1));
  TEST_CHECK(tweak_variant_is_small_string(&test1));
  TEST_CHECK(tweak_variant_string_c_str(&test1) != NULL);
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&test1), "") == 0);
  tweak_variant_destroy_string(&test1);
  TEST_CHECK(tweak_variant_string_is_empty(&test1));
  TEST_CHECK(tweak_variant_is_small_string(&test1));

  puts("Run tests for small initialized string ...");
  tweak_assign_string(&test1, "ABCD");
  TEST_CHECK(!tweak_variant_string_is_empty(&test1));
  TEST_CHECK(tweak_variant_is_small_string(&test1));
  TEST_CHECK(tweak_variant_string_c_str(&test1) != NULL);
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&test1), "ABCD") == 0);
  tweak_variant_destroy_string(&test1);
  TEST_CHECK(tweak_variant_string_is_empty(&test1));
  TEST_CHECK(tweak_variant_is_small_string(&test1));

  puts("Run tests for large initialized string ...");
  tweak_assign_string(&test1, large_sting_literal1);
  TEST_CHECK(!tweak_variant_string_is_empty(&test1));
  TEST_CHECK(!tweak_variant_is_small_string(&test1));
  TEST_CHECK(tweak_variant_string_c_str(&test1) != NULL);
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&test1), large_sting_literal1) == 0);
  char * old_ptr = test1.buffers.large_buffer;
  size_t old_capacity = test1.capacity;
  tweak_assign_string(&test1, large_sting_literal2);
  TEST_CHECK(!tweak_variant_string_is_empty(&test1));
  TEST_CHECK(!tweak_variant_is_small_string(&test1));
  TEST_CHECK(tweak_variant_string_c_str(&test1) != NULL);
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&test1), large_sting_literal2) == 0);

  // large_sting_literal1 has lesser size.
  // Reallocation shouldn't happen here.
  TEST_CHECK(old_ptr == test1.buffers.large_buffer);
  TEST_CHECK(old_capacity == test1.capacity);

  tweak_variant_destroy_string(&test1);
  TEST_CHECK(tweak_variant_is_small_string(&test1));

  puts("Run tests for tweak_variant_string_swap ...");
  tweak_assign_string(&test1, large_sting_literal1);
  {
    tweak_variant_string test2 = { 0 };
    tweak_variant_swap_string(&test1, &test2);
    TEST_CHECK(!tweak_variant_is_small_string(&test2));
    TEST_CHECK(tweak_variant_string_c_str(&test2) != NULL);
    TEST_CHECK(strcmp(tweak_variant_string_c_str(&test2), large_sting_literal1) == 0);
    tweak_variant_destroy_string(&test2);
  }
  TEST_CHECK(tweak_variant_is_small_string(&test1));
  tweak_variant_destroy_string(&test1); /* Does nothing,
                                           But if client of the code modelled
                                           by the nested block above
                                           didn't claim ownership of the string,
                                           real deallocation would happen here.
                                        */
}

void test_string_to_json() {
  tweak_variant test1 = TWEAK_VARIANT_INIT_EMPTY;
  tweak_variant_assign_string(&test1, "\"abc\"\\\n");
  tweak_variant_string test2 = tweak_variant_to_json(&test1);
  TEST_CHECK(strcmp(tweak_variant_string_c_str(&test2), "{\"string\": \"\\\"abc\\\"\\\\\\n\"}") == 0);
}

TEST_LIST = {
   { "test_string_to_json", test_string_to_json },
   { "test_common", test_common },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};
