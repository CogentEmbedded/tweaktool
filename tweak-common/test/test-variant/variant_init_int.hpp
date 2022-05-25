/**
 * @file variant_init_int.hpp
 * @ingroup internal
 *
 * @brief Type conversion templates.
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


#ifndef TWEAK_VARIANT_INIT_INT_HPP_INCLUDED
#define TWEAK_VARIANT_INIT_INT_HPP_INCLUDED

#include <tweak2/variant.h>

#include <type_traits>
#include <iterator>
#include <vector>
#include <string>
#include <tuple>


namespace detail {

template<typename Q>
class AdHocBuffer {
  const Q *const begin_;

  const Q *const end_;
public:
  using value_type = Q;

  using const_iterator = const Q*;

  AdHocBuffer(const Q* begin, const Q* end)
    :begin_(begin), end_(end)
  {}

  const_iterator begin() const {
    return begin_;
  }

  const_iterator end() const {
    return end_;
  }

  const Q* data() const {
    return begin_;
  }
};

template <tweak_variant_type TweakVariantType> struct ChooseCTypeByVT { };

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_SINT8> {
  using Type = int8_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_SINT16> {
  using Type = int16_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_SINT32> {
  using Type = int32_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_SINT64> {
  using Type = int64_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_UINT8> {
  using Type = uint8_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_UINT16> {
  using Type = uint16_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_UINT32> {
  using Type = uint32_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_UINT64> {
  using Type = uint64_t;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_FLOAT> {
  using Type = float;
};

template <> struct ChooseCTypeByVT<TWEAK_VARIANT_TYPE_VECTOR_DOUBLE> {
  using Type = double;
};

template<typename ArgumentType>
struct HasContiguousStorage {
  static constexpr bool value = false;
};

template<typename ArgumentType>
struct HasContiguousStorage<std::vector<ArgumentType>> {
  static constexpr bool value = true;
};

template<typename ArgumentType, std::size_t N>
struct HasContiguousStorage<std::array<ArgumentType, N>> {
  static constexpr bool value = true;
};

template<tweak_variant_type TweakVariantType, typename ScalarType, typename ArgumentType>
struct AssignScalar {
  static void init(tweak_variant* arg, ArgumentType value) {
    (void)arg;
    (void)value;
    TWEAK_FATAL("Invalid type conversion");
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_BOOL, bool, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_bool(arg, static_cast<bool>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_SINT8, int8_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_sint8(arg, static_cast<int8_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_SINT16, int16_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_sint16(arg, static_cast<int16_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_SINT32, int32_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_sint32(arg, static_cast<int32_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_SINT64, int64_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_sint64(arg, static_cast<int64_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_UINT8, uint8_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_uint8(arg, static_cast<uint8_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_UINT16, uint16_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_uint16(arg, static_cast<uint16_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_UINT32, uint32_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_uint32(arg, static_cast<uint32_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_UINT64, uint64_t, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_uint64(arg, static_cast<uint64_t>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_FLOAT, float, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_float(arg, static_cast<float>(value));
  }
};

template<typename ArgumentType>
struct AssignScalar<TWEAK_VARIANT_TYPE_DOUBLE, double, ArgumentType> {
  static void init(tweak_variant* arg, ArgumentType value) {
    tweak_variant_assign_double(arg, static_cast<double>(value));
  }
};

template<tweak_variant_type TweakVariantType, typename ScalarType, typename ArgumentType>
struct ConvertScalarArgument {
  template< typename U = ArgumentType >
  static void init(tweak_variant* arg,
    typename std::enable_if
      <
        std::is_arithmetic<U>::value, U
      >::type value)
  {
    AssignScalar<TweakVariantType, ScalarType, ArgumentType>::init(arg, value);
  }

  template< typename U = ArgumentType >
  static void init(tweak_variant* arg,
    typename std::enable_if
      <
        !std::is_arithmetic<U>::value, U
      >::type value)
  {
    (void)arg;
    (void)value;
    TWEAK_FATAL("Invalid type conversion");
  }
};

template<tweak_variant_type TweakVariantType, typename ElementType>
struct AssignContiguousBuffer {
  static constexpr bool copyNeeded = true;
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_SINT8, int8_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const int8_t* buffer, std::size_t size) {
    tweak_variant_assign_sint8_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_SINT16, int16_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const int16_t* buffer, std::size_t size) {
    tweak_variant_assign_sint16_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_SINT32, int32_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const int32_t* buffer, std::size_t size) {
    tweak_variant_assign_sint32_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_SINT64, int64_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const int64_t* buffer, std::size_t size) {
    tweak_variant_assign_sint64_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_UINT8, uint8_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const uint8_t* buffer, std::size_t size) {
    tweak_variant_assign_uint8_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_UINT16, uint16_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const uint16_t* buffer, std::size_t size) {
    tweak_variant_assign_uint16_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_UINT32, uint32_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const uint32_t* buffer, std::size_t size) {
    tweak_variant_assign_uint32_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_UINT64, uint64_t> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const uint64_t* buffer, std::size_t size) {
    tweak_variant_assign_uint64_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_FLOAT, float> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const float* buffer, std::size_t size) {
    tweak_variant_assign_float_vector(arg, buffer, size);
  }
};

template<>
struct AssignContiguousBuffer<TWEAK_VARIANT_TYPE_VECTOR_DOUBLE, double> {
  static constexpr bool copyNeeded = false;

  static void init(tweak_variant* arg, const double* buffer, std::size_t size) {
    tweak_variant_assign_double_vector(arg, buffer, size);
  }
};

template<typename ArgumentType>
struct HasConstIterator
{
private:
  typedef char                      yes;
  typedef struct { char array[2]; } no;

  template<typename C> static yes test(typename C::const_iterator*);
  template<typename C> static no  test(...);
public:
  static const bool value = sizeof(test<ArgumentType>(0)) == sizeof(yes);
  typedef ArgumentType type;
};

template <typename ArgumentType>
struct HasBeginEnd
{
  template<typename C> static char (&f(typename std::enable_if<
    std::is_same<decltype(static_cast<typename C::const_iterator (C::*)() const>(&C::begin)),
    typename C::const_iterator(C::*)() const>::value, void>::type*))[1];

  template<typename C> static char (&f(...))[2];

  template<typename C> static char (&g(typename std::enable_if<
    std::is_same<decltype(static_cast<typename C::const_iterator (C::*)() const>(&C::end)),
    typename C::const_iterator(C::*)() const>::value, void>::type*))[1];

  template<typename C> static char (&g(...))[2];

  static bool const beg_value = sizeof(f<ArgumentType>(0)) == 1;
  static bool const end_value = sizeof(g<ArgumentType>(0)) == 1;
};

template <typename ArgumentType>
struct HasCStr
{
  template<typename C> static char (&f(typename std::enable_if<
    std::is_same<decltype(static_cast<typename C::value_type (C::*)() const>(&C::c_str)),
    typename C::value_type(C::*)() const>::value, void>::type*))[1];

  template<typename C> static char (&f(...))[2];

  static bool const c_str_value = sizeof(f<ArgumentType>(0)) == 1;
};

template<typename ArgumentType>
struct IsStlContainer :
  std::integral_constant
  <
    bool,
    HasConstIterator<ArgumentType>::value
    && HasBeginEnd<ArgumentType>::beg_value
    && HasBeginEnd<ArgumentType>::end_value
  >
{ };


template<typename ArgumentType>
struct IsStdString :
  std::integral_constant
  <
    bool,
    HasConstIterator<ArgumentType>::value
    && HasBeginEnd<ArgumentType>::beg_value
    && HasBeginEnd<ArgumentType>::end_value
    && HasCStr<ArgumentType>::c_str_value
  >
{ };


template <typename Q>
struct ChooseScalarVariantTypeByCType {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_NULL;
};
template <> struct ChooseScalarVariantTypeByCType<bool> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_BOOL;
};

template <> struct ChooseScalarVariantTypeByCType<int8_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_SINT8;
};

template <> struct ChooseScalarVariantTypeByCType<int16_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_SINT16;
};

template <> struct ChooseScalarVariantTypeByCType<int32_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_SINT32;
};

template <> struct ChooseScalarVariantTypeByCType<int64_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_SINT64;
};

template <> struct ChooseScalarVariantTypeByCType<uint8_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_UINT8;
};

template <> struct ChooseScalarVariantTypeByCType<uint16_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_UINT16;
};

template <> struct ChooseScalarVariantTypeByCType<uint32_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_UINT32;
};

template <> struct ChooseScalarVariantTypeByCType<uint64_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_UINT64;
};

template <> struct ChooseScalarVariantTypeByCType<float> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_FLOAT;
};

template <> struct ChooseScalarVariantTypeByCType<double> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_DOUBLE;
};

template<typename ElementType>
struct ChooseVectorVariantTypeByElementCType
{
    static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_NULL;
};
template <> struct ChooseVectorVariantTypeByElementCType<int8_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_SINT8;
};

template <> struct ChooseVectorVariantTypeByElementCType<int16_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_SINT16;
};

template <> struct ChooseVectorVariantTypeByElementCType<int32_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_SINT32;
};

template <> struct ChooseVectorVariantTypeByElementCType<int64_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_SINT64;
};

template <> struct ChooseVectorVariantTypeByElementCType<uint8_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_UINT8;
};

template <> struct ChooseVectorVariantTypeByElementCType<uint16_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_UINT16;
};

template <> struct ChooseVectorVariantTypeByElementCType<uint32_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_UINT32;
};

template <> struct ChooseVectorVariantTypeByElementCType<uint64_t> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_UINT64;
};

template <> struct ChooseVectorVariantTypeByElementCType<float> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_FLOAT;
};

template <> struct ChooseVectorVariantTypeByElementCType<double> {
   static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_VECTOR_DOUBLE;
};

template <typename Q>
struct DeduceVectorTypeByElementType {
  using ElementType = typename Q::value_type;
  static constexpr tweak_variant_type TweakVariantType =
    ChooseVectorVariantTypeByElementCType<ElementType>::TweakVariantType;
};

template <typename Q, bool StlContainer, bool StdString>
struct ChooseVectorVariantTypeByCType {
  static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_NULL;
};

template <typename Q>
struct ChooseVectorVariantTypeByCType<Q, true, false> {
  static constexpr tweak_variant_type TweakVariantType = DeduceVectorTypeByElementType<Q>::TweakVariantType;
};

template <typename Q>
struct ChooseVectorVariantTypeByCType<Q, true, true> {
  static constexpr tweak_variant_type TweakVariantType = TWEAK_VARIANT_TYPE_STRING;
};

template <typename Q>
struct ChooseVariantTypeByCType {
private:
  using R = typename std::remove_reference<Q>::type;
public:
  static constexpr tweak_variant_type TweakVariantType =
    std::is_arithmetic<R>::value
      ? ChooseScalarVariantTypeByCType<R>::TweakVariantType
      : ChooseVectorVariantTypeByCType<R, IsStlContainer<R>::value, IsStdString<R>::value>::TweakVariantType;
};

template<tweak_variant_type TweakVariantType, typename ArgumentType, typename ElementType>
struct ConvertStlContainer {
  template<typename U = const ArgumentType&>
  static void init(tweak_variant* arg, typename std::enable_if
      <
        HasContiguousStorage<typename std::remove_reference<U>::type>::value
          && !AssignContiguousBuffer<TweakVariantType, ElementType>::copyNeeded,
        U
      >::type val)
  {
    AssignContiguousBuffer
      <
        TweakVariantType,
        ElementType
      >::init(arg, val.data(), std::distance(val.cbegin(), val.cend()));
  }

  template<typename U = const ArgumentType&>
  static void init(tweak_variant* arg,
    typename std::enable_if<
        !HasContiguousStorage<typename std::remove_reference<U>::type>::value
          || AssignContiguousBuffer<TweakVariantType, ElementType>::copyNeeded,
        U
      >::type val)
  {
    using R = typename ChooseCTypeByVT<TweakVariantType>::Type;
    std::vector<R> contiguousCopy;
    for (auto e : val) {
      contiguousCopy.emplace_back(static_cast<R>(e));
    }
    AssignContiguousBuffer
      <
        TweakVariantType,
        R
      >::init(arg, contiguousCopy.data(), std::distance(contiguousCopy.cbegin(), contiguousCopy.cend()));
  }
};

template<typename ArgumentType>
struct ConvertSequence {
  template<typename U = const ArgumentType&>
  static void init(tweak_variant_type type, tweak_variant* arg,
    typename std::enable_if<IsStlContainer<typename std::remove_reference<U>::type>::value, U>::type value)
  {
    using R = typename std::remove_reference<U>::type;
    using ElementType = typename R::value_type;
    switch (type) {
    case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_SINT8,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_SINT16,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_SINT32,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_SINT64,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_UINT8,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_UINT16,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_UINT32,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_UINT64,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_FLOAT,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
      ConvertStlContainer
        <
          TWEAK_VARIANT_TYPE_VECTOR_DOUBLE,
          ArgumentType,
          ElementType
        >::init(arg, value);
      break;
    default:
      TWEAK_FATAL("Invalid type conversion");
      break;
    }
  }

  template<typename U = const ArgumentType&>
  static void init(tweak_variant_type type, tweak_variant* arg,
    typename std::enable_if<!IsStlContainer<typename std::remove_reference<U>::type>::value, U>::type value)
  {
    (void)type;
    (void)arg;
    (void)value;
    TWEAK_FATAL("Invalid type conversion");
  }
};

template<typename ArgumentType>
struct ConvertStdString {
  static void init(tweak_variant* arg, const ArgumentType& val) {
    (void) arg;
    (void) val;
    TWEAK_FATAL("Invalid type conversion");
  }
};

template<>
struct ConvertStdString<std::string> {
  static void init(tweak_variant* arg, const std::string &val) {
    tweak_variant_assign_string(arg, val.c_str());
  }
};

template<typename ArgumentType>
void init(tweak_variant* arg, tweak_variant_type type, const ArgumentType &val) {
  switch (type) {
  case TWEAK_VARIANT_TYPE_NULL:
    arg->type = TWEAK_VARIANT_TYPE_NULL;
    break;
  case TWEAK_VARIANT_TYPE_BOOL:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_BOOL,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_SINT8:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_SINT8,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_SINT16:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_SINT16,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_SINT32,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_SINT64:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_SINT64,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_UINT8:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_UINT8,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_UINT16:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_UINT16,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_UINT32:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_UINT32,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_UINT64:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_UINT64,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_FLOAT:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_FLOAT,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_DOUBLE:
    ConvertScalarArgument
      <
        TWEAK_VARIANT_TYPE_DOUBLE,
        ArgumentType,
        typename std::remove_reference<ArgumentType>::type
      >::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_STRING:
    ConvertStdString<ArgumentType>::init(arg, val);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    ConvertSequence<ArgumentType>::init(type, arg, val);
    break;
  default:
    TWEAK_FATAL("Unknown type: %d", type);
    break;
  }
}

}

#endif // TWEAK_VARIANT_INIT_INT_HPP_INCLUDED