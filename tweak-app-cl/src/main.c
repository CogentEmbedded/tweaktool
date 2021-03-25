/**
 * @file main.c
 * @ingroup tweak-api
 *
 * @brief Simple REPL based user client for tweak protocol.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/appclient.h>

#include "stringutil.h"
#include "tweakuriutil.h"

#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
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
  pthread_mutex_t lock;
};

static bool s_exit = false;

static struct tweak_uri_list* s_tweak_uri_list;

static const char* s_connection_type = NULL;

static const char* s_params = NULL;

static const char* s_uri = NULL;

static const char* s_run_param = NULL;

static void clear_uri_list(tweak_app_context context, tweak_id id, void *cookie);

static char* get_nth_match(const char* str, size_t length, int n);

const char* get_connection_type() {
  return s_connection_type ? s_connection_type : "nng";
}

const char* get_params() {
  return s_params ? s_params : "role=client";
}

const char* get_uri() {
  return s_uri ? s_uri : "tcp://0.0.0.0:7777/";
}

static const char* s_prompt = NULL;

void free_global_params() {
  free((void*)s_connection_type);
  free((void*)s_params);
  free((void*)s_uri);
  free((void*)s_run_param);
  free((void*)s_prompt);
}

static tweak_id s_tweak_id = TWEAK_INVALID_ID;

static bool s_is_connected = false;

static void connection_status_changed(tweak_app_context context,
  bool is_connected, void *cookie)
{
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
  tweak_variant_string current_value_str = tweak_variant_to_json(&snapshot->current_value);
  tweak_variant_string default_value_str = tweak_variant_to_json(&snapshot->default_value);

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

static void execute_list_cmd(tweak_app_client_context context, char **tokens) {
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

static tweak_id get_tweak_id(tweak_app_client_context context, const char* uri) {
  if (uri) {
    const char *tweak_uri = uri;
    tweak_id result = tweak_app_find_id(context, tweak_uri);
    if (result == TWEAK_INVALID_ID) {
      fprintf(stderr, "ERROR: Unknown uri : %s.\n"
        "Make sure that specified uri exists. Use list command with a pattern.\n", tweak_uri);
    }
    return result;
  } else if (s_tweak_id != TWEAK_INVALID_ID) {
    return s_tweak_id;
  } else {
    fprintf(stderr, "ERROR: Tweak hasn't been selected.\n"
      "Either choose one with select command or privide tweak uri as an argument.\n");
    return TWEAK_INVALID_ID;
  }
}

static void execute_get_cmd(tweak_app_client_context context, char **tokens) {
  tweak_id tweak_id = get_tweak_id(context, tokens[0]);
  if (tweak_id == TWEAK_INVALID_ID)
    return;

  tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
  tweak_app_error_code error_code = tweak_app_item_clone_current_value(context, tweak_id, &value);
  if (error_code == TWEAK_APP_SUCCESS || error_code == TWEAK_APP_SUCCESS_LAST_KNOWN_VALUE) {
    tweak_variant_string string_repr = tweak_variant_to_json(&value);
    const char* format = (error_code == TWEAK_APP_SUCCESS_LAST_KNOWN_VALUE)
      ? "Last known value : %s\n"
      : "%s\n";
    printf(format, tweak_variant_string_c_str(&string_repr));
    tweak_variant_destroy_string(&string_repr);
  } else {
    fprintf(stderr, "ERROR: Backend error. Error code : %x.\n"
      "Make sure that there's no connectivity error.\n", error_code);
  }

  tweak_variant_destroy(&value);
}

static void execute_set_cmd(tweak_app_client_context context, char **tokens) {
  if (!tokens[0]) {
    fprintf(stderr, "ERROR: A value is required for 'set' command.\n"
      " Please provide a value.\n");
    return;
  }

  bool two_arguments = tokens[0] && tokens[1];
  const char* uri = two_arguments ? tokens[0] : NULL;
  const char* value_str = two_arguments ? tokens[1] : tokens[0];

  tweak_id tweak_id = get_tweak_id(context, uri);
  if (tweak_id == TWEAK_INVALID_ID)
    return;

  tweak_variant_type type = tweak_app_item_get_type(context, tweak_id);
  if (type != TWEAK_VARIANT_TYPE_IS_NULL) {
    tweak_variant value = TWEAK_VARIANT_INIT_EMPTY;
    tweak_variant_type_conversion_result conversion_result =
      tweak_variant_from_string(value_str, type, &value);
    if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS
        || conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED)
    {
      if (conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED) {
        tweak_variant_string new_value_string = tweak_variant_to_string(&value);
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
  } else {
    fprintf(stderr, "ERROR: Unknown uri: %s.\n"
      "Please make sure that such item does exist. Use list command with a filter\n", uri);
  }
}

static void execute_line(tweak_app_client_context context, char* line);

static char* expand_tilde(const char* arg) {
  char* result = NULL;
  wordexp_t exp_result = {};
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
        tweak_variant_string current_value_str = tweak_variant_to_string(&snapshot->current_value);
        tweak_variant_string default_value_str = tweak_variant_to_string(&snapshot->default_value);

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
  " - list [ pattern ]\n"
  " - load filename\n"
  " - save filename\n"
  " - select tweak_uri\n"
  " - get [ tweak_uri ]\n"
  " - set [ tweak_uri ] value\n"
  " - exit\n";

static const char s_help_help[] =
  "Prints help on specific command, or list of commands if its optional argument is omitted.\n";

static const char s_help_list[] =
  "This function is invoked by list command followed by optional pattern.\n"
  "It displays all items whose uris matches provided filter.\n"
  "User could provide POSIX regex prefixed by \"re\" argument, as in \"list re [ABC]/d\"\n"
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

static const char s_help_exit[] =
  "Exits from this repl. User might use Ctrl+C instead, of course.\n";

struct command_handler_pair command_handler_pairs[] = {
  { "list", &guess_tweak_uri, &execute_list_cmd, &s_help_list[0] },
  { "select", &guess_tweak_uri, &execute_select_cmd, &s_help_select[0] },
  { "get", &guess_tweak_uri, &execute_get_cmd, &s_help_get[0] },
  { "set", &guess_tweak_uri, &execute_set_cmd, &s_help_set[0] },
  { "load", &rl_filename_completion_function, &execute_load_cmd, &s_help_load[0] },
  { "save", &rl_filename_completion_function, &execute_save_cmd, &s_help_save[0] },
  { "help", &guess_command, &execute_help_cmd, &s_help_help[0] },
  { "?", &guess_command, &execute_help_cmd, &s_help_help[0] },
  { "exit", &guess_no_arg, &execute_exit_cmd, &s_help_exit[0] },
  { NULL, NULL, NULL, NULL }
};

static void execute_help_cmd(tweak_app_client_context context, char **tokens) {
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
  (void) context;
  (void) id;
  struct tweak_uri_list *tweak_uri_list = (struct tweak_uri_list *)cookie;
  pthread_mutex_lock(&tweak_uri_list->lock);
  tweak_app_cl_destroy_sorted_tweak_uris_list(tweak_uri_list->uris_list);
  tweak_uri_list->uris_list = NULL;
  pthread_mutex_unlock(&tweak_uri_list->lock);
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

  pthread_mutex_lock(&tweak_uri_list->lock);
  if (!tweak_uri_list->uris_list)
    tweak_uri_list->uris_list = tweak_app_cl_create_sorted_tweak_uris_list_strstr(tweak_uri_list->app_context, NULL);

  const char* match = tweak_app_cl_tweak_uris_list_pick_nth_match(tweak_uri_list->uris_list, str, n);
  if (match)
    result = strdup(match);

  pthread_mutex_unlock(&tweak_uri_list->lock);
  return result;
}

int main(int argc, char **argv) {
  tweak_common_set_log_level(TWEAK_LOG_LEVEL_FATAL);
  struct tweak_uri_list tweak_uri_list = {};
  pthread_mutex_init(&tweak_uri_list.lock, NULL);
  s_tweak_uri_list = &tweak_uri_list;

  int opt;
  atexit(&free_global_params);
  while ((opt = getopt(argc, argv, "t:p:u:")) != -1) {
    switch (opt) {
    case 't':
      s_connection_type = strdup(optarg);
      break;
    case 'p':
      s_params = strdup(optarg);
      break;
    case 'u':
      s_uri = strdup(optarg);
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

  pthread_mutex_lock(&tweak_uri_list.lock);
  tweak_app_client_context app_context = tweak_app_create_client_context(
    get_connection_type(), get_params(), get_uri(), &client_callbacks);
  tweak_uri_list.app_context = app_context;
  pthread_mutex_unlock(&tweak_uri_list.lock);

  int result = 0;
  if (app_context) {
    result = main_loop(app_context);
    tweak_app_destroy_context(app_context);
  } else {
    fprintf(stderr, "ERROR: Can't create context\n"
      "Please verify program arguments.\n");
    result = EXIT_FAILURE;
  }
  return result;
}
