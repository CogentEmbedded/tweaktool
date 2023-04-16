/**
 * @file main.c
 * @ingroup tweak-api
 *
 * @brief Simple REPL based user client for tweak protocol.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#include <tweak2/appclient.h>
#include <tweak2/defaults.h>
#include <tweak2/log.h>
#include <tweak2/thread.h>

#include "stringutil.h"
#include "tweakuriutil.h"
#include "metadatautil.h"

#include <getopt.h>
#include <inttypes.h>
#include <regex.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/history.h>
#include <readline/readline.h>
#include <wordexp.h>

struct tweak_uri_list {
  tweak_app_client_context app_context;
  struct tweak_app_cl_tweak_uris_list* uris_list;
  tweak_common_mutex lock;
};

static bool s_exit = false;

static const char* s_value_being_edited = NULL;

static struct tweak_uri_list* s_tweak_uri_list;

static const char* s_connection_type = NULL;

static const char* s_params = NULL;

static const char* s_uri = NULL;

static tweak_id s_tweak_id = TWEAK_INVALID_ID;

static bool s_is_connected = false;

static FILE* s_log_output = NULL;

static void clear_uri_list(tweak_app_context context, tweak_id id, void *cookie);

static char* get_nth_match(const char* str, size_t length, int n);

const char* get_connection_type() {
  return s_connection_type ? s_connection_type : "nng";
}

const char* get_params() {
  return s_params ? s_params : "role=client";
}

const char* get_uri() {
  return s_uri ? s_uri : TWEAK_DEFAULT_ENDPOINT;
}

static const char* s_prompt = NULL;

void cleanup() {
  if (s_log_output != NULL) {
    fclose(s_log_output);
  }
}

static void connection_status_changed(tweak_app_context context,
  bool is_connected, void *cookie)
{
  TWEAK_LOG_TRACE_ENTRY("context = %p is_connected = %s cookie = %p", context, is_connected ? "true" : "false", cookie);
  (void) context;
  (void) cookie;
  s_is_connected = is_connected;
}

static void update_prompt() {
  char prompt[64];
  if (s_tweak_id != TWEAK_INVALID_ID) {
    snprintf(prompt, sizeof(prompt), "id : %" PRIu64 " > ", s_tweak_id);
  } else {
    strncpy(prompt, "id : NOT SELECTED > ", sizeof(prompt));
  }
  const char* tmp = s_prompt;
  s_prompt = strdup(prompt);
  free((void*)tmp);
}

static void print_snapshot(const tweak_app_item_snapshot* snapshot) {
  tweak_id tweak_id = snapshot->id;
  tweak_variant_string current_value_str =
    tweak_app_cl_metadata_aware_variant_to_string(&snapshot->current_value, &snapshot->meta);
  tweak_variant_string default_value_str =
    tweak_app_cl_metadata_aware_variant_to_string(&snapshot->default_value, &snapshot->meta);

  printf("tweak_id:%" PRIu64 ", uri:\"%s\", description:\"%s\", meta:\"%s\""
    ", default_value:\"%s\", current_value:\"%s\"\n",
    tweak_id,
    tweak_variant_string_c_str(&snapshot->uri),
    tweak_variant_string_c_str(&snapshot->description),
    tweak_variant_string_c_str(&snapshot->meta),
    tweak_variant_string_c_str(&default_value_str),
    tweak_variant_string_c_str(&current_value_str));

  tweak_variant_destroy_string(&default_value_str);
  tweak_variant_destroy_string(&current_value_str);
}

static void print_brief(const tweak_app_item_snapshot* snapshot) {
  tweak_id tweak_id = snapshot->id;
  tweak_variant_string current_value_str =
    tweak_app_cl_metadata_aware_variant_to_string(&snapshot->current_value, &snapshot->meta);

  printf("tweak_id:%" PRIu64 ", uri:\"%s\", value: %s\n",
    tweak_id,
    tweak_variant_string_c_str(&snapshot->uri),
    tweak_variant_string_c_str(&current_value_str));

  tweak_variant_destroy_string(&current_value_str);
}

static void execute_list_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  bool use_regex = tokens[0] && tokens[1]
    && (strcmp("re", tokens[0]) == 0 || strcmp("regex", tokens[0]) == 0);

  const char* pattern = use_regex ? tokens[1] : tokens[0];
  struct tweak_app_cl_tweak_uris_list* uris_list = use_regex
    ? tweak_app_cl_create_sorted_tweak_uris_list_regex(context, pattern)
    : tweak_app_cl_create_sorted_tweak_uris_list_strstr(context, pattern);

  if (!uris_list) {
    fprintf(stderr, "ERROR: Can't build uri list. Review regex search pattern.\n");
    return;
  }

  const char **p = uris_list->uris;
  while (*p) {
    tweak_id tweak_id = tweak_app_find_id(context, *p);
    if (tweak_id != TWEAK_INVALID_ID) {
      tweak_app_item_snapshot* snapshot = tweak_app_item_get_snapshot(context, tweak_id);
      if (snapshot) {
        print_brief(snapshot);
        tweak_app_release_snapshot(context, snapshot);
      }
    }
    ++p;
  }
  tweak_app_cl_destroy_sorted_tweak_uris_list(uris_list);
}

static void execute_details_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  bool use_regex = tokens[0] && tokens[1]
    && (strcmp("re", tokens[0]) == 0 || strcmp("regex", tokens[0]) == 0);

  const char* pattern = use_regex ? tokens[1] : tokens[0];
  struct tweak_app_cl_tweak_uris_list* uris_list = use_regex
    ? tweak_app_cl_create_sorted_tweak_uris_list_regex(context, pattern)
    : tweak_app_cl_create_sorted_tweak_uris_list_strstr(context, pattern);

  if (!uris_list) {
    fprintf(stderr, "ERROR: Can't build uri list. Review search pattern.\n");
    return;
  }

  const char **p = uris_list->uris;
  while (*p) {
    tweak_id tweak_id = tweak_app_find_id(context, *p);
    if (tweak_id != TWEAK_INVALID_ID) {
      tweak_app_item_snapshot* snapshot = tweak_app_item_get_snapshot(context, tweak_id);
      if (snapshot) {
        print_snapshot(snapshot);
        tweak_app_release_snapshot(context, snapshot);
      }
    }
    ++p;
  }
  tweak_app_cl_destroy_sorted_tweak_uris_list(uris_list);
}

static void execute_select_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  if (tokens[0]) {
    tweak_id tweak_id = tweak_app_find_id(context, tokens[0]);
    if (tweak_id != TWEAK_INVALID_ID) {
      s_tweak_id = tweak_id;
      update_prompt();
    } else {
      fprintf(stderr, "ERROR: Unknown uri : %s.\n"
        "Make sure that specified uri exists. Use list command with a pattern.\n", tokens[0]);
    }
  } else {
    printf("NOTE: No uri provided. Existing selection removed.\n");
    s_tweak_id = TWEAK_INVALID_ID;
  }
}

static tweak_id get_tweak_id(tweak_app_client_context context, const char* uri, bool* explicit_uri) {
  if (uri && uri[0] != '[') {
    const char *tweak_uri = uri;
    tweak_id result = tweak_app_find_id(context, tweak_uri);
    if (result == TWEAK_INVALID_ID) {
      fprintf(stderr, "ERROR: Unknown uri : %s.\n"
        "Make sure that specified uri exists. Use list command with a pattern.\n", tweak_uri);
    }
    if (explicit_uri) {
      *explicit_uri = true;
    }
    return result;
  } else if (s_tweak_id != TWEAK_INVALID_ID) {
    if (explicit_uri) {
      *explicit_uri = false;
    }
    return s_tweak_id;
  } else {
    fprintf(stderr, "ERROR: Tweak hasn't been selected.\n"
      "Either choose one with select command or provide tweak uri as an argument.\n");
    return TWEAK_INVALID_ID;
  }
}

static void execute_wait_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  if (tokens[0] != NULL) {
    const char* uris[] = { tokens[0] };
    tweak_id ids[1];
    if (tweak_app_client_wait_uris(context, uris, 1, ids, TWEAK_COMMON_TIMESPAN_INFINITE) == TWEAK_APP_SUCCESS) {
      printf("Wait success. Item ID = %" PRIu64 "\n", ids[0]);
    } else {
      fprintf(stderr, "ERROR: tweak_app_client_wait_uris() failed\n");
    }
  } else {
    fprintf(stderr, "ERROR: wait command requires mandatory uri parameter\n");
  }
}

static void execute_get_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  tweak_id tweak_id = get_tweak_id(context, tokens[0], NULL);
  if (tweak_id == TWEAK_INVALID_ID)
    return;

  tweak_app_item_snapshot* snapshot = tweak_app_item_get_snapshot(context, tweak_id);
  if (snapshot) {
    tweak_variant_string string_repr =
      tweak_app_cl_metadata_aware_variant_to_string(&snapshot->current_value, &snapshot->meta);
    const char* format = s_is_connected
      ? "%s\n"
      : "Last known value : %s\n";
    printf(format, tweak_variant_string_c_str(&string_repr));
    tweak_variant_destroy_string(&string_repr);
    tweak_app_release_snapshot(context, snapshot);
  } else {
    TWEAK_LOG_ERROR("tweak_app_item_get_snapshot() with context = %p and tweak_id = %" PRIu64 "", context, tweak_id);
    fprintf(stderr, "ERROR: Internal tweak-app-cl error. Check error logs\n");
  }
}

static void execute_set_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  if (!tokens[0]) {
    fprintf(stderr, "ERROR: A value is required for 'set' command.\n"
      " Please provide a value.\n");
    return;
  }

  bool explicit_uri = false;
  tweak_id tweak_id = get_tweak_id(context, tokens[0], &explicit_uri);
  if (tweak_id == TWEAK_INVALID_ID)
    return;

  char* value_str = explicit_uri
    ? tweak_app_cl_merge_tokens((const char**)&tokens[1], " ")
    : tweak_app_cl_merge_tokens((const char**)&tokens[0], " ");

  if (value_str == NULL) {
    fprintf(stderr, "ERROR: Out of memory");
    return;
  }

  tweak_app_item_snapshot *snapshot = tweak_app_item_get_snapshot(context, tweak_id);
  if (snapshot && snapshot->current_value.type != TWEAK_VARIANT_TYPE_NULL) {
    tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
    tweak_variant_type_conversion_result conversion_result =
       tweak_app_cl_metadata_aware_variant_from_string(value_str, snapshot->current_value.type,
         tweak_variant_get_item_count(&snapshot->current_value), &snapshot->meta, &value);
    if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS
        || conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED)
    {
      if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED) {
        tweak_variant_string new_value_string =
          tweak_app_cl_metadata_aware_variant_to_string(&value, &snapshot->meta);
        fprintf(stderr, "WARNING: Value %s was truncated to %s. Please check this value.\n",
          value_str, tweak_variant_string_c_str(&new_value_string));
        tweak_variant_destroy_string(&new_value_string);
      }
      tweak_app_error_code error_code = tweak_app_item_replace_current_value(context,
        tweak_id, &value);
      if (error_code == TWEAK_APP_SUCCESS) {
        printf("Ok\n");
      } else if (error_code == TWEAK_APP_PEER_DISCONNECTED) {
        fprintf(stderr, "ERROR: Server is disconnected, can't update value.\n"
          "Please check network connection.\n");
      } else {
        fprintf(stderr, "Tweak client error : %x\n", error_code);
      }
      tweak_variant_destroy(&value);
    } else {
      fprintf(stderr, "ERROR: Can't parse string \"%s\" to value.\n"
        "Please make sure it has a valid format.\n", value_str);
    }
    tweak_app_release_snapshot(context, snapshot);
  } else {
    TWEAK_LOG_ERROR("tweak_app_item_get_snapshot() with context = %p and tweak_id = %" PRIu64 "", context, tweak_id);
    fprintf(stderr, "ERROR: Internal tweak-app-cl error. Check error logs\n");
  }
  free(value_str);
}

#if !defined(__QNX__) && !defined(__APPLE__)
static int initial_readline_content_hook() {
  if (s_value_being_edited) {
    rl_insert_text (s_value_being_edited);
    s_value_being_edited = NULL;
    rl_startup_hook = (rl_hook_func_t *) NULL;
  }
  return 0;
}
#endif

static char* readline_edit(const char* prompt, const char* initial_text) {
  s_value_being_edited = initial_text;
#if !defined(__QNX__) && !defined(__APPLE__)
  rl_startup_hook = initial_readline_content_hook;
#endif
  return readline(prompt);
}

static void execute_edit_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  tweak_id tweak_id = get_tweak_id(context, tokens[0], NULL);
  if (tweak_id == TWEAK_INVALID_ID)
    return;

  tweak_app_item_snapshot *snapshot = tweak_app_item_get_snapshot(context, tweak_id);
  if (snapshot && snapshot->current_value.type != TWEAK_VARIANT_TYPE_NULL) {
    tweak_variant_string string_repr =
      tweak_app_cl_metadata_aware_variant_to_string(&snapshot->current_value, &snapshot->meta);
    tweak_variant_string prompt = TWEAK_VARIANT_STRING_EMPTY;
    tweak_string_format(&prompt, "Edit id : %" PRIu64 " = ", tweak_id);
    char* line = readline_edit(tweak_variant_string_c_str(&prompt), tweak_variant_string_c_str(&string_repr));
    tweak_variant_destroy_string(&string_repr);
    tweak_variant_destroy_string(&prompt);

    tweak_variant new_value = TWEAK_VARIANT_INIT_EMPTY;
    size_t item_count = tweak_variant_get_item_count(&snapshot->current_value);
    tweak_variant_type_conversion_result conversion_result =
      tweak_app_cl_metadata_aware_variant_from_string(line,
      snapshot->current_value.type, item_count, &snapshot->meta, &new_value);
    free(line);
    if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS) {
      tweak_app_error_code error_code = tweak_app_item_replace_current_value(context, tweak_id, &new_value);
      if (error_code == TWEAK_APP_SUCCESS) {
        printf("Ok\n");
      } else if (error_code == TWEAK_APP_PEER_DISCONNECTED) {
        fprintf(stderr, "ERROR: Server is disconnected, can't update value.\n"
          "Please check network connection.\n");
      } else {
        fprintf(stderr, "Tweak client error : %x\n", error_code);
      }
    } else {
      fprintf(stderr, "ERROR: Can't parse edited value.\n");
    }
    tweak_app_release_snapshot(context, snapshot);
  } else {
    TWEAK_LOG_ERROR("tweak_app_item_get_snapshot() with context = %p and tweak_id = %" PRIu64 "", context, tweak_id);
    fprintf(stderr, "ERROR: Internal tweak-app-cl error. Check error logs\n");  }
  }

static void execute_line(tweak_app_client_context context, char* line);

static char* expand_tilde(const char* arg) {
  char* result = NULL;
  wordexp_t exp_result = { 0 };
  wordexp(arg, &exp_result, 0);
  if (exp_result.we_wordc == 1) {
    result = strdup(exp_result.we_wordv[0]);
  }
  wordfree(&exp_result);
  return result;
}

static void run_script(tweak_app_client_context context, FILE* file);

static void generate_script(tweak_app_client_context context, FILE* file);

static void execute_load_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  if (tokens[0]) {
    char* filename = expand_tilde(tokens[0]);
    if (filename) {
      FILE* file = fopen(filename, "r");
      if (file) {
        run_script(context, file);
        fclose(file);
      } else {
        fprintf(stderr, "ERROR: Can't open file for reading: %s.\n"
          "Please check that file does exist and you have read permissions.\n", filename);
      }
    } else {
      fprintf(stderr, "ERROR: Filename %s is ambiguous\n", tokens[0]);
    }
    free(filename);
  } else {
    fprintf(stderr, "ERROR: Usage: load <filename>\n"
      "Please provide valid filename.\n");
  }
}

static void execute_save_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  if (tokens[0]) {
    char* filename = expand_tilde(tokens[0]);
    if (filename) {
      FILE* file = fopen(filename, "w");
      if (file) {
        generate_script(context, file);
        fclose(file);
      } else {
        fprintf(stderr, "ERROR: Can't open file for writing: %s\n"
          "Please check that file does exist and you have read permissions.\n", filename);
      }
      free(filename);
    } else {
      fprintf(stderr, "ERROR: Filename %s is ambiguous\n", tokens[0]);
    }
  } else {
    fprintf(stderr, "ERROR: Usage: save <filename>\n"
      "Please provide valid filename.\n");
  }
}

static void run_script(tweak_app_client_context context, FILE* file) {
  char* line = NULL;
  ssize_t nread;
  for (;;) {
    size_t len = 0;
    nread = getline(&line, &len, file);
    if (nread < 0)
      break;

    execute_line(context, line);
    if (s_exit)
      break;
  }
  free(line);
}

static void generate_script(tweak_app_client_context context, FILE* file) {
  struct tweak_app_cl_tweak_uris_list* uris_list = tweak_app_cl_create_sorted_tweak_uris_list_regex(context, NULL);
  for (size_t ix = 0; ix < uris_list->size; ++ix) {
    tweak_id tweak_id = tweak_app_find_id(context, uris_list->uris[ix]);
    if (tweak_id != TWEAK_INVALID_ID) {
      tweak_app_item_snapshot* snapshot = tweak_app_item_get_snapshot(context, tweak_id);
      if (snapshot) {
        tweak_variant_string current_value_str =
          tweak_app_cl_metadata_aware_variant_to_string(&snapshot->current_value, &snapshot->meta);
        tweak_variant_string default_value_str =
          tweak_app_cl_metadata_aware_variant_to_string(&snapshot->default_value, &snapshot->meta);

        fprintf(file, "\n");
        fprintf(file, "# Item uri: %s\n", tweak_variant_string_c_str(&snapshot->uri));
        fprintf(file, "# Description : %s\n", tweak_variant_string_c_str(&snapshot->description));
        fprintf(file, "# Meta : %s\n", tweak_variant_string_c_str(&snapshot->meta));
        fprintf(file, "# Default value : %s\n", tweak_variant_string_c_str(&default_value_str));
        fprintf(file, "\n");

        fprintf(file, "set %s %s\n",
                tweak_variant_string_c_str(&snapshot->uri),
                tweak_variant_string_c_str(&current_value_str));

        tweak_variant_destroy_string(&default_value_str);
        tweak_variant_destroy_string(&current_value_str);
        tweak_app_release_snapshot(context, snapshot);
      }
    }
  }
  tweak_app_cl_destroy_sorted_tweak_uris_list(uris_list);
}

static void execute_exit_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  (void) context;
  (void) tokens;
  s_exit = true;
}

static void execute_help_cmd(tweak_app_client_context context, char **tokens);

static char** command_line_completion(const char *text, int start, int end);

int main_loop(tweak_app_client_context context) {
  char *line = NULL;
  using_history();
  update_prompt();
  rl_attempted_completion_function = command_line_completion;
  while (!s_exit) {
    line = readline(s_prompt);
    add_history(line);
    execute_line(context, line);
    if (!s_is_connected) {
      fprintf(stderr, "ERROR: Client is in disconnected state.\n"
        "Please check network availability.\n");
    }
    update_prompt();
    free(line);
  }
  return 0;
}

typedef void (*command_handler_proc)(tweak_app_client_context context, char **tokens);

typedef char* (*guess_generator_proc)(const char * line, int state);

struct command_handler_pair {
  const char* command;
  guess_generator_proc guess_generator;
  command_handler_proc handler;
  const char* help_string;
};

static char* guess_command(const char *text, int state);

static char* guess_tweak_uri(const char *text, int state);

static char* guess_no_arg(const char *text, int state);

static const char s_quick_help[] =
  "Supported commands are: \n"
  " - ? or help [ command ]\n"
  " - wait <tweak_uri>\n"
  " - details [ pattern ]\n"
  " - list [ pattern ]\n"
  " - load filename\n"
  " - save filename\n"
  " - select tweak_uri\n"
  " - get [ tweak_uri ]\n"
  " - edit [ tweak_uri ]\n"
  " - set [ tweak_uri ] value\n"
  " - exit\n";

static const char s_help_help[] =
  "Prints help on specific command, or list of commands if its optional argument is omitted.\n";

static const char s_wait_help[] =
  "Blocks until item with given uri appears in scope.\n";

static const char s_help_list[] =
  "This function is invoked by list command followed by optional pattern.\n"
  "It displays all items whose uris matches provided filter.\n"
  "User could provide POSIX regex prefixed by \"re\" argument, as in \"list re [ABC]/d\"\n"
  "that will match all occurrences of A/d, B/d, or C/d.\n"
  "If pattern is omitted, no filter is applied.\n";

static const char s_help_details[] =
  "This function is invoked by details command followed by optional pattern.\n"
  "It displays all items whose uris matches provided filter.\n"
  "User could provide POSIX regex prefixed by \"re\" argument, as in \"details re [ABC]/d\"\n"
  "that will match all occurrences of A/d, B/d, or C/d.\n"
  "If pattern is omitted, no filter is applied.\n";

static const char s_help_load[] =
  "This command allows to run a batch consisting of commands provided by this REPL.\n"
  "It has a mandatory filename argument.\n";

static const char s_help_save[] =
  "This command allows to store a dump of the current state of a server as a sequence of set commands.\n"
  "Metainformation is written as comments to set commands.\n"
  "It has a mandatory filename argument.\n";

static const char s_help_select[] =
  "Select command chooses a single default item to access with set or get commands.\n"
  "It has an optional tweak_uri argument. When no tweak_uri given, it clears current selection.\n";

static const char s_help_get[] =
  "This command displays most recent value of an item.\n"
  "Like set, it has two variations: one allowing to pick an item explicitly by its uri\n"
  "and one that gives value of an item previously selected by select command.\n"
  "Former one takes a single uri argument, latter doesn't require any argument at all.\n";

static const char s_help_set[] =
  "This command sends updated value to the connected server.\n"
  "Two variants of this command available: with three arguments and with two arguments.\n"
  "Former command affects value of an item provided explicitly by uri,\n"
  "latter affects value of an item previously selected with select command.\n";

static const char s_help_edit[] =
  "This command allows to edit value interactively and sends updated value to the connected server.\n"
  "Like set, it has two variations: one allowing to pick an item explicitly by its uri\n"
  "and one that gives value of an item previously selected by select command.\n"
  "Former one takes a single uri argument, latter doesn't require any argument at all.\n";

static const char s_help_exit[] =
  "Exits from this repl. User might use Ctrl+C instead, of course.\n";

struct command_handler_pair command_handler_pairs[] = {
  { "wait", &guess_tweak_uri, &execute_wait_cmd, &s_wait_help[0] },
  { "list", &guess_tweak_uri, &execute_list_cmd, &s_help_list[0] },
  { "details", &guess_tweak_uri, &execute_details_cmd, &s_help_details[0] },
  { "select", &guess_tweak_uri, &execute_select_cmd, &s_help_select[0] },
  { "get", &guess_tweak_uri, &execute_get_cmd, &s_help_get[0] },
  { "set", &guess_tweak_uri, &execute_set_cmd, &s_help_set[0] },
  { "edit", &guess_tweak_uri, &execute_edit_cmd, &s_help_edit[0] },
  { "load", &rl_filename_completion_function, &execute_load_cmd, &s_help_load[0] },
  { "save", &rl_filename_completion_function, &execute_save_cmd, &s_help_save[0] },
  { "help", &guess_command, &execute_help_cmd, &s_help_help[0] },
  { "?", &guess_command, &execute_help_cmd, &s_help_help[0] },
  { "exit", &guess_no_arg, &execute_exit_cmd, &s_help_exit[0] },
  { NULL, NULL, NULL, NULL }
};

static void execute_help_cmd(tweak_app_client_context context, char **tokens) {
  TWEAK_LOG_TRACE_ENTRY("context = %p tokens = %p", context, tokens);
  (void) context;
  bool handled = false;
  if (tokens[0]) {
    for (size_t ix = 0; command_handler_pairs[ix].command != NULL; ++ix) {
      if (strcmp(command_handler_pairs[ix].command, tokens[0]) == 0
          && command_handler_pairs[ix].help_string != NULL)
      {
        puts(command_handler_pairs[ix].help_string);
        handled = true;
        break;
      }
    }
  }
  if (!handled)
    puts(s_quick_help);
}

static char* command_line_generator(const char *text, int state) {
  char* result = NULL;
  int first_incomplete_token = tweak_app_cl_find_first_incomplete_token(rl_line_buffer, TWEAK_APP_CL_BACKSLASH, TWEAK_APP_CL_SEPARATORS);
  switch (first_incomplete_token) {
  case 0:
    result = guess_command(text, state);
    break;
  case 1: {
    char** tokens = tweak_app_cl_tokenize(rl_line_buffer, TWEAK_APP_CL_BACKSLASH, TWEAK_APP_CL_SEPARATORS);
    for (size_t ix = 0; command_handler_pairs[ix].command != NULL; ++ix) {
      if (strcmp(command_handler_pairs[ix].command, tokens[0]) == 0
          && command_handler_pairs[ix].guess_generator != NULL)
      {
        result = command_handler_pairs[ix].guess_generator(text, state);
        break;
      }
    }
    tweak_app_cl_release_tokens(tokens);
  } break;
  default:
    result = NULL;
  }
  return result;
}

static char** command_line_completion(const char *text, int start, int end) {
  (void) start;
  (void) end;
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, &command_line_generator);
}

static char* guess_no_arg(const char *text, int state) {
  (void) text;
  (void) state;
  return NULL;
}

static char* guess_command(const char *text, int state) {
  static int list_index = 0;
  static int len = 0;

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  for (;;) {
    if (list_index >= (int)(sizeof(command_handler_pairs) / sizeof(command_handler_pairs[0])))
      break;

    const char *name = command_handler_pairs[list_index++].command;
    if (!name)
      break;

    if (strncmp(name, text, len) == 0)
      return strdup(name);
  }

  return NULL;
}

static char* guess_tweak_uri(const char *text, int state) {
  static int list_index = 0, len = 0;

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  return get_nth_match(text, len, list_index++);
}

static void execute_line(tweak_app_client_context context, char* line) {
  TWEAK_LOG_TRACE_ENTRY("context = %p line = %s", context, line);

  char* comment_pos = strchr(line, '#');
  if (comment_pos)
    *comment_pos = '\0';

  char** tokens = tweak_app_cl_tokenize(line, TWEAK_APP_CL_BACKSLASH, TWEAK_APP_CL_SEPARATORS);
  if (tokens && tokens[0]) {
    bool handled = false;
    for (size_t ix = 0; command_handler_pairs[ix].command != NULL; ix++) {
      if (strcmp(command_handler_pairs[ix].command, tokens[0]) == 0
          && command_handler_pairs[ix].handler != NULL)
      {
        command_handler_pairs[ix].handler(context, &tokens[1]);
        handled = true;
        break;
      }
    }
    if (!handled) {
      fprintf(stderr, "ERROR: Unknown command: %s\n"
        "List of valid commands follows, please use one of them.\n"
        "The help command with an argument could provide more specific\n"
        "info on each command and their typical use-cases.\n", tokens[0]);
      fprintf(stderr, s_quick_help);
    }
  }
  tweak_app_cl_release_tokens(tokens);
}

static void clear_uri_list(tweak_app_context context, tweak_id id, void *cookie) {
  TWEAK_LOG_TRACE_ENTRY("context = %p, id = %" PRIu64 ", cookie = %p", context, id, cookie);
  struct tweak_uri_list *tweak_uri_list = (struct tweak_uri_list *)cookie;
  tweak_common_mutex_lock(&tweak_uri_list->lock);
  tweak_app_cl_destroy_sorted_tweak_uris_list(tweak_uri_list->uris_list);
  tweak_uri_list->uris_list = NULL;
  tweak_common_mutex_unlock(&tweak_uri_list->lock);
}

static void check_id_and_clear_uri_list(tweak_app_context context, tweak_id id, void *cookie) {
  if (id == s_tweak_id) {
    s_tweak_id = TWEAK_INVALID_ID;
    update_prompt();
  }
  clear_uri_list(context, id, cookie);
}

static char* get_nth_match(const char* str, size_t length, int n) {
  (void) length;
  if (!s_tweak_uri_list)
    return NULL;

  struct tweak_uri_list *tweak_uri_list = s_tweak_uri_list;
  char* result = NULL;

  tweak_common_mutex_lock(&tweak_uri_list->lock);
  if (!tweak_uri_list->uris_list)
    tweak_uri_list->uris_list = tweak_app_cl_create_sorted_tweak_uris_list_strstr(tweak_uri_list->app_context, NULL);

  const char* match = tweak_app_cl_tweak_uris_list_pick_nth_match(tweak_uri_list->uris_list, str, n);
  if (match)
    result = strdup(match);

  tweak_common_mutex_unlock(&tweak_uri_list->lock);
  return result;
}

static void output_proc(const char* string) {
  if (s_log_output != NULL) {
    fputs(string, s_log_output);
    fputc('\n', s_log_output);
  }
}

int main(int argc, char **argv) {
  int result = EXIT_FAILURE;
  tweak_common_set_custom_handler(&output_proc);
  struct tweak_uri_list tweak_uri_list = { 0 };
  tweak_common_mutex_init(&tweak_uri_list.lock);
  s_tweak_uri_list = &tweak_uri_list;

  int opt;
  atexit(&cleanup);
  while ((opt = getopt(argc, argv, "t:p:u:L:")) != -1) {
    switch (opt) {
    case 't':
      s_connection_type = optarg;
      break;
    case 'p':
      s_params = optarg;
      break;
    case 'u':
      s_uri = optarg;
      break;
    case 'L':
      s_log_output = fopen(optarg, "wa+");
      if (!s_log_output) {
        TWEAK_FATAL("Can't open file: %s", optarg);
      }
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-t connection type] [-p params] [-u uri]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  tweak_app_client_callbacks client_callbacks = {
    .cookie = &tweak_uri_list,
    .on_connection_status_changed = &connection_status_changed,
    .on_new_item = &clear_uri_list,
    .on_item_removed = &check_id_and_clear_uri_list
  };

  tweak_common_mutex_lock(&tweak_uri_list.lock);
  tweak_app_client_context app_context = tweak_app_create_client_context(
    get_connection_type(), get_params(), get_uri(), &client_callbacks);
  tweak_uri_list.app_context = app_context;
  tweak_common_mutex_unlock(&tweak_uri_list.lock);

  if (app_context) {
    result = main_loop(app_context);
    tweak_app_flush_queue(app_context);
    tweak_app_destroy_context(app_context);
  } else {
    fprintf(stderr, "ERROR: Can't create context\n"
      "Please verify program arguments.\n");
    result = EXIT_FAILURE;
  }
  return result;
}
