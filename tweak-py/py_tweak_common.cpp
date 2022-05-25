/**
 * @file py_tweak_common.cpp
 * @ingroup tweak-external-interfaces
 *
 * @brief Tweak 2 Python 3 bindings.
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

#include "py_tweak_common.hpp"

#include <tweak2/tweak2.h>
#include <tweak2/metadata.h>
#include <tweak2/appclient.h>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <unordered_map>
#include <stdint.h>

namespace {
template<typename Q>
Q* element_ptr(const py::buffer_info &info, ssize_t offset) {
  return reinterpret_cast<Q*>(static_cast<char*>(info.ptr) + offset);
}

struct flatten_context {
  size_t cur_index;
  std::vector<ssize_t> indices;
  std::vector<ssize_t> dim_factors;
};

ssize_t calculate_offset(const std::vector<ssize_t> &strides, const std::vector<ssize_t> &indices) {
  ssize_t acc = 0;
  for (size_t ix = 0; ix < strides.size(); ++ix) {
    acc += strides[ix] * indices[ix];
  }
  return acc;
}

static ssize_t calculate_array_index(const std::vector<ssize_t> &dim_factors,
    const std::vector<ssize_t> &indices)
{
  ssize_t acc = 0;
  for (size_t ix = 0; ix < dim_factors.size(); ++ix) {
    acc += dim_factors[ix] * indices[ix];
  }
  return acc;
}

template<typename T>
void flatten(T* out, struct flatten_context &context, const py::buffer_info &info) {
  ssize_t upper_bound = info.shape[context.cur_index];
  for (ssize_t ix = 0; ix < upper_bound; ix++) {
    context.indices[context.cur_index] = ix;
    if ((info.ndim - 1) == context.cur_index) {
      ssize_t offset = calculate_offset(info.strides, context.indices);
      if (offset < 0 ) {
          throw std::runtime_error("Negative index");
      }
      ssize_t index = calculate_array_index(context.dim_factors, context.indices);
      out[index] = *element_ptr<T>(info, offset);
    } else {
      ++context.cur_index;
      flatten<T>(out, context, info);
      --context.cur_index;
    }
  }
}

std::vector<ssize_t> make_offset_factors_map(const py::buffer_info &info) {
  std::vector<ssize_t> result(info.ndim);
  for (ssize_t dim = 0; dim < info.ndim; dim++) {
    ssize_t product = 1;
    for (ssize_t ix = 0; ix < dim; ix++) {
      product *= info.shape[info.ndim - ix - 1];
    }
    result[info.ndim - dim - 1] = product;
  }
  return result;
}

void validate_metadata(tweak_metadata metadata, const py::buffer_info &info) {
  tweak_metadata_layout layout = tweak_metadata_get_layout(metadata);
  if (layout == NULL) {
    throw tweak2::PyTweakException("Metadata lacks layout");
  }

  ssize_t numDimensions = static_cast<ssize_t>(tweak_metadata_layout_get_number_of_dimensions(layout));
  if (numDimensions != info.ndim) {
    std::ostringstream ss;
    ss << "Dimensions disagree: metadata has " << numDimensions << ", specified array has " << info.ndim;
    throw tweak2::PyTweakException(ss.str());
  }

  for (size_t ix = 0; ix < numDimensions; ++ix) {
    if (tweak_metadata_layout_get_dimension(layout, ix) != info.shape[ix]) {
      std::ostringstream ss;
      ss << "Dimension disagree: metadata has " << tweak_metadata_layout_get_dimension(layout, ix)
         << ", specified array has " << info.shape[ix];
      throw tweak2::PyTweakException(ss.str());
    }
  }
}

}

namespace tweak2 {

size_t getSize(const py::buffer_info &info) {
  ssize_t result = 1;
  for (ssize_t dim = 0; dim < info.ndim; dim++) {
    result *= info.shape[dim];
  }
  return static_cast<size_t>(result);
}

/* Based on https://docs.python.org/3/library/struct.html */

bool is_floating(const std::string &arg) {
  return arg == "f" || arg == "d";
}

