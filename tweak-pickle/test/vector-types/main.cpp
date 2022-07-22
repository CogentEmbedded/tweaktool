/**
 * @file main.c
 * @ingroup tweak-internal
 *
 * @brief test suite for tweak2::pickle library.
 *
 * @copyright 2021-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#include <tweak2/log.h>
#include <tweak2/defaults.h>
#include <tweak2/pickle_client.h>
#include <tweak2/pickle_server.h>

#include <acutest.h>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <type_traits>

namespace {

template<bool U>
struct ChooseDefaultIntegerDistribType;

template<>
struct ChooseDefaultIntegerDistribType<true> {
    using Type = uint32_t;
};

template<>
struct ChooseDefaultIntegerDistribType<false> {
    using Type = int32_t;
};

template<class T>
struct ChooseIntegerDistribType {
    using Type = typename ChooseDefaultIntegerDistribType< std::is_unsigned<T>::value >::Type;
};

template<>
struct ChooseIntegerDistribType<uint64_t> {
    using Type = uint64_t;
};

template<>
struct ChooseIntegerDistribType<int64_t> {
    using Type = int64_t;
};

template<typename Q, bool F>
struct VectorGen0
{ };

template<typename Q>
struct Dispatcher{ };

template<>
struct Dispatcher<uint8_t> {
  static void assign_vector(tweak_variant* arg, const uint8_t* arr, size_t size) {
    return tweak_variant_assign_uint8_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<uint16_t> {
  static void assign_vector(tweak_variant* arg, const uint16_t* arr, size_t size) {
    return tweak_variant_assign_uint16_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<uint32_t> {
  static void assign_vector(tweak_variant* arg, const uint32_t* arr, size_t size) {
    return tweak_variant_assign_uint32_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<uint64_t> {
  static void assign_vector(tweak_variant* arg, const uint64_t* arr, size_t size) {
    return tweak_variant_assign_uint64_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<int8_t> {
  static void assign_vector(tweak_variant* arg, const int8_t* arr, size_t size) {
    return tweak_variant_assign_sint8_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<int16_t> {
  static void assign_vector(tweak_variant* arg, const int16_t* arr, size_t size) {
    return tweak_variant_assign_sint16_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<int32_t> {
  static void assign_vector(tweak_variant* arg, const int32_t* arr, size_t size) {
    return tweak_variant_assign_sint32_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<int64_t> {
  static void assign_vector(tweak_variant* arg, const int64_t* arr, size_t size) {
    return tweak_variant_assign_sint64_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<float> {
  static void assign_vector(tweak_variant* arg, const float* arr, size_t size) {
    return tweak_variant_assign_float_vector(arg, arr, size);
  }
};

template<>
struct Dispatcher<double> {
  static void assign_vector(tweak_variant* arg, const double* arr, size_t size) {
    return tweak_variant_assign_double_vector(arg, arr, size);
  }
};

template <typename Q>
struct VectorGen0<Q, false> {
    static tweak_variant generateRandomTweakVector(uint32_t size) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<typename ChooseIntegerDistribType<Q>::Type> distrib(
            std::numeric_limits<Q>::min(), std::numeric_limits<Q>::max());
        std::vector<Q> acc;
        acc.reserve(size);
        for (size_t ix = 0; ix < size; ++ix)
        {
            acc.emplace_back(static_cast<Q>(distrib(gen)));
        }
        tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
        Dispatcher<Q>::assign_vector(&result, &acc[0], size);
        return result;
    }
};

template <typename Q>
struct VectorGen0<Q, true> {
    static tweak_variant generateRandomTweakVector(uint32_t size) {
        std::default_random_engine generator;
        std::uniform_real_distribution<Q> distribution(-1., 1.);
        std::vector<Q> acc;
        acc.reserve(size);
        for (size_t ix = 0; ix < size; ++ix)
        {
            acc.emplace_back(distribution(generator));
        }
        tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
        Dispatcher<Q>::assign_vector(&result, &acc[0], size);
        return result;
    }
};

template<typename Q>
struct VectorGen
{
    static tweak_variant generateRandomTweakVector(uint32_t size) {
        return VectorGen0<Q, std::is_floating_point<Q>::value>::generateRandomTweakVector(size);
    }
};
}

struct TestContext {
    std::mutex m;

    std::condition_variable cv;

    int32_t answer_count;

    tweak_id id;

    tweak_variant sample_vector1 = {};

    tweak_variant sample_vector2 = {};

    tweak_variant_string string1 = {};

    tweak_variant_string string2 = {};

    tweak_variant_string string3 = {};

    void reset_answer_count(int32_t new_value);

    void notify_answer_received();

    void wait_answers();

    static void handle_connection_state_listener_vector(tweak_pickle_connection_state connection_state, void *cookie);

    static void handle_subscribe_vector(tweak_pickle_subscribe* subscribe, void *cookie);

    static void handle_add_item_vector(tweak_pickle_add_item *add_item, void *cookie);

    static void handle_client_change_tweak_vector(tweak_pickle_change_item *change, void *cookie);

    static void handle_server_change_tweak_vector(tweak_pickle_change_item *change_item, void *cookie);

    static void handle_server_remove_tweak_vector(tweak_pickle_remove_item* pickle_remove_item, void *cookie);
};

void TestContext::reset_answer_count(int32_t new_value) {
    std::lock_guard<std::mutex> lk(m);
    answer_count = new_value;
    cv.notify_all();
}

void TestContext::notify_answer_received() {
    std::lock_guard<std::mutex> lk(m);
    --answer_count;
    cv.notify_all();
}

void TestContext::wait_answers() {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [this]{ return answer_count <= 0; });
}

void TestContext::handle_connection_state_listener_vector(tweak_pickle_connection_state connection_state,
    void *cookie)
{
  (void) connection_state;
  TestContext &instance = *static_cast<TestContext*>(cookie);
  instance.notify_answer_received();
}

void TestContext::handle_subscribe_vector(tweak_pickle_subscribe* subscribe,
    void *cookie)
{
  (void) subscribe;
  TestContext &instance = *static_cast<TestContext*>(cookie);
  instance.notify_answer_received();
}

void TestContext::handle_add_item_vector(tweak_pickle_add_item *add_item, void *cookie) {
  TEST_CHECK(add_item != NULL);
  TEST_CHECK(cookie != NULL);
  TestContext &instance = *static_cast<TestContext*>(cookie);
  instance.id = add_item->id;
  tweak_variant_swap_string(&instance.string1, &add_item->uri);
  tweak_variant_swap_string(&instance.string2, &add_item->meta);
  tweak_variant_swap_string(&instance.string3, &add_item->description);
  tweak_variant_swap(&instance.sample_vector1, &add_item->current_value);
  tweak_variant_swap(&instance.sample_vector2, &add_item->default_value);
  instance.notify_answer_received();
}

void TestContext::handle_client_change_tweak_vector(tweak_pickle_change_item *change, void *cookie) {
  TEST_CHECK(change != NULL);
  TEST_CHECK(cookie != NULL);
  TestContext &instance = *static_cast<TestContext*>(cookie);
  instance.id = change->id;
  tweak_variant_swap(&instance.sample_vector1, &change->value);
  instance.notify_answer_received();
}

void TestContext::handle_server_change_tweak_vector(tweak_pickle_change_item *change, void *cookie) {
  TEST_CHECK(change != NULL);
  TEST_CHECK(cookie != NULL);
  TestContext &instance = *static_cast<TestContext*>(cookie);
  instance.id = change->id;
  tweak_variant_swap(&instance.sample_vector1, &change->value);
  instance.notify_answer_received();
}

void TestContext::handle_server_remove_tweak_vector(tweak_pickle_remove_item* pickle_remove_item,
    void *cookie)
{
  TEST_CHECK(pickle_remove_item != NULL);
  TEST_CHECK(cookie != NULL);
  TestContext &instance = *static_cast<TestContext*>(cookie);
  instance.id = pickle_remove_item->id;
  instance.notify_answer_received();
}

tweak_variant_string create_variant_string(const char* arg) {
  tweak_variant_string rv = {};
  tweak_assign_string(&rv, arg);
  return rv;
}

std::string generateNngUri(int port) {
  char uri0[256];
  snprintf(uri0, sizeof(uri0), TWEAK_DEFAULT_ENDPOINT_TEMPLATE, port);
  return uri0;
}

template<typename Q>
bool compareBuffers(const struct tweak_variant_buffer* vec1,
    const struct tweak_variant_buffer* vec2)
{
    TEST_CHECK(vec1->size == vec2->size);
    const Q* begin = static_cast<const Q*>(tweak_buffer_get_data_const(vec1));
    const Q* end = begin + tweak_buffer_get_size(vec1) / sizeof(Q);
    const Q* begin2 = static_cast<const Q*>(tweak_buffer_get_data_const(vec2));
    return std::equal(begin, end, begin2);
}

bool compareVectors(const tweak_variant* vec1, const tweak_variant* vec2) {
    switch (vec1->type) {
    case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_SINT8);
        return compareBuffers<int8_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_SINT16);
        return compareBuffers<int16_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_SINT32);
        return compareBuffers<int32_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_SINT64);
        return compareBuffers<int64_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_UINT8);
        return compareBuffers<uint8_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_UINT16);
        return compareBuffers<uint16_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_UINT32);
        return compareBuffers<uint32_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_UINT64);
        return compareBuffers<uint64_t>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_FLOAT);
        return compareBuffers<float>(&vec1->value.buffer, &vec2->value.buffer);
    case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
        TEST_CHECK(vec2->type == TWEAK_VARIANT_TYPE_VECTOR_DOUBLE);
        return compareBuffers<double>(&vec1->value.buffer, &vec2->value.buffer);
    default:
        TEST_CHECK(false);
        return false;
    }
}

template<typename Q>
void test_pickle_q() {
  int port = 32769 + rand() % 20000;
  std::string uri0;
  uri0 = generateNngUri(port);

  TestContext clientTestContext;
  TestContext serverTestContext;

  tweak_pickle_server_descriptor server_descriptor;
  memset(&server_descriptor, 0, sizeof(server_descriptor));
  server_descriptor.context_type = "nng";
  server_descriptor.uri = uri0.c_str();
  server_descriptor.params = "role=server";

  server_descriptor.skeleton.connection_state_listener.callback = &TestContext::handle_connection_state_listener_vector;
  server_descriptor.skeleton.connection_state_listener.cookie = &serverTestContext;
  server_descriptor.skeleton.subscribe_listener.callback = &TestContext::handle_subscribe_vector;
  server_descriptor.skeleton.subscribe_listener.cookie = &serverTestContext;
  server_descriptor.skeleton.change_item_listener.callback = &TestContext::handle_client_change_tweak_vector;
  server_descriptor.skeleton.change_item_listener.cookie = &serverTestContext;

  tweak_pickle_server_endpoint sep = tweak_pickle_create_server_endpoint(&server_descriptor);
  TEST_CHECK(sep != NULL);

  tweak_pickle_client_descriptor client_descriptor;
  memset(&client_descriptor, 0, sizeof(client_descriptor));
  client_descriptor.context_type = "nng";
  client_descriptor.uri = uri0.c_str();
  client_descriptor.params = "role=client";
  client_descriptor.skeleton.connection_state_listener.callback = &TestContext::handle_connection_state_listener_vector;
  client_descriptor.skeleton.connection_state_listener.cookie = &clientTestContext;
  client_descriptor.skeleton.add_item_listener.callback = &TestContext::handle_add_item_vector;
  client_descriptor.skeleton.add_item_listener.cookie = &clientTestContext;
  client_descriptor.skeleton.change_item_listener.callback = &TestContext::handle_server_change_tweak_vector;
  client_descriptor.skeleton.change_item_listener.cookie = &clientTestContext;
  client_descriptor.skeleton.remove_item_listener.callback = &TestContext::handle_server_remove_tweak_vector;
  client_descriptor.skeleton.remove_item_listener.cookie = &clientTestContext;

  serverTestContext.reset_answer_count(1);
  clientTestContext.reset_answer_count(1);
  tweak_pickle_client_endpoint cep = tweak_pickle_create_client_endpoint(&client_descriptor);
  TEST_CHECK(cep != NULL);

  clientTestContext.wait_answers();
  serverTestContext.wait_answers();

  tweak_variant current_value = VectorGen<Q>::generateRandomTweakVector(100);
  tweak_variant default_value = VectorGen<Q>::generateRandomTweakVector(100);

  tweak_pickle_add_item new_item = {};
  new_item.uri = create_variant_string("X");
  new_item.id = 26;
  new_item.meta = create_variant_string("spinbox");
  new_item.description = create_variant_string("First control");
  new_item.default_value = default_value;
  new_item.current_value = current_value;

  clientTestContext.reset_answer_count(1);
  tweak_pickle_server_add_item(sep, &new_item);
  clientTestContext.wait_answers();
  TEST_CHECK(compareVectors(&current_value, &clientTestContext.sample_vector1));
  TEST_CHECK(compareVectors(&default_value, &clientTestContext.sample_vector2));
  tweak_variant_destroy(&clientTestContext.sample_vector1);
  tweak_variant_destroy(&clientTestContext.sample_vector2);
  tweak_variant_destroy(&current_value);
  tweak_variant_destroy(&default_value);

  tweak_variant server_value = VectorGen<Q>::generateRandomTweakVector(100);

  tweak_pickle_change_item server_change_item = {};
  server_change_item.id = 17;
  server_change_item.value = server_value;

  tweak_variant client_value = VectorGen<Q>::generateRandomTweakVector(100);

  clientTestContext.reset_answer_count(1);
  tweak_pickle_server_change_item(sep, &server_change_item);
  clientTestContext.wait_answers();
  TEST_CHECK(compareVectors(&server_value, &clientTestContext.sample_vector1));
  tweak_variant_destroy(&clientTestContext.sample_vector1);

  clientTestContext.reset_answer_count(1);
  tweak_pickle_remove_item pickle_remove_item = {};
  pickle_remove_item.id = 42;
  tweak_pickle_server_remove_item(sep, &pickle_remove_item);
  clientTestContext.wait_answers();

  serverTestContext.reset_answer_count(1);
  tweak_pickle_client_subscribe(cep, NULL);
  serverTestContext.wait_answers();

  tweak_pickle_change_item client_change_item = {};
  client_change_item.id = 11;
  client_change_item.value = client_value;

  serverTestContext.reset_answer_count(1);
  tweak_pickle_client_change_item(cep, &client_change_item);
  serverTestContext.wait_answers();
  TEST_CHECK(compareVectors(&client_value, &serverTestContext.sample_vector1));
  tweak_variant_destroy(&serverTestContext.sample_vector1);
  tweak_variant_destroy(&clientTestContext.sample_vector1);
  tweak_pickle_destroy_client_endpoint(cep);
  tweak_pickle_destroy_server_endpoint(sep);
  tweak_variant_destroy(&server_value);
  tweak_variant_destroy(&client_value);
}

void test_pickle_uint8() {
    test_pickle_q<uint8_t>();
}

void test_pickle_uint16() {
    test_pickle_q<uint16_t>();
}

void test_pickle_uint32() {
    test_pickle_q<uint32_t>();
}

void test_pickle_uint64() {
    test_pickle_q<uint64_t>();
}

void test_pickle_sint8() {
    test_pickle_q<int8_t>();
}

void test_pickle_sint16() {
    test_pickle_q<int16_t>();
}

void test_pickle_sint32() {
    test_pickle_q<int32_t>();
}

void test_pickle_sint64() {
    test_pickle_q<int64_t>();
}

void test_pickle_float() {
    test_pickle_q<float>();
}

void test_pickle_double() {
    test_pickle_q<double>();
}

const char* TEST_STRING1 = "The three national symbols of England are the St. George's cross "
"(usually seen as a flag), the red rose and the Three Lions crest (usually seen as a badge). "
"The red rose is widely recognised as the national flower of England. The red rose is on the "
"badge of the English Rugby Union team.";

const char* TEST_STRING2 = "MailChimp’s voice is human. It’s familiar, friendly, and "
"straightforward. Our priority is explaining our products and helping our users get "
"their work done so they can get on with their lives. We want to educate people without "
"patronizing or confusing them.";

const char* TEST_STRING3 = "For example, textbooks are usually written with an objective "
"tone which includes facts and reasonable explanations. The objective tone is matter-of-fact "
"and neutral. The details are mostly facts. On the other hand, fiction and personal essays "
"are usually written with a subjective tone. A subjective tone uses words that describe "
"feelings, judgments, or opinions. The details are likely to include experiences, senses, "
"feelings, and thoughts";

const char* TEST_STRING4 = "Suzy is writing a job acceptance letter to an employer but "
"is unsure of the tone she should take in the message. She has decided to accept the "
"position. When she asks herself, \"What is my intent upon writing?\" she answers, "
"\"I want to accept the position, thank the company for the offer, and establish "
"goodwill with \"my new co-workers.\" As she writes the letter she quickly assumes "
"a tone that is appreciative for the offer and enthusiastic about beginning a new job.";

bool compareStrings(const tweak_variant* str1, const tweak_variant* str2) {
    if (str1->type != TWEAK_VARIANT_TYPE_STRING) {
        TEST_CHECK(false);
        return false;
    }
    if (str2->type != TWEAK_VARIANT_TYPE_STRING) {
        TEST_CHECK(false);
        return false;
    }
    const tweak_variant_string *s1 = &str1->value.string;
    const tweak_variant_string *s2 = &str2->value.string;
    return strcmp(tweak_variant_string_c_str(s1), tweak_variant_string_c_str(s2)) == 0;
}

void test_pickle_string() {
  int port = 32769 + rand() % 20000;
  std::string uri0;
  uri0 = generateNngUri(port);

  TestContext clientTestContext;
  TestContext serverTestContext;

  tweak_pickle_server_descriptor server_descriptor;
  memset(&server_descriptor, 0, sizeof(server_descriptor));
  server_descriptor.context_type = "nng";
  server_descriptor.uri = uri0.c_str();
  server_descriptor.params = "role=server";

  server_descriptor.skeleton.connection_state_listener.callback = &TestContext::handle_connection_state_listener_vector;
  server_descriptor.skeleton.connection_state_listener.cookie = &serverTestContext;
  server_descriptor.skeleton.subscribe_listener.callback = &TestContext::handle_subscribe_vector;
  server_descriptor.skeleton.subscribe_listener.cookie = &serverTestContext;
  server_descriptor.skeleton.change_item_listener.callback = &TestContext::handle_client_change_tweak_vector;
  server_descriptor.skeleton.change_item_listener.cookie = &serverTestContext;

  tweak_pickle_server_endpoint sep = tweak_pickle_create_server_endpoint(&server_descriptor);
  TEST_CHECK(sep != NULL);

  tweak_pickle_client_descriptor client_descriptor;
  memset(&client_descriptor, 0, sizeof(client_descriptor));
  client_descriptor.context_type = "nng";
  client_descriptor.uri = uri0.c_str();
  client_descriptor.params = "role=client";
  client_descriptor.skeleton.connection_state_listener.callback = &TestContext::handle_connection_state_listener_vector;
  client_descriptor.skeleton.connection_state_listener.cookie = &clientTestContext;
  client_descriptor.skeleton.add_item_listener.callback = &TestContext::handle_add_item_vector;
  client_descriptor.skeleton.add_item_listener.cookie = &clientTestContext;
  client_descriptor.skeleton.change_item_listener.callback = &TestContext::handle_server_change_tweak_vector;
  client_descriptor.skeleton.change_item_listener.cookie = &clientTestContext;
  client_descriptor.skeleton.remove_item_listener.callback = &TestContext::handle_server_remove_tweak_vector;
  client_descriptor.skeleton.remove_item_listener.cookie = &clientTestContext;

  serverTestContext.reset_answer_count(1);
  clientTestContext.reset_answer_count(1);
  tweak_pickle_client_endpoint cep = tweak_pickle_create_client_endpoint(&client_descriptor);
  TEST_CHECK(cep != NULL);

  clientTestContext.wait_answers();
  serverTestContext.wait_answers();

  tweak_variant current_value = {};
  tweak_variant_assign_string(&current_value, TEST_STRING1);
  tweak_variant default_value = {};
  tweak_variant_assign_string(&default_value, TEST_STRING2);

  tweak_pickle_add_item new_item = {};
  new_item.uri = create_variant_string("X");
  new_item.id = 26;
  new_item.meta = create_variant_string("spinbox");
  new_item.description = create_variant_string("First control");
  new_item.default_value = default_value;
  new_item.current_value = current_value;

  clientTestContext.reset_answer_count(1);
  tweak_pickle_server_add_item(sep, &new_item);
  clientTestContext.wait_answers();
  TEST_CHECK(compareStrings(&current_value, &clientTestContext.sample_vector1));
  TEST_CHECK(compareStrings(&default_value, &clientTestContext.sample_vector2));
  tweak_variant_destroy(&clientTestContext.sample_vector1);
  tweak_variant_destroy(&clientTestContext.sample_vector2);
  tweak_variant_destroy(&current_value);
  tweak_variant_destroy(&default_value);

  tweak_variant server_value = {};
  tweak_variant_assign_string(&server_value, TEST_STRING3);

  tweak_pickle_change_item server_change_item = {};
  server_change_item.id = 17;
  server_change_item.value = server_value;

  tweak_variant client_value = {};
  tweak_variant_assign_string(&client_value, TEST_STRING4);

  clientTestContext.reset_answer_count(1);
  tweak_pickle_server_change_item(sep, &server_change_item);
  clientTestContext.wait_answers();
  TEST_CHECK(compareStrings(&server_value, &clientTestContext.sample_vector1));

  clientTestContext.reset_answer_count(1);
  tweak_pickle_remove_item pickle_remove_item = {};
  pickle_remove_item.id = 42;
  tweak_pickle_server_remove_item(sep, &pickle_remove_item);
  clientTestContext.wait_answers();

  serverTestContext.reset_answer_count(1);
  tweak_pickle_client_subscribe(cep, NULL);
  serverTestContext.wait_answers();

  tweak_pickle_change_item client_change_item = {};
  client_change_item.id = 11;
  client_change_item.value = client_value;

  serverTestContext.reset_answer_count(1);
  tweak_pickle_client_change_item(cep, &client_change_item);
  serverTestContext.wait_answers();
  TEST_CHECK(compareStrings(&client_value, &serverTestContext.sample_vector1));

  tweak_pickle_destroy_client_endpoint(cep);
  tweak_pickle_destroy_server_endpoint(sep);

  tweak_variant_destroy(&serverTestContext.sample_vector1);
  tweak_variant_destroy(&clientTestContext.sample_vector1);
  tweak_variant_destroy(&server_value);
  tweak_variant_destroy(&client_value);
}

TEST_LIST = {
   { "test-pickle_uint8", test_pickle_uint8 },
   { "test-pickle_uint16", test_pickle_uint16 },
   { "test-pickle_uint32", test_pickle_uint32 },
   { "test-pickle_uint64", test_pickle_uint64 },
   { "test-pickle_sint8", test_pickle_sint8 },
   { "test-pickle_sint16", test_pickle_sint16 },
   { "test-pickle_sint32", test_pickle_sint32 },
   { "test-pickle_sint64", test_pickle_sint64 },
   { "test-pickle_float", test_pickle_float },
   { "test-pickle_double", test_pickle_double },
   { "test_pickle_string", test_pickle_string },
     { NULL, NULL }     /* zeroed record marking the end of the list */
};
