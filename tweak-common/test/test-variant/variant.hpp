/**
 * @file variant.hpp
 * @ingroup tweak-api
 *
 * @brief Tweak variant type.
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

/**
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_VARIANT_HPP_INCLUDED
#define TWEAK_VARIANT_HPP_INCLUDED

#include <tweak2/variant.h>

#include <type_traits>
#include <iterator>
#include <vector>
#include <array>
#include <string>
#include <optional>

#include "variant_init_int.hpp"

namespace tweak2
{
/**
 * Maps C++ type to @see tweak_variant_type.
 */
template <typename Q>
struct ChooseVariantTypeByCType {
  static constexpr tweak_variant_type TweakVariantType =
    detail::ChooseVariantTypeByCType<Q>::TweakVariantType;
};
class Variant {
  tweak_variant tv_;

  Variant(tweak_variant tv)
    :tv_(tv)
  {}

public:
  /**
   * @brief Destructor.
   *
   */
  ~Variant() {
    tweak_variant_destroy(&tv_);
  }

  /**
   * @brief Default constructor.
   */
  Variant()
    :tv_(TWEAK_VARIANT_INIT_EMPTY)
  {}

  /**
   * @brief Constructs from a C array.
   *
   * @param arg C array.
   */
  template <
    typename Q,
    std::size_t N,
    typename
      std::enable_if<
        ChooseVariantTypeByCType<detail::AdHocBuffer<Q>>::TweakVariantType != TWEAK_VARIANT_TYPE_NULL
        && !std::is_same<Q, char>::value
      >::type* = nullptr
  >
  Variant(const Q(&arg)[N],
          tweak_variant_type type = ChooseVariantTypeByCType<detail::AdHocBuffer<Q>>::TweakVariantType)
    :tv_(TWEAK_VARIANT_INIT_EMPTY)
  {
    detail::init<detail::AdHocBuffer<Q>>(&tv_, type, detail::AdHocBuffer<Q>(&arg[0], &arg[N]));
  }

  /**
   * @brief Constructs variant string from a C string literal.
   *
   * @param arg C string.
   */
  template <std::size_t N>
  Variant(const char(&arg)[N])
    :tv_(TWEAK_VARIANT_INIT_EMPTY)
  {
    detail::init<std::string>(&tv_, TWEAK_VARIANT_TYPE_STRING, std::string(arg));
  }

  /**
   * @brief allows Variant arg = {1,2,3} assignments.
   *
   * @param arg C array.
   */
  template<typename Q>
  Variant(std::initializer_list<Q> arg)
    :tv_(TWEAK_VARIANT_INIT_EMPTY)
  {
    tweak_variant_type type = ChooseVariantTypeByCType<std::initializer_list<Q>>::TweakVariantType;
    detail::init<std::initializer_list<Q>>(&tv_, type, arg);
  }

  /**
   * @brief Constructs variant string from a C string literal.
   *
   * @param arg C array.
   */
  Variant(const char *arg)
    :tv_(TWEAK_VARIANT_INIT_EMPTY)
  {
    detail::init<std::string>(&tv_, TWEAK_VARIANT_TYPE_STRING, std::string(arg));
  }

  /**
   * @brief Constructs from a scalar or an stl container.
   *
   * @param arg scalar or stl container of scalars.
   */
  template <
    typename Q,
    typename
      std::enable_if<
        ChooseVariantTypeByCType<Q>::TweakVariantType != TWEAK_VARIANT_TYPE_NULL
      >::type* = nullptr
  >
  Variant(const Q& arg,
          tweak_variant_type type = ChooseVariantTypeByCType<Q>::TweakVariantType)
    :tv_(TWEAK_VARIANT_INIT_EMPTY)
  {
    detail::init<Q>(&tv_, type, arg);
  }

  /**
   * @brief Copy constructor.
   *
   * @param o instance to copy.
   */
  Variant(const Variant& o)
    :tv_(tweak_variant_copy(&o.tv_))
  {}

  /**
   * @brief Move constructor.
   *
   * @param o instance to move.
   */
  Variant(Variant&& o)
    :tv_(TWEAK_VARIANT_INIT_EMPTY)
  {
    tweak_variant_swap(&tv_, &o.tv_);
  }

  /**
   * @brief create Variant instance given type and string representation.
   *
   * @param target_type intended type of Variant.
   * @param str Variant representation.
   * @param strict if this optional parameter is false, this function shall allow
   * truncated values as valid results. That means, string "256" shall
   * yield rounded value 255 for @p target_type being equal to @see TWEAK_VARIANT_TYPE_UINT8.
   *
   * @return typle comprising of result and parse status. If parse failed, result is null Variant
   */
  static std::optional<Variant>
    fromString(const std::string &str, tweak_variant_type target_type, bool strict = true)
  {
    tweak_variant result = TWEAK_VARIANT_INIT_EMPTY;
    tweak_variant_type_conversion_result conversion_result =
        tweak_variant_from_string(str.c_str(), target_type, &result);

    bool success;
    switch (conversion_result) {
    case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS:
      success = true;
      break;
    case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED:
      success = !strict;
      break;
    default:
      success = false;
      break;
    }

    return success ? std::optional<Variant>(Variant(result)) : std::nullopt;
  }

  /**
   * Convert Variant instance to string.
   *
   * @return string representation.
   */
  std::string toString() const {
    tweak_variant_string tmp = tweak_variant_to_string(&tv_);
    std::string result(tweak_variant_string_c_str(&tmp));
    tweak_variant_destroy_string(&tmp);
    return result;
  }

