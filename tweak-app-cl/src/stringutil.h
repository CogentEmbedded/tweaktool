/**
 * @file stringutil.h
 * @ingroup tweak-api
 *
 * @brief string utilities to handle user unput in tweak-app-cl program.
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

#ifndef TWEAK_APP_CL_STRINGUTIL_INCLUDED
#define TWEAK_APP_CL_STRINGUTIL_INCLUDED

#include <stddef.h>

#define TWEAK_APP_CL_SEPARATORS " \r\n\t"

#define TWEAK_APP_CL_BACKSLASH '\\'

/**
 * Concatenates tokens, interleaving them with @p separator.
 *
 * @param tokens list of tokens to concatenate, terminated with NULL pointer.
 * @param separator string to put between tokens.
 *
 * @return concatenated string. Should be released with @see free(). NULL if malloc() returned NULL.
 */
char* tweak_app_cl_merge_tokens(const char** tokens, const char* separator);

/**
 * @brief Similar to @see strpbrk, but result points to '\0' termiator in case of
 * no match being found.
 *
 * @param arg string to find match in.
 * @param seps string containing all possible separators. @see strpbrk.
 *
 * @return pointer within range arg .. arg + strlen(arg) meeting the criterion.
 */
char* tweak_app_cl_strpbrk_no_null(const char* arg, const char* seps);

/**
 * @brief Returns first char in string @p str that isn't equal to one of separators equal
 * to one of separators enumerated in @p sep.
 *
 * @param arg string to find match in.
 * @param seps string containing all possible separators.
 *
 * @return pointer within range arg .. arg + strlen(arg) meeting the criterion.
 */
char* tweak_app_cl_trimleft(const char *str, const char *separators);

/**
 * @brief Similar to @see tweak_app_cl_strpbrk_no_null, but skips separators
 * prefixed by char provided as @p escape parameter.
 *
 * @param arg string to find match in.
 * @param escape character guarding separators.
 * @param seps string containing all possible separators.
 *
 * @return pointer within range arg .. arg + strlen(arg) meeting the criterion.
 */
char* tweak_app_cl_strpbrk_unescaped_no_null(const char* arg, char escape, const char* seps);

/**
 * @brief removes @p escape separators from string @p arg having certain @p length.
 * Puts result on heap.
 *
 * @note this mathod is designed to tokenize command lines piece by piece,
 * thus @p length parameter is here to extract said pieces.
 *
 * @param arg Source string.
 * @param escape Escape char to remove from string.
 * @param length length on @p arg fragment to handle. Can be smaller than
 * actual length of the string.
 *
 * @return result string allocated on heap. Shall be released by @see free
 * function to avoid memory leak.
 */
char* tweak_app_cl_unescape_str(const char* arg, char escape, size_t length);

/**
 * @brief Tokenize a string having @p escape char and @p separators.
 *
 * @details Suppose, the line is equal to " a\ b c d\  ", escape is
 * '\' and separators are " ". This method shall provide list:
 * ```
 *  |-> "a b"
 *  |-> "c"
 *  |-> "d "
 *  |-> NULL
 * ```
 *
 * @param line Source string.
 * @param escape Escape char to guard separators.
 * @param separators string containing all possible separators.
 *
 * @return list of pointers to strings allocated on heap terminated by NULL.
 * Each string is allocated on heap as well. Use @see tweak_app_cl_release_tokens
 * to deallocate heap memory occupied by this list and its contents.
 */
char** tweak_app_cl_tokenize(const char* line, char escape, const char* separators);

/**
 * @brief Helper method to deallocate heap memory claimed by @see tweak_app_cl_tokenize.
 *
 * @param tokens list of tokens.
 */
void tweak_app_cl_release_tokens(char** tokens);

/**
 * @brief Helper method to determine which token in command line requires completion.
 *
 * @details Suppose, escape is '\' and separators are " ".
 * Then, this method shall return
 * - 0 for lines " li" (leading space isn't a typo here) or "list",
 * - 1 for lines "list " or " list /" or even " list /foo/b\ ar",
 * - 2 for line " set /foo/b\ ar ".
 *
 * @param escape Escape char to guard separators.
 * @param separators string containing all possible separators.
 *
 * @return number of token in line that should be completed.
 */
int tweak_app_cl_find_first_incomplete_token(const char* line, char escape, const char* separators);

#endif
