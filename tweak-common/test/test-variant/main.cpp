/**
 * @file main.c
 *
 * @brief Test suite for common library.
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

#include <tweak2/variant.h>
#include <tweak2/json.h>

#include <acutest.h>
#include <algorithm>
#include <random>
#include <cmath>
#include <limits>
#include <cstring>
#include <vector>
#include "variant.hpp"

namespace
{

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

template <typename Q, bool F> struct VectorGen0 { };

template <typename Q> struct VectorGen0<Q, false> {
   static tweak2::Variant generateRandomTweakVector(uint32_t size) {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<typename ChooseIntegerDistribType<Q>::Type> distrib(
          std::numeric_limits<Q>::min(), std::numeric_limits<Q>::max());

      std::vector<Q> acc;
      for (size_t ix = 0; ix < size; ++ix)
      {
         acc.emplace_back(static_cast<Q>(distrib(gen)));
      }

      return acc;
   }
};

template <typename Q> struct VectorGen0<Q, true> {
   static tweak2::Variant generateRandomTweakVector(uint32_t size) {
      std::default_random_engine generator;
      std::uniform_real_distribution<Q> distribution(
         std::numeric_limits<Q>::min(),
         std::numeric_limits<Q>::max());
      std::vector<Q> acc;
      for (size_t ix = 0; ix < size; ++ix)
      {
         acc.emplace_back(static_cast<Q>(distribution(generator)));
      }

      return acc;
   }
};

template <typename Q> struct VectorGen {
   static tweak2::Variant generateRandomTweakVector(uint32_t size) {
      return VectorGen0<Q, std::is_floating_point<Q>::value>::generateRandomTweakVector(size);
   }
};

constexpr size_t Size = 100;

template<typename Q> struct CompareNumbers {
   template<typename I, typename W> static bool is_equal(I b0, I e0, W b1) {
      return std::equal(b0, e0, b1);
   }
};

template<> struct CompareNumbers<float> {
   template<typename I, typename W> static bool is_equal(I b0, I e0, W b1) {
      return std::equal(b0, e0, b1, [](float a, float b) -> bool {
         return std::fabs(a - b) < std::numeric_limits<float>::epsilon();
      });
   }
};

template<> struct CompareNumbers<double> {
   template<typename I, typename W> static bool is_equal(I b0, I e0, W b1) {
      return std::equal(b0, e0, b1, [](double a, double b) -> bool {
         return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
      });
   }
};

template<typename Q> void test_to_from_string() {
   tweak2::Variant random_variant = VectorGen<Q>::generateRandomTweakVector(Size);
   std::string s = random_variant.toString();
   auto rv2_tmp = tweak2::Variant::fromString(s,
      tweak2::ChooseVariantTypeByCType<std::vector<Q>>::TweakVariantType);
   if (!rv2_tmp) {
     throw "Unexpected!";
   }
   tweak2::Variant rv2 = *rv2_tmp;
   auto b0_tmp = random_variant.getBuffer<Q>();
   if (!b0_tmp) {
     throw "Unexpected!";
   }
   const Q *b0 = *b0_tmp;
   const Q *e0 = b0 + Size;
   auto b1_tmp = rv2.getBuffer<Q>();
   if (!b1_tmp) {
     throw "Unexpected!";
   }
   const Q *b1 = *b1_tmp;
   TEST_CHECK(CompareNumbers<Q>::is_equal(b0, e0, b1));
}

template<typename Q> struct CheckStrType {};

template<> struct CheckStrType<int8_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("sint8", arg) == 0;
   }
};

template<> struct CheckStrType<int16_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("sint16", arg) == 0;
   }
};

template<> struct CheckStrType<int32_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("sint32", arg) == 0;
   }
};

template<> struct CheckStrType<int64_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("sint64", arg) == 0;
   }
};

template<> struct CheckStrType<uint8_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("uint8", arg) == 0;
   }
};

template<> struct CheckStrType<uint16_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("uint16", arg) == 0;
   }
};

template<> struct CheckStrType<uint32_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("uint32", arg) == 0;
   }
};

template<> struct CheckStrType<uint64_t> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("uint64", arg) == 0;
   }
};

template<> struct CheckStrType<float> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("float", arg) == 0;
   }
};

template<> struct CheckStrType<double> {
   static bool checkTypeStr(const char* arg) {
      return std::strcmp("double", arg) == 0;
   }
};

template<typename Q>
std::vector<Q> readArrayNode(const struct tweak_json_node* array_node) {
   std::vector<Q> result;
   std::size_t size;
   if (tweak_json_get_array_size(array_node, &size) != TWEAK_JSON_GET_SIZE_SUCCESS) {
      throw "Parse error";
   }
   for (std::size_t ix = 0; ix < size; ++ix) {
      const struct tweak_json_node* item_node = tweak_json_get_array_item(array_node, ix, TWEAK_JSON_NODE_TYPE_NUMBER);
      if (item_node == nullptr) {
         throw "Parse error";
      }

      auto tmp =
        tweak2::Variant::fromString(tweak_json_node_as_c_str(item_node),
          tweak2::ChooseVariantTypeByCType<Q>::TweakVariantType);

      if (!tmp) {
        throw "Parse error";
      }

      tweak2::Variant v_scalar = *tmp;

      auto scalar = v_scalar.getScalar<Q>();
      if (!scalar) {
        throw "Parse error";
      }
      result.push_back(*scalar);
   }
   return result;
}

template<typename Q>
void test_to_json()
{
   tweak2::Variant random_variant = VectorGen<Q>::generateRandomTweakVector(Size);
   std::string s = random_variant.toJson();
   struct tweak_json_node* root = tweak_json_parse(s.c_str());
   TEST_CHECK(root != nullptr);
   TEST_CHECK(tweak_json_get_type(root) == TWEAK_JSON_NODE_TYPE_OBJECT);
   const struct tweak_json_node* vector_node =
      tweak_json_get_object_field(root, "vector", TWEAK_JSON_NODE_TYPE_OBJECT);
   TEST_CHECK(tweak_json_get_type(vector_node) == TWEAK_JSON_NODE_TYPE_OBJECT);

   const struct tweak_json_node* item_type_str_node =
      tweak_json_get_object_field(vector_node, "item_type", TWEAK_JSON_NODE_TYPE_STRING);

   TEST_CHECK(tweak_json_get_type(item_type_str_node) == TWEAK_JSON_NODE_TYPE_STRING);
   TEST_CHECK(CheckStrType<Q>::checkTypeStr(tweak_json_node_as_c_str(item_type_str_node)));

   const struct tweak_json_node* items_array_node =
      tweak_json_get_object_field(vector_node, "items", TWEAK_JSON_NODE_TYPE_ARRAY);

   TEST_CHECK(tweak_json_get_type(items_array_node) == TWEAK_JSON_NODE_TYPE_ARRAY);

   std::vector<Q> array = readArrayNode<Q>(items_array_node);

   auto b0_tmp = random_variant.getBuffer<Q>();
   if (!b0_tmp) {
      throw "Error!";
   }
   const Q *b0 = *b0_tmp;
   const Q *e0 = b0 + Size;

   TEST_CHECK(CompareNumbers<Q>::is_equal(b0, e0, array.begin()));

   tweak_json_destroy(root);
}

}

void test_common_variant_sint8()
{
   test_to_from_string<int8_t>();
}

void test_common_variant_sint16()
{
   test_to_from_string<int16_t>();
}

void test_common_variant_sint32()
{
   test_to_from_string<int32_t>();
}

void test_common_variant_sint64()
{
   test_to_from_string<int64_t>();
}

void test_common_variant_uint8()
{
   test_to_from_string<uint8_t>();
}

void test_common_variant_uint16()
{
   test_to_from_string<uint16_t>();
}

void test_common_variant_uint32()
{
   test_to_from_string<uint32_t>();
}

void test_common_variant_uint64()
{
   test_to_from_string<uint64_t>();
}

void test_common_variant_float()
{
   test_to_from_string<float>();
}

void test_common_variant_double()
{
   test_to_from_string<double>();
}

void test_variant_to_json_sint8()
{
   test_to_json<int8_t>();
}

void test_variant_to_json_sint16()
{
   test_to_json<int16_t>();
}

void test_variant_to_json_sint32()
{
   test_to_json<int32_t>();
}

void test_variant_to_json_sint64()
{
   test_to_json<int64_t>();
}

void test_variant_to_json_uint8()
{
   test_to_json<uint8_t>();
}

void test_variant_to_json_uint16()
{
   test_to_json<uint16_t>();
}

void test_variant_to_json_uint32()
{
   test_to_json<uint32_t>();
}

void test_variant_to_json_uint64()
{
   test_to_json<uint64_t>();
}

void test_variant_to_json_float()
{
   test_to_json<float>();
}

void test_variant_to_json_double()
{
   test_to_json<double>();
}

TEST_LIST = {
    {"test_variant_to_json_sint8", test_variant_to_json_sint8},
    {"test_variant_to_json_sint16", test_variant_to_json_sint16},
    {"test_variant_to_json_sint32", test_variant_to_json_sint32},
    {"test_variant_to_json_sint64", test_variant_to_json_sint64},
    {"test_variant_to_json_uint8", test_variant_to_json_uint8},
    {"test_variant_to_json_uint16", test_variant_to_json_uint16},
    {"test_variant_to_json_uint32", test_variant_to_json_uint32},
    {"test_variant_to_json_uint64", test_variant_to_json_uint64},
    {"test_variant_to_json_float", test_variant_to_json_float},
    {"test_variant_to_json_double", test_variant_to_json_double},
    {"test_common_variant_sint8", test_common_variant_sint8},
    {"test_common_variant_sint16", test_common_variant_sint16},
    {"test_common_variant_sint32", test_common_variant_sint32},
    {"test_common_variant_sint64", test_common_variant_sint64},
    {"test_common_variant_uint8", test_common_variant_uint8},
    {"test_common_variant_uint16", test_common_variant_uint16},
    {"test_common_variant_uint32", test_common_variant_uint32},
    {"test_common_variant_uint64", test_common_variant_uint64},
    {"test_common_variant_float", test_common_variant_float},
    {"test_common_variant_double", test_common_variant_double},
    {NULL, NULL} /* zeroed record marking the end of the list */
};