bool is_unsigned(const std::string &arg) {
  return arg == "B" || arg == "H" || arg == "I" || arg == "Q" || arg == "L" || arg == "N";
}

tweak_variant_type getVariantType(const py::buffer_info &info) {
  switch (info.itemsize) {
  case 1:
    return is_unsigned(info.format) ? TWEAK_VARIANT_TYPE_VECTOR_UINT8 : TWEAK_VARIANT_TYPE_VECTOR_SINT8;
  case 2:
    return is_unsigned(info.format) ? TWEAK_VARIANT_TYPE_VECTOR_UINT16 : TWEAK_VARIANT_TYPE_VECTOR_SINT16;
  case 4:
    if (is_floating(info.format)) {
      return TWEAK_VARIANT_TYPE_VECTOR_FLOAT;
    } else {
      return is_unsigned(info.format)
        ? TWEAK_VARIANT_TYPE_VECTOR_UINT32
        : TWEAK_VARIANT_TYPE_VECTOR_SINT32;
    }
  case 8:
    if (is_floating(info.format)) {
      return TWEAK_VARIANT_TYPE_VECTOR_DOUBLE;
    } else {
      return is_unsigned(info.format)
        ? TWEAK_VARIANT_TYPE_VECTOR_UINT64
        : TWEAK_VARIANT_TYPE_VECTOR_SINT64;
    }
  }
  throw tweak2::PyTweakException(std::string("Unrecognized item size : ") + std::to_string(info.itemsize));
}

VariantGuard convertPyBuffer(const py::buffer &buffer, MetadataGuard metadata) {
  VariantGuard result;
  py::buffer_info info = buffer.request();
  validate_metadata(metadata.get(), info);
  tweak_variant_type variant_type = getVariantType(info);
  flatten_context flatten_context;
  flatten_context.indices.resize(info.ndim);
  flatten_context.cur_index = 0;
  flatten_context.dim_factors = make_offset_factors_map(info);
  switch(variant_type) {
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    tweak_variant_assign_sint8_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<int8_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    tweak_variant_assign_sint16_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<int16_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    tweak_variant_assign_sint32_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<int32_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    tweak_variant_assign_sint64_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<int64_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    tweak_variant_assign_uint8_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<uint8_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    tweak_variant_assign_uint16_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<uint16_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    tweak_variant_assign_uint32_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<uint32_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    tweak_variant_assign_uint64_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<uint64_t*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    tweak_variant_assign_float_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<float*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    tweak_variant_assign_double_vector(&result.get(), NULL, getSize(info));
    flatten(static_cast<double*>(tweak_buffer_get_data(&result.get().value.buffer)), flatten_context, info);
    break;
  default:
    throw tweak2::PyTweakException(std::string("Unrecognized type : ") + info.format);
  }
  return result;
}

template<typename T> py::buffer_info Buffer::makeBufferInfo(Buffer& b) {
  tweak_metadata_layout layout = tweak_metadata_get_layout(b.metadata_.get());
  if (!layout) {
    throw tweak2::PyTweakException("Buffer instance lacks layout");
  }
  ssize_t numDimensions = static_cast<ssize_t>(tweak_metadata_layout_get_number_of_dimensions(layout));
  std::vector<ssize_t> shape(numDimensions);
  std::vector<ssize_t> strides(numDimensions);
  for (ssize_t ix = 0; ix < numDimensions; ix++) {
    shape[ix] = static_cast<ssize_t>(tweak_metadata_layout_get_dimension(layout, ix));
    size_t acc = 1;
    for (size_t iy = 1; iy <= ix; iy++) {
      acc *= shape[iy];
    }
    strides[numDimensions - ix - 1] = acc * sizeof(T);
  }

  return py::buffer_info(
    tweak_buffer_get_data(&b.variant_.get().value.buffer), /* Pointer to buffer */
    sizeof(T),                                             /* Size of one scalar */
    py::format_descriptor<T>::format(),                    /* Python struct-style format descriptor */
    numDimensions,                                         /* Number of dimensions */
    shape,                                                 /* Buffer dimensions */
    strides);                                              /* Strides (in bytes) for each index */
}

