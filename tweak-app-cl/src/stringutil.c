/**
 * @file stringutil.c
 * @ingroup tweak-api
 *
 * @brief string utilities to handle user unput in tweak-app-cl program.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <stdlib.h>
#include <string.h>
#include "stringutil.h"

char** tweak_app_cl_tokenize(const char* line, char escape, const char* separators) {
  char** result = NULL;
  size_t count = 0;
  const char* prev_p = tweak_app_cl_trimleft(line, separators);
  const char* p = prev_p;
  p = tweak_app_cl_strpbrk_unescaped_no_null(p, escape, separators);
  while(*prev_p) {
    result = realloc(result, sizeof(result) * (count + 1));

    if (!result)
      return NULL;

    result[count] = *prev_p ? tweak_app_cl_unescape_str(prev_p, escape, p - prev_p) : NULL;
    ++count;

    prev_p = tweak_app_cl_trimleft(p, separators);
    p = tweak_app_cl_strpbrk_unescaped_no_null(prev_p, escape, separators);
  }
  
  result = realloc(result, sizeof(result) * (count + 1));
  if (!result)
    return NULL;

  result[count] = NULL;

  return result;
}

void tweak_app_cl_release_tokens(char** tokens) {
  if (!tokens)
    return;

  size_t ix = 0;
  while (tokens[ix] != NULL) {
    free(tokens[ix]);
    ++ix;
  }
  free(tokens);
}

char* tweak_app_cl_unescape_str(const char* arg, char escape, size_t length) {
  char* result = malloc(1);
  if (!result)
    return NULL;

  *result = '\0';
  size_t size = 0;
  size_t reserved = 0;

  char* pos = (char*)arg;
  char* end = pos + length;

  while (*pos && pos < end) {
    if (size + 1 >= reserved) {
      size_t new_reserved = size + 1 >= 10 ? (size + 1) * 3 / 2 : 10;
      result = realloc(result, new_reserved);

      if (!result)
        return NULL;

      reserved = new_reserved;
    }

    result[size++] = *pos++;
    if (*pos == escape)
      ++pos;
  }

  if (size + 1 >= reserved) {
    result = realloc(result, size + 1);
    if(!result)
      return NULL;
  }

  result[size] = '\0';
  return result;
}

char* tweak_app_cl_strpbrk_unescaped_no_null(const char* arg, char escape, const char* seps) {
  char prev_symbol;
  char* result;

  result = tweak_app_cl_strpbrk_no_null(arg, seps);
  prev_symbol = result > arg ? *(result - 1) : '\0';

  while (*result && prev_symbol == escape) {
    result = tweak_app_cl_strpbrk_no_null(result + 1, seps);
    prev_symbol = *(result - 1);
  }

  return result;
}

char* tweak_app_cl_trimleft(const char *str, const char *separators) {
  if (!str)
    return NULL;

  while (strpbrk(str, separators) == str)
    ++str;

  return (char*)str;
}

char* tweak_app_cl_strpbrk_no_null(const char* arg, const char* seps) {
  if (!arg)
    return NULL;

  if (!seps)
    return (char*)arg; 

  char* result;
  result = strpbrk(arg, seps);
  if (!result)
    result = (char*)(arg + strlen(arg));

  return result;
}

int tweak_app_cl_find_first_incomplete_token(const char* line, char escape, const char* separators) {
  int incomplete_token_found = 0;
  char* token_pos;
  char* separator_pos;
  
  token_pos = tweak_app_cl_trimleft(line, separators);
  separator_pos = tweak_app_cl_strpbrk_unescaped_no_null(token_pos, escape, TWEAK_APP_CL_SEPARATORS);
  while (*separator_pos) {
    ++incomplete_token_found;
    token_pos = tweak_app_cl_trimleft(separator_pos, separators);
    separator_pos = tweak_app_cl_strpbrk_unescaped_no_null(token_pos, escape, TWEAK_APP_CL_SEPARATORS);
  }
  
  return incomplete_token_found;
}
