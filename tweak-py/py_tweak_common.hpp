/**
 * @file py_tweak_common.hpp
 * @ingroup tweak-py
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
 * @defgroup tweak-py Tweak Python API
 * Part of internal library API.
 */

#ifndef PY_TWEAK_COMMON
#define PY_TWEAK_COMMON

#include <tweak2/variant.h>
#include <tweak2/appcommon.h>
#include <tweak2/metadata.h>
#include "gil_loop.hpp"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <string>
#include <exception>
#include <functional>
#include <vector>
#include <sstream>

namespace py = pybind11;

namespace tweak2
{
std::string variantTypeToString(tweak_variant_type arg);

py::object convertFromTweak(const tweak_variant* value);

tweak_variant convertScalarToTweak(const py::object& value, tweak_variant_type type);

using TimeoutMillis = uint64_t;

constexpr TimeoutMillis DEFAULT_WAIT_TIMEOUT = 10000;

class PyTweakException: public std::exception
{
public:
    /** Constructor (C strings).
     *  @param message C-style string error message.
     *                 The string contents are copied upon construction.
     *                 Hence, responsibility for deleting the char* lies
     *                 with the caller.
     */
    explicit PyTweakException(const char* message)
        : msg_(message) {}

    /** Constructor (C++ STL strings).
     *  @param message The error message.
     */
    explicit PyTweakException(const std::string& message)
        : msg_(message) {}

    /** Destructor.
     * Virtual to allow for subclassing.
     */
    virtual ~PyTweakException() noexcept {}

    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a const char*. The underlying memory
     *          is in posession of the Exception object. Callers must
     *          not attempt to free the memory.
     */
    virtual const char* what() const noexcept {
       return msg_.c_str();
    }

protected:
    /** Error message.
     */
    std::string msg_;
};

static inline bool typeHasDataLayout(tweak_variant_type arg) {
  switch (arg) {
  case TWEAK_VARIANT_TYPE_NULL:
    throw PyTweakException("Unexpected null type");
  case TWEAK_VARIANT_TYPE_BOOL:
  case TWEAK_VARIANT_TYPE_SINT8:
  case TWEAK_VARIANT_TYPE_SINT16:
  case TWEAK_VARIANT_TYPE_SINT32:
  case TWEAK_VARIANT_TYPE_SINT64:
  case TWEAK_VARIANT_TYPE_UINT8:
  case TWEAK_VARIANT_TYPE_UINT16:
  case TWEAK_VARIANT_TYPE_UINT32:
  case TWEAK_VARIANT_TYPE_UINT64:
  case TWEAK_VARIANT_TYPE_FLOAT:
  case TWEAK_VARIANT_TYPE_DOUBLE:
  case TWEAK_VARIANT_TYPE_STRING:
    return false;
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
    return true;
  }
  throw PyTweakException(std::string("Unknown type: ") + std::to_string(arg));
}

tweak_variant_type getVariantType(const py::buffer_info &info);

size_t getSize(const py::buffer_info &info);

VariantGuard convertPyBuffer(const py::buffer &buffer, MetadataGuard metadata);

class Buffer {
  VariantGuard variant_;
  MetadataGuard metadata_;
  template<typename T> py::buffer_info makeBufferInfo(Buffer& b);
public:
  Buffer() = default;
  Buffer(VariantGuard&& variant, MetadataGuard&& metadata)
    : variant_(std::move(variant))
    , metadata_(std::move(metadata))
  {}

  Buffer(const Buffer& o)
    : variant_(o.variant_.copy())
    , metadata_(tweak_metadata_copy(o.metadata_.get()))
  {}

  ~Buffer() = default;

  Buffer& operator=(const Buffer& o){
    Buffer tmp(o);
    swap(tmp);
    return *this;
  }

  void swap(Buffer& o) noexcept(true) {
    variant_.swap(o.variant_);
    metadata_.swap(o.metadata_);
  }

  py::buffer_info makeBufferInfo();
};

class TweakBase {
protected:
  GilLoop gilLoop;

  VariantGuard convertFromPyObject(tweak_id id, const py::object& arg) const;

  py::object convertToPyObject(tweak_id id, VariantGuard&& arg) const;

  VariantGuard copyCurrentValue(tweak_id id) const;

  MetadataGuard copyMetadata(tweak_id id) const;

  void replaceCurrentValue(tweak_id id, VariantGuard &&arg);
public:
  void set(tweak_id id, const py::object& value);

  void set(const std::string &uri, const py::object& value, TimeoutMillis timeout);

  void set(const std::string &uri, const py::object& value)
  {
    set(uri, value, DEFAULT_WAIT_TIMEOUT);
  }

  py::object get(tweak_id id);

  py::object get(const std::string &uri, TimeoutMillis timeout);

  py::object get(const std::string &uri)
  {
    return get(uri, DEFAULT_WAIT_TIMEOUT);
  }

  virtual tweak_id find(const std::string& uri, TimeoutMillis timeout) = 0;

  virtual tweak_app_context getContext() const = 0;

  virtual ~TweakBase() = default;
};

const char* translate_app_error_code(tweak_app_error_code arg);

} // namespace tweak2

#endif