py::buffer_info Buffer::makeBufferInfo() {
  switch (variant_.get().type) {
  case TWEAK_VARIANT_TYPE_NULL:
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
    throw py::value_error("Not a vector type");
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return Buffer::makeBufferInfo<int8_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return Buffer::makeBufferInfo<int16_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return Buffer::makeBufferInfo<int32_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return Buffer::makeBufferInfo<int64_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return Buffer::makeBufferInfo<uint8_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return Buffer::makeBufferInfo<uint16_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return Buffer::makeBufferInfo<uint32_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return Buffer::makeBufferInfo<uint64_t>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return Buffer::makeBufferInfo<float>(*this);
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return Buffer::makeBufferInfo<double>(*this);
  }
  throw py::value_error("Unexpected type");
}

std::string variantTypeToString(tweak_variant_type arg) {
  switch (arg) {
  case TWEAK_VARIANT_TYPE_NULL:
    return "TWEAK_VARIANT_TYPE_NULL";
  case TWEAK_VARIANT_TYPE_BOOL:
    return "TWEAK_VARIANT_TYPE_BOOL";
  case TWEAK_VARIANT_TYPE_SINT8:
    return "TWEAK_VARIANT_TYPE_SINT8";
  case TWEAK_VARIANT_TYPE_SINT16:
    return "TWEAK_VARIANT_TYPE_SINT16";
  case TWEAK_VARIANT_TYPE_SINT32:
    return "TWEAK_VARIANT_TYPE_SINT32";
  case TWEAK_VARIANT_TYPE_SINT64:
    return "TWEAK_VARIANT_TYPE_SINT64";
  case TWEAK_VARIANT_TYPE_UINT8:
    return "TWEAK_VARIANT_TYPE_UINT8";
  case TWEAK_VARIANT_TYPE_UINT16:
    return "TWEAK_VARIANT_TYPE_UINT16";
  case TWEAK_VARIANT_TYPE_UINT32:
    return "TWEAK_VARIANT_TYPE_UINT32";
  case TWEAK_VARIANT_TYPE_UINT64:
    return "TWEAK_VARIANT_TYPE_UINT64";
  case TWEAK_VARIANT_TYPE_FLOAT:
    return "TWEAK_VARIANT_TYPE_FLOAT";
  case TWEAK_VARIANT_TYPE_DOUBLE:
    return "TWEAK_VARIANT_TYPE_DOUBLE";
  case TWEAK_VARIANT_TYPE_STRING:
    return "TWEAK_VARIANT_TYPE_STRING";
  case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    return "TWEAK_VARIANT_TYPE_VECTOR_SINT8";
  case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    return "TWEAK_VARIANT_TYPE_VECTOR_SINT16";
  case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    return "TWEAK_VARIANT_TYPE_VECTOR_SINT32";
  case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    return "TWEAK_VARIANT_TYPE_VECTOR_SINT64";
  case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    return "TWEAK_VARIANT_TYPE_VECTOR_UINT8";
  case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    return "TWEAK_VARIANT_TYPE_VECTOR_UINT16";
  case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    return "TWEAK_VARIANT_TYPE_VECTOR_UINT32";
  case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    return "TWEAK_VARIANT_TYPE_VECTOR_UINT64";
  case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    return "TWEAK_VARIANT_TYPE_VECTOR_FLOAT";
  case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
    return "TWEAK_VARIANT_TYPE_VECTOR_DOUBLE";
  }
  std::stringstream ss;
  ss << "Unknown type: " << arg;
  return ss.str();
}

