/**
 * @file main.cpp
 * @ingroup tweak-api
 *
 * @brief test suite for implementation of simplified server side tweak2 public API.
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

#include <tweak2/tweak2.h>
#include <tweak2/log.h>
#include <tweak2/defaults.h>

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
#include <vector>
#include <functional>
#include <unordered_set>
#include <typeinfo>

#if defined(_MSC_BUILD)
#include <getopt.h>
#else
#include <unistd.h>
#endif

namespace {

std::string connectionType = "nng";

std::string params = "role=server";

std::string uri = TWEAK_DEFAULT_ENDPOINT;

std::string logFileName = "-";

FILE* logFile = NULL;

uint32_t numItems = 10000;

constexpr uint32_t UPDATE_DELAY_MILLIS = 33;

constexpr size_t MAX_ARRAY_SIZE = 100;

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
    logFile = fopen(logFileName.c_str(), "wa+");
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
  std::string result;
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
    result.push_back(c);
  }
  result[length] = '\0';
  return result;
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

bool isInputItem(const std::string &arg) {
  return arg.find("/Alpha") == 0 || arg.find("/Bravo") == 0 ||
         arg.find("/Charlie") == 0;
}

template <typename T> struct TweakFactory {
  static std::string controlType();
  static std::string type();
  static std::string minValue();
  static std::string maxValue();
  static std::string generateMeta(const std::string &uri);
  static std::string generateVectorMeta(const std::string &uri, std::size_t array_size);
  static tweak_id generateTweak(const std::string &uri);
  static tweak_id generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc, const T* array, std::size_t array_size);
  static tweak_id generateVectorTweak(const std::string &uri, const T* array, std::size_t array_size);
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

template<> std::string TweakFactory<bool>::generateMeta(const std::string &arg){
  std::ostringstream oss;
  oss << "{\"readonly\":" << (isInputItem(arg) ? "false" : "true") << "}";
  return oss.str();
}

template<typename T> std::string TweakFactory<T>::generateMeta(const std::string &arg) {
  (void)arg;
  std::ostringstream oss;
  oss << "{\"min\":" << minValue() << ", \"max\":" << maxValue()
      << ", \"readonly\":" << (isInputItem(uri) ? "false" : "true") << "}";
  return oss.str();
}

template<typename T>
std::string TweakFactory<T>::generateVectorMeta(const std::string &uri0, std::size_t array_size) {
  std::ostringstream oss;
  oss << "{\"min\":" << minValue()
       << ", \"max\":" << maxValue()
       << ", \"readonly\":" << (isInputItem(uri0) ? "false" : "true")
       << ", \"layout\": "
         << "{"
               << "\"order\": \"row-major\""
               << ", \"dimensions\": [" << array_size << "]"
         << "}}";
  return oss.str();
}

template <>
tweak_id TweakFactory<bool>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_bool(arg.c_str(), "bool tweak",
      generateMeta(arg).c_str(), false);
}

template <>
tweak_id TweakFactory<int8_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_int8(arg.c_str(), "int8 tweak",
      generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<int16_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_int16(arg.c_str(), "int16 tweak",
                                generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<int32_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_int32(arg.c_str(), "int32 tweak",
                                generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<int64_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_int64(arg.c_str(), "int64 tweak",
                                generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<uint8_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_uint8(arg.c_str(), "uint8 tweak",
                                generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<uint16_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_uint16(arg.c_str(), "uint16 tweak",
                                 generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<uint32_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_uint32(arg.c_str(), "uint32 tweak",
                                 generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<uint64_t>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_uint64(arg.c_str(), "uint64 tweak",
                                 generateMeta(arg).c_str(), 0);
}

template <>
tweak_id TweakFactory<float>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_float(arg.c_str(), "float tweak",
                                generateMeta(arg).c_str(), 0.f);
}

template <>
tweak_id TweakFactory<double>::generateTweak(const std::string &arg) {
  return tweak_add_scalar_double(arg.c_str(), "double tweak",
                                 generateMeta(arg).c_str(), 0.);
}

template<typename T> tweak_id TweakFactory<T>::generateVectorTweak(const std::string &uri0,
  const T* array, std::size_t array_size)
{
  tweak_add_item_ex_desc desc;
  memset(&desc, 0, sizeof(desc));
  std::string meta = generateVectorMeta(uri0, array_size);
  desc.uri = uri0.c_str();
  desc.meta = meta.c_str();
  std::string tmp = std::string(typeid(T).name()) + std::string(" vector tweak");
  desc.description = tmp.c_str();
  return generateVectorTweakCommon(&desc, array, array_size);
}

template<> tweak_id TweakFactory<int8_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const int8_t* array, std::size_t array_size)
{
  return tweak_create_vector_sint8(desc, array, array_size);
}

template<> tweak_id TweakFactory<int16_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const int16_t* array, std::size_t array_size)
{
  return tweak_create_vector_sint16(desc, array, array_size);
}

template<> tweak_id TweakFactory<int32_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const int32_t* array, std::size_t array_size)
{
  return tweak_create_vector_sint32(desc, array, array_size);
}

template<> tweak_id TweakFactory<int64_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const int64_t* array, std::size_t array_size)
{
  return tweak_create_vector_sint64(desc, array, array_size);
}

template<> tweak_id TweakFactory<uint8_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const uint8_t* array, std::size_t array_size)
{
  return tweak_create_vector_uint8(desc, array, array_size);
}

template<> tweak_id TweakFactory<uint16_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const uint16_t* array, std::size_t array_size)
{
  return tweak_create_vector_uint16(desc, array, array_size);
}

template<> tweak_id TweakFactory<uint32_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const uint32_t* array, std::size_t array_size)
{
  return tweak_create_vector_uint32(desc, array, array_size);
}

template<> tweak_id TweakFactory<uint64_t>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const uint64_t* array, std::size_t array_size)
{
  return tweak_create_vector_uint64(desc, array, array_size);
}

template<> tweak_id TweakFactory<float>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const float* array, std::size_t array_size)
{
  return tweak_create_vector_float(desc, array, array_size);
}

template<> tweak_id TweakFactory<double>::generateVectorTweakCommon(struct tweak_add_item_ex_desc* desc,
  const double* array, std::size_t array_size)
{
  return tweak_create_vector_double(desc, array, array_size);
}

template <bool U> struct ChooseDefaultIntegerDistribType;

template <> struct ChooseDefaultIntegerDistribType<true> {
   using Type = uint32_t;
};

template <> struct ChooseDefaultIntegerDistribType<false> {
   using Type = int32_t;
};

template <class T> struct ChooseIntegerDistribType {
   using Type = typename ChooseDefaultIntegerDistribType<std::is_unsigned<T>::value>::Type;
};

template <> struct ChooseIntegerDistribType<uint64_t> {
   using Type = uint64_t;
};

template <> struct ChooseIntegerDistribType<int64_t> {
   using Type = int64_t;
};

template <typename Q, bool F> struct FillRandom0 { };

template <typename Q> struct FillRandom0<Q, false> {
  template<typename I>
    static void fillRandom(I b, I e)
    {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<typename ChooseIntegerDistribType<Q>::Type> distrib(
          std::numeric_limits<Q>::min(), std::numeric_limits<Q>::max());

      for (I itr = b; itr != e; ++itr) {
        *itr = static_cast<Q>(distrib(gen));
      }
    }
};

template <typename Q> struct FillRandom0<Q, true> {
  template<typename I>
    static void fillRandom(I b, I e)
    {
      std::default_random_engine generator;
      std::uniform_real_distribution<Q> distribution(-1., 1.);

      for (I itr = b; itr != e; ++itr) {
        *itr = distribution(generator);
      }
    }
};

template <typename I>
void fillRandomSeq(I b, I e) {
  using Q = typename std::iterator_traits<I>::value_type;
  return FillRandom0<Q, std::is_floating_point<Q>::value>::fillRandom(b, e);
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
  TWEAK_VARIANT_TYPE_DOUBLE,
  TWEAK_VARIANT_TYPE_VECTOR_SINT8,
  TWEAK_VARIANT_TYPE_VECTOR_SINT16,
  TWEAK_VARIANT_TYPE_VECTOR_SINT32,
  TWEAK_VARIANT_TYPE_VECTOR_SINT64,
  TWEAK_VARIANT_TYPE_VECTOR_UINT8,
  TWEAK_VARIANT_TYPE_VECTOR_UINT16,
  TWEAK_VARIANT_TYPE_VECTOR_UINT32,
  TWEAK_VARIANT_TYPE_VECTOR_UINT64,
  TWEAK_VARIANT_TYPE_VECTOR_FLOAT,
  TWEAK_VARIANT_TYPE_VECTOR_DOUBLE
};

template <typename Q>
struct UpdateVectorHelper {
  static void set_vector(tweak_id id, const Q* value);
};

template <>
struct UpdateVectorHelper<int8_t> {
  static void set_vector(tweak_id id, const int8_t* value) {
    tweak_set_vector_sint8(id, value);
  }
};

template <>
struct UpdateVectorHelper<int16_t> {
  static void set_vector(tweak_id id, const int16_t* value) {
    tweak_set_vector_sint16(id, value);
  }
};

template <>
struct UpdateVectorHelper<int32_t> {
  static void set_vector(tweak_id id, const int32_t* value) {
    tweak_set_vector_sint32(id, value);
  }
};

template <>
struct UpdateVectorHelper<int64_t> {
  static void set_vector(tweak_id id, const int64_t* value) {
    tweak_set_vector_sint64(id, value);
  }
};

template <>
struct UpdateVectorHelper<uint8_t> {
  static void set_vector(tweak_id id, const uint8_t* value) {
    tweak_set_vector_uint8(id, value);
  }
};

template <>
struct UpdateVectorHelper<uint16_t> {
  static void set_vector(tweak_id id, const uint16_t* value) {
    tweak_set_vector_uint16(id, value);
  }
};

template <>
struct UpdateVectorHelper<uint32_t> {
  static void set_vector(tweak_id id, const uint32_t* value) {
    tweak_set_vector_uint32(id, value);
  }
};

template <>
struct UpdateVectorHelper<uint64_t> {
  static void set_vector(tweak_id id, const uint64_t* value) {
    tweak_set_vector_uint64(id, value);
  }
};

template <>
struct UpdateVectorHelper<float> {
  static void set_vector(tweak_id id, const float* value) {
    tweak_set_vector_float(id, value);
  }
};

template <>
struct UpdateVectorHelper<double> {
  static void set_vector(tweak_id id, const double* value) {
    tweak_set_vector_double(id, value);
  }
};

template<typename Q>
std::function<void()> createRandomVectorItem(const std::string &uri) {
  std::size_t array_size = std::max((size_t)10, rand() % MAX_ARRAY_SIZE);
  std::vector<Q> array(array_size);
  fillRandomSeq(array.begin(), array.end());
  tweak_id tweak_id = TweakFactory<Q>::generateVectorTweak(uri, &array[0], array_size);
  return [array_size, tweak_id]() -> void {
    std::vector<Q> array(array_size);
    fillRandomSeq(array.begin(), array.end());
    UpdateVectorHelper<Q>::set_vector(tweak_id, &array[0]);
  };
}

std::function<void()> createRandomItem(const std::string &arg) {
  tweak_variant_type numType =
    tweak_variant_types[rand() % (sizeof(tweak_variant_types) / sizeof(tweak_variant_types[0]))];
  tweak_id tweak_id;
  switch (numType) {
  case TWEAK_VARIANT_TYPE_BOOL:
    tweak_id = TweakFactory<bool>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_bool(tweak_id, (rand() % 2) ? true : false);
    };
  case TWEAK_VARIANT_TYPE_SINT8:
    tweak_id = TweakFactory<int8_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_int8(tweak_id, (int8_t)(rand() & 0xff));
    };
  case TWEAK_VARIANT_TYPE_SINT16:
    tweak_id = TweakFactory<int16_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_int16(tweak_id, (int16_t)(rand() & 0xffff));
    };
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    tweak_id = TweakFactory<int32_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_int32(tweak_id, (int32_t)rand());
    };
  case TWEAK_VARIANT_TYPE_SINT64:
    tweak_id = TweakFactory<int64_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_int64(tweak_id, ((int64_t)rand() << 32) | rand());
    };
  case TWEAK_VARIANT_TYPE_UINT8:
    tweak_id = TweakFactory<uint8_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint8(tweak_id, (uint8_t)(rand() & 0xff));
    };
  case TWEAK_VARIANT_TYPE_UINT16:
    tweak_id = TweakFactory<uint16_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint16(tweak_id, (uint16_t)(rand() & 0xffff));
    };
  case TWEAK_VARIANT_TYPE_UINT32:
    tweak_id = TweakFactory<uint32_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint32(tweak_id, (uint32_t)rand());
    };
  case TWEAK_VARIANT_TYPE_UINT64:
    tweak_id = TweakFactory<uint64_t>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_uint64(tweak_id, ((uint64_t)rand() << 32) | rand());
    };
  case TWEAK_VARIANT_TYPE_FLOAT:
    tweak_id = TweakFactory<float>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_float(tweak_id, rand() * 1.f / (float)RAND_MAX + 1.f);
    };
  case TWEAK_VARIANT_TYPE_DOUBLE:
    tweak_id = TweakFactory<double>::generateTweak(arg);
    return [tweak_id]() -> void {
      tweak_set_scalar_double(tweak_id, rand() * 1. / (double)RAND_MAX + 1.);
    };
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return createRandomVectorItem<int8_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return createRandomVectorItem<int16_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return createRandomVectorItem<int32_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return createRandomVectorItem<int64_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return createRandomVectorItem<uint8_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return createRandomVectorItem<uint16_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return createRandomVectorItem<uint32_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return createRandomVectorItem<uint64_t>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return createRandomVectorItem<float>(arg);
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return createRandomVectorItem<double>(arg);
  default:
    break;
  }
  abort();
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
    std::string uri0;
    do {
      uri0 = generateItemUri(folders);
    } while(existing_items.find(uri0) != existing_items.end());
    existing_items.insert(uri0);
    if (isInputItem(uri0)) {
      createRandomItem(uri0);
    } else {
      result.push_back(createRandomItem(uri0));
    }
  }

  std::string meta = TweakFactory<int32_t>::generateMeta("/test/test");
  tweak_add_scalar_int32("/test/test", "permanent test value", meta.c_str(), 42);

  std::string meta1 = TweakFactory<int32_t>::generateMeta("/test/test1");
  tweak_id counter1 = tweak_add_scalar_int32("/test/test1", "permanent test value", meta1.c_str(), 0);
  result.push_back([counter1]() -> void {
    int32_t c = tweak_get_scalar_int32(counter1);
    ++c;
    tweak_set_scalar_int32(counter1, c);
  });

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

  {
    std::ostringstream oss;
    oss << "{\"min\":" << -1.f
        << ", \"max\":" << 1.f
        << ", \"readonly\": true"
        << ", \"layout\": "
          << "{"
               << "\"order\": \"row-major\""
               << ", \"dimensions\": [4, 4]"
          << "}}";
    tweak_add_item_ex_desc desc0;
    memset(&desc0, 0, sizeof(desc0));
    desc0.uri = "/test/matrix";
    std::string tmp = oss.str();
    desc0.meta = tmp.c_str();
    desc0.description = "float vector tweak";
    tweak_create_vector_float(&desc0, NULL, 16);
  }

  tweak_add_scalar_int32("/zzzzzzzzzzzzzzzzzzzztest/zzzzzzzzzzzzzzzzzzzz", "terminator for test",
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