  /**
   * Convert Variant instance to string.
   *
   * @return string representation as json string.
   */
  std::string toJson() const{
    tweak_variant_string tmp = tweak_variant_to_json(&tv_);
    std::string result(tweak_variant_string_c_str(&tmp));
    tweak_variant_destroy_string(&tmp);
    return result;
  }

  /**
   * Assign Variant value.
   *
   * @param o Variant to assign.
   *
   * @return self reference.
   */
  Variant& operator=(const Variant& o)
  {
    Variant tmp(o);
    tweak_variant_swap(&tv_, &tmp.tv_);
    return *this;
  }

  /**
   * Move Variant value.
   *
   * @param o Variant to move.
   *
   * @return self reference.
   */
  Variant& operator=(Variant&& o)
  {
    tweak_variant_swap(&tv_, &o.tv_);
    tweak_variant_destroy(&o.tv_);
    return *this;
  }

  /**
   * @brief Assigns from a C array.
   *
   * @param arg C array.
   */
  template <
    typename Q,
    std::size_t N,
    typename
      std::enable_if<
        ChooseVariantTypeByCType<detail::AdHocBuffer<Q>>::TweakVariantType != TWEAK_VARIANT_TYPE_NULL
        && !std::is_same<Q, char>::value
      >::type* = nullptr
  >
  Variant& operator=(const Q(&arg)[N])
  {
    Variant tmp(arg);
    tweak_variant_swap(&tv_, &tmp.tv_);
    return *this;
  }

  /**
   * @brief Creates a variant string from a C string literal.
   *
   * @param arg C string literal.
   */
  template <std::size_t N>
  Variant& operator=(const char(&arg)[N])
  {
    return operator=(&arg[0]);
  }

  /**
   * @brief Creates a variant string from a const char*.
   *
   * @param arg C string literal.
   */
  Variant& operator=(const char *arg)
  {
    Variant tmp(arg);
    tweak_variant_swap(&tv_, &tmp.tv_);
    return *this;
  }

  /**
   * Checks Variant values for equality.
   *
   * @param o Variant instance to compare with.
   *
   * @return true if equals.
   */
  bool operator==(const Variant& o) const
  {
    return tweak_variant_is_equal(&tv_, &o.tv_);
  }

  /**
   * Assign value to a Variant.
   *
   * @param arg scalar value ar an stl comtainer.
   *
   * @return self reference.
   */
  template<
    typename Q,
    typename
      std::enable_if<
        ChooseVariantTypeByCType<Q>::TweakVariantType != TWEAK_VARIANT_TYPE_NULL
      >::type* = nullptr
  > Variant& operator=(const Q& arg)
  {
    Variant tmp(arg);
    tweak_variant_swap(&tv_, &tmp.tv_);
    return *this;
  }

  /**
   * Extract scalar value.
   *
   * @return tuple comprising of scalar value and bool result of an operation.
   */
  template<typename Q> std::optional<Q> getScalar() const {
    switch (tv_.type) {
    case TWEAK_VARIANT_TYPE_BOOL:
      return static_cast<Q>(tv_.value.b);
    case TWEAK_VARIANT_TYPE_SINT8:
      return static_cast<Q>(tv_.value.sint8);
    case TWEAK_VARIANT_TYPE_SINT16:
      return static_cast<Q>(tv_.value.sint16);
    case TWEAK_VARIANT_TYPE_SINT32:
      return static_cast<Q>(tv_.value.sint32);
    case TWEAK_VARIANT_TYPE_SINT64:
      return static_cast<Q>(tv_.value.sint64);
    case TWEAK_VARIANT_TYPE_UINT8:
      return static_cast<Q>(tv_.value.uint8);
    case TWEAK_VARIANT_TYPE_UINT16:
      return static_cast<Q>(tv_.value.uint16);
    case TWEAK_VARIANT_TYPE_UINT32:
      return static_cast<Q>(tv_.value.uint32);
    case TWEAK_VARIANT_TYPE_UINT64:
      return static_cast<Q>(tv_.value.uint64);
    case TWEAK_VARIANT_TYPE_FLOAT:
      return static_cast<Q>(tv_.value.fp32);
    case TWEAK_VARIANT_TYPE_DOUBLE:
      return static_cast<Q>(tv_.value.fp64);
    default:
      return std::nullopt;
    }
  }

  /**
   * Access const vector data.
   *
   * @return tuple comprising of buffer pointer and bool result of an operation.
   */
  template<typename Q> std::optional<const Q*> getBuffer() const {
    switch (tv_.type) {
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
      return static_cast<const Q*>(tweak_buffer_get_data_const(&tv_.value.buffer));
    default:
      return std::nullopt;
    }
  }

  /**
   * Access vector data.
   *
   * @return tuple comprising of buffer pointer and bool result of an operation.
   */
  template<typename Q> std::optional<Q*> getBuffer() {
    switch (tv_.type) {
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
      return static_cast<Q*>(tweak_buffer_get_data(&tv_.value.buffer));
    default:
      return std::nullopt;
    }
  }

  /**
   * Get vector size.
   *
   * @return size of buffer.
   */
  template<typename Q> std::size_t getBufferSize() const {
    switch (tv_.type) {
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
      return tweak_buffer_get_size(&tv_.value.buffer) / sizeof(Q);
    default:
      return 0UL;
    }
  }
};

}

#endif // TWEAK_VARIANT_H_INCLUDED