py::object convertFromTweak(const tweak_variant* value) {
  switch (value->type) {
  case TWEAK_VARIANT_TYPE_BOOL:
    return py::bool_(value->value.b);
  case TWEAK_VARIANT_TYPE_SINT8:
    return py::int_(value->value.sint8);
  case TWEAK_VARIANT_TYPE_SINT16:
    return py::int_(value->value.sint16);
  case TWEAK_VARIANT_TYPE_SINT32:
    return py::int_(value->value.sint32);
  case TWEAK_VARIANT_TYPE_SINT64:
    return py::int_(value->value.sint64);
  case TWEAK_VARIANT_TYPE_UINT8:
    return py::int_(value->value.uint8);
  case TWEAK_VARIANT_TYPE_UINT16:
    return py::int_(value->value.uint16);
  case TWEAK_VARIANT_TYPE_UINT32:
    return py::int_(value->value.uint32);
  case TWEAK_VARIANT_TYPE_UINT64:
    return py::int_(value->value.uint64);
  case TWEAK_VARIANT_TYPE_FLOAT:
    return py::float_(value->value.fp32);
  case TWEAK_VARIANT_TYPE_DOUBLE:
    return py::float_(value->value.fp64);
  case TWEAK_VARIANT_TYPE_STRING:
    return py::str(tweak_variant_string_c_str(&value->value.string));
  default:
    throw py::value_error(std::string("Unsupported type:") + variantTypeToString(value->type));
  }
}

tweak_variant convertScalarToTweak(const py::object& value, tweak_variant_type type) {
  tweak_variant retVal = TWEAK_VARIANT_INIT_EMPTY;
  switch (type) {
  case TWEAK_VARIANT_TYPE_BOOL:
    tweak_variant_assign_bool(&retVal, value.cast<bool>());
    break;
  case TWEAK_VARIANT_TYPE_SINT8:
    tweak_variant_assign_sint8(&retVal, value.cast<int8_t>());
    break;
  case TWEAK_VARIANT_TYPE_SINT16:
    tweak_variant_assign_sint16(&retVal, value.cast<int16_t>());
    break;
  case TWEAK_VARIANT_TYPE_SINT32:
    tweak_variant_assign_sint32(&retVal, value.cast<int32_t>());
    break;
  case TWEAK_VARIANT_TYPE_SINT64:
    tweak_variant_assign_sint64(&retVal, value.cast<int64_t>());
    break;
  case TWEAK_VARIANT_TYPE_UINT8:
    tweak_variant_assign_uint8(&retVal, value.cast<uint8_t>());
    break;
  case TWEAK_VARIANT_TYPE_UINT16:
    tweak_variant_assign_uint16(&retVal, value.cast<uint16_t>());
    break;
  case TWEAK_VARIANT_TYPE_UINT32:
    tweak_variant_assign_uint32(&retVal, value.cast<uint32_t>());
    break;
  case TWEAK_VARIANT_TYPE_UINT64:
    tweak_variant_assign_uint64(&retVal, value.cast<uint64_t>());
    break;
  case TWEAK_VARIANT_TYPE_FLOAT:
    tweak_variant_assign_float(&retVal, value.cast<float>());
    break;
  case TWEAK_VARIANT_TYPE_DOUBLE:
    tweak_variant_assign_double(&retVal, value.cast<double>());
    break;
  case TWEAK_VARIANT_TYPE_STRING:
    tweak_variant_assign_string(&retVal, value.cast<std::string>().c_str());
    break;
  default:
    throw py::value_error(std::string("Unsupported type:") + tweak2::variantTypeToString(type));
  }
  return retVal;
}

const char* translate_app_error_code(tweak_app_error_code arg) {
    switch (arg) {
    case TWEAK_APP_SUCCESS:
        return "TWEAK_APP_SUCCESS";
    case TWEAK_APP_SUCCESS_LAST_KNOWN_VALUE:
        return "TWEAK_APP_SUCCESS_LAST_KNOWN_VALUE";
    case TWEAK_APP_ITEM_NOT_FOUND:
        return "TWEAK_APP_ITEM_NOT_FOUND";
    case TWEAK_APP_TYPE_MISMATCH:
        return "TWEAK_APP_TYPE_MISMATCH";
    case TWEAK_APP_INVALID_ARGUMENT:
        return "TWEAK_APP_INVALID_ARGUMENT";
    case TWEAK_APP_PEER_DISCONNECTED:
        return "TWEAK_APP_PEER_DISCONNECTED";
    }
    TWEAK_FATAL("Unknown app error code: %d", arg);
    return NULL;
}

