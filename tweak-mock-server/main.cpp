/**
 * @file main.cpp
 * @ingroup tweak-api
 *
 * @brief test suite for implementation of simplified server side tweak2 public API.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

#include <tweak2/tweak2.h>
#include <tweak2/log.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <random>
#include <signal.h>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <vector>
#include <functional>
#include <unordered_set>

namespace {

std::string connectionType = "nng";

std::string params = "role=server";

std::string uri = "tcp://0.0.0.0:7777/";

std::string logFileName = "-";

FILE* logFile = NULL;

uint32_t numItems = 10000;

constexpr uint32_t UPDATE_DELAY_MILLIS = 33;

const std::vector<std::string> alphabetCodes {
  "Alfa",
  "Bravo",
  "Charlie",
  "Delta",
  "Echo",
  "Foxtrot",
  "Golf",
  "Hotel",
  "India",
  "Juliett",
  "Kilo",
  "Lima",
  "Mike",
  "November",
  "Oscar",
  "Papa",
  "Quebec",
  "Romeo",
  "Sierra",
  "Tango",
  "Uniform",
  "Victor",
  "Whiskey",
  "X-ray",
  "Yankee",
  "Zulu",
};
std::vector<std::function<void()>> createItems();

void valueChangeRoutine(const std::vector<std::function<void()>> &mutators);

volatile bool shouldExit = false;

void handle_signal(int signum) {
  shouldExit = true;
  signal(signum, NULL);
}

static void output_proc(const char* string) {
  fputs(string, logFile);
  fputc('\n', logFile);
}

int runServer() {
  if (logFileName != "-") {
    logFile = fopen(logFileName.c_str(), "w+");
    if (!logFile) {
      printf("Can't open file: %s\n", logFileName.c_str());
    }
    tweak_common_set_custom_handler(&output_proc);
  }
  tweak_initialize_library(connectionType.c_str(), params.c_str(), uri.c_str());
  std::vector<std::function<void()>> mutators = createItems();
  std::thread valueChangeRoutineThread([&mutators]() -> void {
    valueChangeRoutine(mutators);
  });
  valueChangeRoutineThread.join();
  tweak_finalize_library();
  return 0;
}

std::string generateRandomUri(uint32_t min, uint32_t max) {
  std::string uri;
  uint32_t length = min + rand() % (max - min);
  uint32_t char_range = ('9' - '0') + ('Z' - 'A');
  for (uint32_t ix = 0; ix < length; ++ix) {
    uint32_t nchar = rand() % char_range;
    char c;
    if (nchar < ('9' - '0')) {
      c = (char)('0' +  nchar);
    } else {
      c = (char)('A' +  (nchar - ('9' - '0')));
    }
    uri.push_back(c);
  }
  uri[length] = '\0';
  return uri;
}

std::string generateSegment() {
  return alphabetCodes[rand() % alphabetCodes.size()] + std::string("_") + generateRandomUri(2, 8);
}

std::string generateBranch(uint32_t max_branch_length){
  std::ostringstream oss;
  for (size_t ix = 0; ix < max_branch_length; ++ix) {
    oss << "/" << generateSegment();
  }
  return oss.str();
}

std::string generateItemUri(const std::vector<std::string> &folders) {
  const std::string &branch = folders[rand() % folders.size()];
  return branch.substr(0, branch.find_last_of('/') + 1) + generateSegment();
}

bool isInputItem(const std::string& uri) {
  return uri.find("/Alpha") == 0 || uri.find("/Bravo") == 0 || uri.find("/Charlie") == 0;
}

template <typename T> struct TweakFactory {
  static std::string controlType();
  static std::string type();
  static std::string minValue();
  static std::string maxValue();
  static std::string generateMeta(const std::string &uri);
  static tweak_id generateTweak(const std::string &uri);
};

template<typename T> std::string TweakFactory<T>::minValue() {
  if (std::is_integral<T>::value) {
    return std::to_string(std::numeric_limits<T>::min());
  } else if (std::is_floating_point<T>::value) {
    return "-1";
  } else {
    return "undefined";
  }
}

template<typename T> std::string TweakFactory<T>::maxValue() {
  if (std::is_integral<T>::value) {
    return std::to_string(std::numeric_limits<T>::max());
  } else if (std::is_floating_point<T>::value) {
    return "1";
  } else {
    return "undefined";
  }
}

template<> std::string TweakFactory<bool>::generateMeta(const std::string &uri){
  std::ostringstream oss;
  oss << "{\"readonly\":" << (isInputItem(uri) ? "false" : "true") << "}";
  return oss.str();
}

template<typename T> std::string TweakFactory<T>::generateMeta(const std::string &uri) {
  std::ostringstream oss;
  oss << "{\"min\":" << minValue() << ",\"max\":" << maxValue()
      << ",\"readonly\":" << (isInputItem(uri) ? "false" : "true") << "}";
  return oss.str();
}

template<> tweak_id TweakFactory<bool>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_bool(uri.c_str(), "bool tweak", generateMeta(uri).c_str(), false);
}

template<> tweak_id TweakFactory<int8_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_int8(uri.c_str(), "int8 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<int16_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_int16(uri.c_str(), "int16 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<int32_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_int32(uri.c_str(), "int32 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<int64_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_int64(uri.c_str(), "int64 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<uint8_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_uint8(uri.c_str(), "uint8 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<uint16_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_uint16(uri.c_str(), "uint16 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<uint32_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_uint32(uri.c_str(), "uint32 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<uint64_t>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_uint64(uri.c_str(), "uint64 tweak", generateMeta(uri).c_str(), 0);
}

template<> tweak_id TweakFactory<float>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_float(uri.c_str(), "float tweak", generateMeta(uri).c_str(), 0.f);
}

template<> tweak_id TweakFactory<double>::generateTweak(const std::string &uri) {
  return tweak_add_scalar_double(uri.c_str(), "double tweak", generateMeta(uri).c_str(), 0.);
}

static constexpr tweak_variant_type tweak_variant_types[] = {
  TWEAK_VARIANT_TYPE_BOOL,
  TWEAK_VARIANT_TYPE_SINT8,
  TWEAK_VARIANT_TYPE_SINT16,
  TWEAK_VARIANT_TYPE_SINT32,
  TWEAK_VARIANT_TYPE_SINT64,
  TWEAK_VARIANT_TYPE_UINT8,
  TWEAK_VARIANT_TYPE_UINT16,
  TWEAK_VARIANT_TYPE_UINT32,
  TWEAK_VARIANT_TYPE_UINT64,
  TWEAK_VARIANT_TYPE_FLOAT,
  TWEAK_VARIANT_TYPE_DOUBLE
};

std::function<void()> createRandomItem(const std::string &uri) {
  tweak_variant_type numType =
    tweak_variant_types[rand() % (sizeof(tweak_variant_types) / sizeof(tweak_variant_types[0]))];
  tweak_id tweak_id;
  switch (numType) {
  case TWEAK_VARIANT_TYPE_BOOL:
    tweak_id = TweakFactory<bool>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_bool(tweak_id, (rand() % 2) ? true : false);
    };
  case TWEAK_VARIANT_TYPE_SINT8:
    tweak_id = TweakFactory<int8_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_int8(tweak_id, (int8_t)(rand() & 0xff));
    };
  case TWEAK_VARIANT_TYPE_SINT16:
    tweak_id = TweakFactory<int16_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_int16(tweak_id, (int16_t)(rand() & 0xffff));
    };
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    tweak_id = TweakFactory<int32_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_int32(tweak_id, (int32_t)rand());
    };
  case TWEAK_VARIANT_TYPE_SINT64:
    tweak_id = TweakFactory<int64_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_int64(tweak_id, ((int64_t)rand() << 32) | rand());
    };
  case TWEAK_VARIANT_TYPE_UINT8:
    tweak_id = TweakFactory<uint8_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint8(tweak_id, (uint8_t)(rand() & 0xff));
    };
  case TWEAK_VARIANT_TYPE_UINT16:
    tweak_id = TweakFactory<uint16_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint16(tweak_id, (uint16_t)(rand() & 0xffff));
    };
  case TWEAK_VARIANT_TYPE_UINT32:
    tweak_id = TweakFactory<uint32_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint32(tweak_id, (uint32_t)rand());
    };
  case TWEAK_VARIANT_TYPE_UINT64:
    tweak_id = TweakFactory<uint64_t>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint64(tweak_id, ((uint64_t)rand() << 32) | rand());
    };
  case TWEAK_VARIANT_TYPE_FLOAT:
    tweak_id = TweakFactory<float>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_float(tweak_id, rand() * 1.f / (float)RAND_MAX + 1.f);
    };
  case TWEAK_VARIANT_TYPE_DOUBLE:
    tweak_id = TweakFactory<double>::generateTweak(uri);
    return [tweak_id]() -> void {
      tweak_set_scalar_double(tweak_id, rand() * 1. / (double)RAND_MAX + 1.);
    };
  default:
    abort();
  }
  return std::function<void()>();
}

void button_handler(tweak_id tweak_id, void* cookie) {
  (void) cookie;
  if (tweak_get_scalar_bool(tweak_id)) {
    fprintf(stderr, "Button pressed\n");
  }
}

std::vector<std::function<void()>> createItems() {
  std::vector<std::function<void()>> result;
  const size_t max_branch_length = (size_t)log10(numItems);
  uint32_t num_folders = ((uint32_t)log10(numItems)) * 10;
  std::vector<std::string> folders;
  for (uint32_t ix = 0; ix < num_folders; ix++) {
    folders.push_back(generateBranch(rand() % max_branch_length + 1));
  }
  std::unordered_set<std::string> existing_items;
  for (uint32_t ix = 0; ix < numItems; ix++) {
    std::string uri;
    do {
      uri = generateItemUri(folders);
    } while(existing_items.find(uri) != existing_items.end());
    existing_items.insert(uri);
    if (isInputItem(uri)) {
      createRandomItem(uri);
    } else {
      result.push_back(createRandomItem(uri));
    }
  }

  std::string meta = TweakFactory<int32_t>::generateMeta("/test/test");
  tweak_add_scalar_int32("/test/test", "permanent test value", meta.c_str(), 42);

  tweak_add_scalar_int64("/test/test1", "permanent test value",
     "{\"readonly\":false,\"type\":\"uint64_t\",\"min\":1.17549e-38,\"max\":3.40282e+38}", 0
  );

  tweak_add_scalar_int32("/test/test2", "permanent test value",
    "{\"options\":[{\"text\": \"a\", \"value\": -1}, \"b\", \"c\"]}", 0);

  struct tweak_add_item_ex_desc desc = {};
  desc.uri = "/test/test3";
  desc.description = "permanent test value";
  desc.item_change_listener = &button_handler;
  desc.meta = "{\"control\": \"button\", \"caption\": \"Push Me!\"}";
  desc.cookie = NULL;
  tweak_add_scalar_bool_ex(&desc, false);

  tweak_add_scalar_float("/test/test3", "permanent test value",
    "{\"readonly\":false, \"min\":-300.0, \"max\":3000.0}", 0.);

  tweak_add_scalar_bool("/test/jira_153_1", "permanent test value", NULL, false);

  tweak_add_scalar_bool("/test/jira_153_2", "permanent test value",
    "{\"options\": [{\"value\": false, \"text\": \"On\"}, {\"value\": true, \"text\": \"Off\"}]}",
    false);

  tweak_add_scalar_int32("/test/tweak_jira_153_4", "permanent test value",
    "{\"min\":-100,\"max\":100}", false);

  tweak_add_scalar_int32("/test/tweak_jira_153_5", "permanent test value",
    "{\"min\":-100, \"max\":100, \"step\": 10}", false);

  tweak_add_scalar_int32("/test/tweak_jira_153_6", "permanent test value",
    "{\"max\":100, \"step\": 100}", false);

  return result;
}

void valueChangeRoutine(const std::vector<std::function<void()>> &mutators) {
  std::vector<size_t> sample;
  sample.resize(mutators.size());
  for (size_t ix = 0; ix < sample.size(); ++ix) {
    sample[ix] = ix;
  }
  size_t sample_size = mutators.size() / 10;
  std::random_device rd;
  std::mt19937 g(rd());
  while (!shouldExit) {
    std::shuffle(sample.begin(), sample.end(), g);
    for (size_t ix = 0; ix < sample_size; ++ix) {
      mutators[sample[ix]]();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_DELAY_MILLIS));
  }
}

}

int main(int argc, char** argv) {
  signal(SIGTERM, &handle_signal);
  signal(SIGINT, &handle_signal);
  signal(SIGQUIT, &handle_signal);
  signal(SIGHUP, &handle_signal);
  uint32_t seed = 31337;
  int opt;
  while ((opt = getopt(argc, argv, "t:p:u:N:S:L:")) != -1) {
    switch (opt) {
    case 't':
      connectionType = optarg;
      break;
    case 'p':
      params = optarg;
      break;
    case 'u':
      uri = optarg;
      break;
    case 'N':
      numItems = std::stoul(optarg);
      break;
    case 'S':
      seed = std::stoul(optarg);
      break;
    case 'L':
      logFileName = optarg;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-s] [-t connection type] [-p params] [-u uri] [-N numItems]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  srand(seed);
  return runServer();
}
