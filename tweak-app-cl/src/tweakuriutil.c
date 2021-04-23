/**
 * @file tweakuriutil.c
 * @ingroup tweak-api
 *
 * @brief populate/filter/sort utilities for collections of tweak uris.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <regex.h>
#include <tweak2/appclient.h>
#include "tweakuriutil.h"
#include "stringutil.h"

struct enumerate_context {
  const char* substring;
  regex_t regex;
  char** uris;
  size_t size;
  size_t reserved;
};

static bool ensure_capacity(struct enumerate_context* enumerate_context, size_t size) {
  if (size >= enumerate_context->reserved) {
    size_t new_reserved = size > 10 ? size * 3 / 2 : 10;
    enumerate_context->uris = realloc(enumerate_context->uris, new_reserved * sizeof(enumerate_context->uris[0]));
    if (!enumerate_context->uris) {
      return false;
    }
    enumerate_context->reserved = new_reserved;
  }
  return true;
}

static int pstr_comparator(const void* lhs, const void* rhs) {
  const char* const* pstr1 = lhs;
  const char* const* pstr2 = rhs;
  return strcmp(*pstr1, *pstr2);
}

static bool enumerate_traverse_strstr(const tweak_app_item_snapshot* snapshot, void* cookie) {
  assert(cookie);
  struct enumerate_context* enumerate_context = (struct enumerate_context*)cookie;

  bool match = enumerate_context->substring
    ? strstr(tweak_variant_string_c_str(&snapshot->uri), enumerate_context->substring) != NULL
    : true;

  if (match) {
    if (!ensure_capacity(enumerate_context, enumerate_context->size + 1))
      return false;

    enumerate_context->uris[enumerate_context->size] = strdup(tweak_variant_string_c_str(&snapshot->uri));
    ++enumerate_context->size;
  }
  return true;
}

struct tweak_app_cl_tweak_uris_list* tweak_app_cl_create_sorted_tweak_uris_list_strstr(tweak_app_client_context context,
                                                                               const char* filter)
{
  struct enumerate_context enumerate_context = {
    .substring = filter
  };

  bool success = tweak_app_traverse_items(context, &enumerate_traverse_strstr, &enumerate_context) 
    && ensure_capacity(&enumerate_context, enumerate_context.size + 1);
  
  if (success)
    enumerate_context.uris[enumerate_context.size] = NULL;

  qsort(enumerate_context.uris, enumerate_context.size,
    sizeof(enumerate_context.uris[0]), &pstr_comparator);

  if (success) {
    struct tweak_app_cl_tweak_uris_list* result = calloc(1, sizeof(*result));
    if (!result) {
      tweak_app_cl_release_tokens(enumerate_context.uris);
      return NULL;
    }

    result->uris = (const char **)enumerate_context.uris;
    result->size = enumerate_context.size;

    return result;
  } else {
    return NULL;
  }
}

static bool enumerate_traverse_regex(const tweak_app_item_snapshot* snapshot, void* cookie) {
  assert(cookie);
  struct enumerate_context* enumerate_context = (struct enumerate_context*)cookie;
  if (regexec(&enumerate_context->regex, tweak_variant_string_c_str(&snapshot->uri), 0, NULL, 0) == 0) {
    if (!ensure_capacity(enumerate_context, enumerate_context->size + 1))
      return false;

    enumerate_context->uris[enumerate_context->size] = strdup(tweak_variant_string_c_str(&snapshot->uri));
    ++enumerate_context->size;
  }
  return true;
}

struct tweak_app_cl_tweak_uris_list* tweak_app_cl_create_sorted_tweak_uris_list_regex(tweak_app_client_context context,
                                                                                      const char* filter)
{
  const char* pattern = ".*";
  if (filter) {
    pattern = filter;
  }

  struct enumerate_context enumerate_context = { 0 };
  int ec = regcomp(&enumerate_context.regex, pattern, 0);
  if (ec != 0) {
    return NULL;
  }

  bool success = tweak_app_traverse_items(context, &enumerate_traverse_regex, &enumerate_context) 
    && ensure_capacity(&enumerate_context, enumerate_context.size + 1);
  
  if (success)
    enumerate_context.uris[enumerate_context.size] = NULL;

  qsort(enumerate_context.uris, enumerate_context.size,
    sizeof(enumerate_context.uris[0]), &pstr_comparator);

  regfree(&enumerate_context.regex);

  if (success) {
    struct tweak_app_cl_tweak_uris_list* result = calloc(1, sizeof(*result));
    if (!result) {
      tweak_app_cl_release_tokens(enumerate_context.uris);
      return NULL;
    }

    result->uris = (const char **)enumerate_context.uris;
    result->size = enumerate_context.size;

    return result;
  } else {
    return NULL;
  }
}

typedef int (*comparator_proc)(const void *, const void *);

static size_t lower_bound(const void *needle, 
                          const void *haystack,
                          size_t tweak_id_count,
                          size_t tweak_id_size,
                          comparator_proc comparator)
{
  const char *byte_array = haystack;
  size_t mid;

  size_t low = 0;
  size_t high = tweak_id_count;

  while (low < high) {
    mid = low + (high - low) / 2U;
    if (comparator(needle, byte_array + (mid * tweak_id_size)) <= 0) {
      high = mid;
    } else {
      low = mid + 1;
    }
  }

  return low;
}

static bool startswith(const char *pre, const char *str) {
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

const char* tweak_app_cl_tweak_uris_list_pick_nth_match(struct tweak_app_cl_tweak_uris_list* uris_list,
                                                        const char* prefix,
                                                        int n_match)
{
  size_t pos;
  pos = lower_bound(&prefix,
    uris_list->uris,
    uris_list->size,
    sizeof(uris_list->uris[0]),
    &pstr_comparator);

  pos += n_match;

  if (pos >= uris_list->size)
    return NULL;

  const char *result = NULL; 
  if (startswith(prefix, uris_list->uris[pos]))
    result = uris_list->uris[pos];

  return result;
}

void tweak_app_cl_destroy_sorted_tweak_uris_list(struct tweak_app_cl_tweak_uris_list* tweak_uris_list) {
  if (!tweak_uris_list)
    return;

  tweak_app_cl_release_tokens((char**)tweak_uris_list->uris);
  free(tweak_uris_list);
}
