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

#include <tweak2.h>
#include <tweak2/log.h>

#include <algorithm>
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
#include <sys/signalfd.h>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <vector>

namespace {

std::string connectionType = "nng";

std::string params = "role=server";

std::string uri = "tcp://0.0.0.0:7777/";

std::string logFileName = "-";

FILE* logFile = NULL;

uint32_t numItems = 10000;

constexpr uint32_t NUM_TYPES = 11;

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
struct ItemDesc {
  tweak_id id;
  int item_type;
};

std::vector<ItemDesc> immutableItems;

std::vector<ItemDesc> mutableItems;

void createItems();

void valueChangeRoutine();

volatile bool shouldExit = false;

int runServer() {
  if (logFileName != "-") {
    logFile = fopen(logFileName.c_str(), "w+");
    if (!logFile) {
      printf("Can't open file: %s\n", logFileName.c_str());
    }
    tweak_common_set_output_file(logFile);
  }
  tweak_initialize_library(connectionType.c_str(), params.c_str(), uri.c_str());
  createItems();
  std::thread valueChangeRoutineThread(&valueChangeRoutine);

  sigset_t mask;
  int sfd;
  struct signalfd_siginfo fdsi;

  sigemptyset(&mask);
  sigaddset(&mask, SIGHUP);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  sigaddset(&mask, SIGTERM);

  sfd = signalfd(-1, &mask, 0);
  if (sfd == -1) {
    return EXIT_FAILURE;
  }

  ssize_t s = read(sfd, &fdsi, sizeof(fdsi));
  if (s == sizeof(fdsi)) {
    switch(fdsi.ssi_signo) {
    case SIGHUP:
      printf("Got SIGHUP\n");
      break;
    case SIGINT:
      printf("Got SIGINT\n");
      break;
    case SIGQUIT:
      printf("Got SIGQUIT\n");
      break;
    case SIGTERM:
      printf("Got SIGTERM\n");
      break;
    default:
      printf("Read unexpected signal :%d\n", fdsi.ssi_signo);
      break;
    }
  } else {
    printf("Invalid number of bytes being read\n");
  }
  shouldExit = true;
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
  static std::string minValue();
  static std::string maxValue();
  static std::string generateMeta(const std::string &uri);
  static tweak_id generateTweak(const std::string &uri);
};

template<> std::string TweakFactory<bool>::controlType() {
  return "checkbox";
}

template<> std::string TweakFactory<bool>::minValue() {
  return "false";
}

template<> std::string TweakFactory<bool>::maxValue() {
  return "true";
}

template<typename T> std::string TweakFactory<T>::controlType() {
  if (std::is_integral<T>::value) {
    return "spinbox";
  } else {
    return "slider";
  }
}

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

template<typename T> std::string TweakFactory<T>::generateMeta(const std::string &uri) {
  std::ostringstream oss;
  oss << "{" << "\"control\":\"" << controlType() << "\", "
      << "\"min\":" << minValue() << ", \"max\":" << maxValue() << ", "
      << "\"readonly\":" << (isInputItem(uri) ? "false" : "true") << "}";
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

ItemDesc createRandomItem(const std::string &uri) {
  ItemDesc result = {};
  result.item_type = rand() % NUM_TYPES;
  switch (result.item_type) {
  case 0:
    result.id = TweakFactory<bool>::generateTweak(uri);
    break;
  case 1:
    result.id = TweakFactory<int8_t>::generateTweak(uri);
    break;
  case 2:
    result.id = TweakFactory<int16_t>::generateTweak(uri);
    break;
  case 3:
    result.id = TweakFactory<int32_t>::generateTweak(uri);
    break;
  case 4:
    result.id = TweakFactory<int64_t>::generateTweak(uri);
    break;
  case 5:
    result.id = TweakFactory<uint8_t>::generateTweak(uri);
    break;
  case 6:
    result.id = TweakFactory<uint16_t>::generateTweak(uri);
    break;
  case 7:
    result.id = TweakFactory<uint32_t>::generateTweak(uri);
    break;
  case 8:
    result.id = TweakFactory<uint64_t>::generateTweak(uri);
    break;
  case 9:
    result.id = TweakFactory<float>::generateTweak(uri);
    break;
  case 10:
    result.id = TweakFactory<double>::generateTweak(uri);
    break;
  default:
    abort();
    break;
  }
  return result;
}

void createItems() {
  const size_t max_branch_length = (size_t)log10(numItems);
  uint32_t num_folders = ((uint32_t)log10(numItems)) * 10;
  std::vector<std::string> folders;
  for (uint32_t ix = 0; ix < num_folders; ix++) {
    folders.push_back(generateBranch(rand() % max_branch_length + 1));
  }
  for (uint32_t ix = 0; ix < numItems; ix++) {
    std::string uri = generateItemUri(folders);
    if (isInputItem(uri))
    {
      auto item = createRandomItem(uri);
      if (item.id != TWEAK_INVALID_ID) {
        immutableItems.push_back(item);
      }
    } else {
      auto item = createRandomItem(uri);
      if (item.id != TWEAK_INVALID_ID) {
        mutableItems.push_back(item);
      }
    }
  }

  ItemDesc testItem; /* Needed for command line client test */
  std::string meta = TweakFactory<int32_t>::generateMeta("/test/test");
  testItem.id = tweak_add_scalar_int32("/test/test", "permanent test value", meta.c_str(), 42);
  testItem.item_type = 7;
  immutableItems.push_back(testItem);
}

void randomUpdateItem(const ItemDesc& ItemDesc) {
  switch (ItemDesc.item_type) {
  case 0:
    tweak_set_scalar_bool(ItemDesc.id, (rand() % 2) ? true : false);
    break;
  case 1:
    tweak_set_scalar_int8(ItemDesc.id, (int8_t)(rand() & 0xff));
    break;
  case 2:
    tweak_set_scalar_int16(ItemDesc.id, (int16_t)(rand() & 0xffff));
    break;
  case 3:
    tweak_set_scalar_int32(ItemDesc.id, (int32_t)rand());
    break;
  case 4:
    tweak_set_scalar_int64(ItemDesc.id, ((int64_t)rand() << 32) | rand());
    break;
  case 5:
    tweak_set_scalar_uint8(ItemDesc.id, (uint8_t)(rand() & 0xff));
    break;
  case 6:
    tweak_set_scalar_uint16(ItemDesc.id, (uint16_t)(rand() & 0xffff));
    break;
  case 7:
    tweak_set_scalar_uint32(ItemDesc.id, (uint32_t)rand());
    break;
  case 8:
    tweak_set_scalar_uint64(ItemDesc.id, ((uint64_t)rand() << 32) | rand());
    break;
  case 9:
    tweak_set_scalar_float(ItemDesc.id, rand() * 1.f / (float)RAND_MAX + 1.f);
    break;
  case 10:
    tweak_set_scalar_double(ItemDesc.id, rand() * 1. / (double)RAND_MAX + 1.);
    break;
  default:
    abort();
    break;
  }
}

void valueChangeRoutine() {
  size_t sample_size = mutableItems.size() / 10;
  std::vector<ItemDesc> sample = mutableItems;
  std::random_device rd;
  std::mt19937 g(rd());
  while (!shouldExit) {
    std::shuffle(sample.begin(), sample.end(), g);
    for (size_t ix = 0; ix < sample_size; ++ix) {
      randomUpdateItem(sample[ix]);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_DELAY_MILLIS));
  }
}

}

int main(int argc, char** argv) {
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