VariantGuard TweakBase::convertFromPyObject(tweak_id id, const py::object& arg) const {
  tweak_variant_type type = tweak_app_item_get_type(getContext(), id);
  if (type == TWEAK_VARIANT_TYPE_NULL) {
    throw py::key_error(std::string("Bad tweak id"));
  }

  tweak_app_error_code error_code;
  VariantGuard result;
  if (tweak2::typeHasDataLayout(type)) {
    py::buffer buffer = py::cast<py::buffer>(arg);
    py::buffer_info info = buffer.request();
    tweak_metadata tmp = NULL;
    error_code = tweak_app_item_get_metadata(getContext(), id, &tmp);
    if (error_code != TWEAK_APP_SUCCESS) {
      throw tweak2::PyTweakException(
        std::string("tweak_app_item_get_metadata() returned error: ")
          + translate_app_error_code(error_code));
    }
    MetadataGuard meta(tmp);
    result = convertPyBuffer(buffer, std::move(meta));
  } else {
    tweak_variant tmp = tweak2::convertScalarToTweak(arg, type);
    result = VariantGuard(&tmp);
  }
  return std::move(result);
}

VariantGuard TweakBase::copyCurrentValue(tweak_id id) const {
  VariantGuard value;
  tweak_app_error_code error_code;
  error_code = tweak_app_item_clone_current_value(getContext(), id, &value.get());
  if (error_code != TWEAK_APP_SUCCESS) {
    throw tweak2::PyTweakException(
      std::string("tweak_app_item_clone_current_value() returned error: ")
        + translate_app_error_code(error_code));
  }
  return std::move(value);
}

MetadataGuard TweakBase::copyMetadata(tweak_id id) const {
  tweak_metadata tmp = NULL;
  tweak_app_error_code error_code;
  error_code = tweak_app_item_get_metadata(getContext(), id, &tmp);
  if (error_code != TWEAK_APP_SUCCESS) {
      throw tweak2::PyTweakException(std::string("ERROR: Internal tweak error"));
  }
  return MetadataGuard(tmp);
}

void TweakBase::replaceCurrentValue(tweak_id id, VariantGuard &&arg) {
  VariantGuard tmp = std::move(arg);
  tweak_app_error_code error_code =
    tweak_app_item_replace_current_value(getContext(), id, &tmp.get());
  if (error_code != TWEAK_APP_SUCCESS) {
    throw tweak2::PyTweakException(
      std::string("tweak_app_item_replace_current_value() returned error: ")
        + translate_app_error_code(error_code));
  }
}

void TweakBase::set(tweak_id id, const py::object& value) {
  replaceCurrentValue(id, convertFromPyObject(id, value));
}

void TweakBase::set(const std::string &uri, const py::object& value, tweak2::TimeoutMillis timeout) {
  tweak_id id = find(uri, timeout);
  if (id != TWEAK_INVALID_ID) {
    set(id, value);
  } else {
      throw py::index_error(uri);
  }
}

py::object TweakBase::convertToPyObject(tweak_id id, VariantGuard&& arg) const {
    VariantGuard value = std::move(arg);
    py::object retVal;
    if (tweak2::typeHasDataLayout(value.type())) {
      tweak_metadata tmp = NULL;
      tweak_app_error_code error_code = tweak_app_item_get_metadata(getContext(), id, &tmp);
      MetadataGuard metadata(tmp);
      if (error_code != TWEAK_APP_SUCCESS) {
        throw tweak2::PyTweakException(
            std::string("tweak_app_item_clone_current_value() returned error: ")
                + translate_app_error_code(error_code));
      }
      retVal = py::cast(tweak2::Buffer(std::move(value), std::move(metadata)));
    } else {
      retVal = tweak2::convertFromTweak(&value.get());
    }
    return retVal;
}

py::object TweakBase::get(tweak_id id) {
  return convertToPyObject(id, copyCurrentValue(id));
}

py::object TweakBase::get(const std::string &uri, tweak2::TimeoutMillis timeout) {
  tweak_id id = find(uri, timeout);
  if (id != TWEAK_INVALID_ID) {
      return get(id);
  } else {
      throw py::index_error(uri);
  }
}

}
