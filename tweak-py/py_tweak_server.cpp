/**
 * @file py_tweak_server.cpp
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

#include "gil_loop.hpp"

#include <tweak2/metadata.h>
#include <tweak2/appclient.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <unordered_map>

#include "py_tweak_server.hpp"

namespace tweak2 {

namespace {
std::unique_ptr<TweakServer> theTweakServer;
};

TweakServer::TweakServer()
    :server_context(NULL)
{}

TweakServer::TweakServer(const std::string &context_type,
    const std::string &params, const std::string &uri)
{
    tweak_app_server_callbacks server_callbacks = {
        .cookie = this,
        .on_current_value_changed = &current_value_changed_callback_tweak_app
    };

    server_context = tweak_app_create_server_context(context_type.c_str(), params.c_str(),
        uri.c_str(), &server_callbacks);
}

TweakServer::~TweakServer() {
    if (server_context != NULL) {
        tweak_app_destroy_context(server_context);
    }
}

tweak_id TweakServer::addItem(const std::string& uri, const std::string& description,
    const std::string& meta, VariantGuard&& value, ListenerGuard listener)
{
    VariantGuard tmp = std::move(value);
    tweak_id result = tweak_app_server_add_item(getContext(),
        uri.c_str(), description.c_str(), meta.c_str(), &tmp.get(), listener.get());

    if (result == TWEAK_INVALID_ID) {
        throw tweak2::PyTweakException(std::string("ERROR: Internal tweak error"));
    }

    listeners[result] = std::move(listener);
    return result;
}

void TweakServer::current_value_changed_callback_tweak_app(tweak_app_context context,
    tweak_id id, tweak_variant* value, void *cookie)
{
    (void) context;
    (void) value;
    static_cast<TweakServer*>(cookie)->on_value_changed(id);
}

void TweakServer::current_value_changed_callback_tweak2h(tweak_id id, void* cookie)
{
    static_cast<TweakServer*>(cookie)->on_value_changed(id);
}

tweak_app_context TweakServer::getContext() const {
    return server_context
        ? server_context
        : (tweak_app_server_context) tweak_get_default_server_instance();
}

void TweakServer::on_value_changed(tweak_id id) {
    VariantGuard variant = copyCurrentValue(id);
    tweak_app_error_code error_code;
    MetadataGuard metadata;
    if (tweak2::typeHasDataLayout(variant.type())) {
        metadata = copyMetadata(id);
    }
    gilLoop.feed(id, std::move(variant), std::move(metadata));
}

tweak_id TweakServer::find(const std::string& uri) {
    return find(uri, tweak2::DEFAULT_WAIT_TIMEOUT);
}

tweak_id TweakServer::find(const std::string& uri, tweak2::TimeoutMillis timeout) {
    (void) timeout;
    return tweak_app_find_id(getContext(), uri.c_str());
}

tweak_id TweakServer::add_bool(const std::string &uri,
    const std::string &description, const std::string &meta,
    bool initial_value, ItemChangedCallback callback)
{
    tweak_variant variant = TWEAK_VARIANT_INIT_EMPTY;
    tweak_variant_assign_bool(&variant, initial_value);

    ListenerGuard listener =
        ListenerGuard(tweak_create_default_client_item_changed_listener(&current_value_changed_callback_tweak2h, this));

    tweak_id id = addItem(uri, description, meta, VariantGuard(&variant), std::move(listener));
    gilLoop.setHandler(id, callback);
    return id;
}

tweak_id TweakServer::add_int(const std::string &uri, const std::string &description,
                              const std::string &meta, int64_t initial_value, ItemChangedCallback callback)
{
    tweak_variant variant = TWEAK_VARIANT_INIT_EMPTY;

    ListenerGuard listener =
        ListenerGuard(tweak_create_default_client_item_changed_listener(&current_value_changed_callback_tweak2h, this));

    tweak_variant_assign_sint64(&variant, initial_value);
    tweak_id id = addItem(uri, description, meta, VariantGuard(&variant), std::move(listener));
    gilLoop.setHandler(id, callback);
    return id;
}

tweak_id TweakServer::add_float(const std::string &uri, const std::string &description,
                                const std::string &meta, double initial_value, ItemChangedCallback callback)
{
    tweak_variant variant = TWEAK_VARIANT_INIT_EMPTY;
    tweak_variant_assign_double(&variant, initial_value);

    ListenerGuard listener =
        ListenerGuard(tweak_create_default_client_item_changed_listener(&current_value_changed_callback_tweak2h, this));

    tweak_id id = addItem(uri, description, meta, VariantGuard(&variant), std::move(listener));
    gilLoop.setHandler(id, callback);
    return id;
}

tweak_id TweakServer::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    bool initial_value, ItemChangedCallback callback)
{
    return add_bool(uri, description, meta, initial_value, callback);
}

tweak_id TweakServer::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    int64_t initial_value, ItemChangedCallback callback)
{
    return add_int(uri, description, meta, initial_value, callback);;
}

tweak_id TweakServer::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    double initial_value, ItemChangedCallback callback)
{
    return add_float(uri, description, meta, initial_value, callback);
}

tweak_id TweakServer::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    const std::string &initial_value, ItemChangedCallback callback)
{
    tweak_variant variant = TWEAK_VARIANT_INIT_EMPTY;
    tweak_variant_assign_string(&variant, initial_value.c_str());
    ListenerGuard listener =
        ListenerGuard(tweak_create_default_client_item_changed_listener(&current_value_changed_callback_tweak2h, this));

    tweak_id id = addItem(uri, description, meta, VariantGuard(&variant), std::move(listener));

    gilLoop.setHandler(id, callback);
    return id;
}

tweak_id TweakServer::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    const py::object &initial_value, ItemChangedCallback callback)
{
    py::buffer buffer = py::cast<py::buffer>(initial_value);
    py::buffer_info info = buffer.request();

    tweak2::ListenerGuard listener =
        tweak2::ListenerGuard(
            tweak_create_default_client_item_changed_listener(&current_value_changed_callback_tweak2h, this));

    VariantGuard value = convertPyBuffer(buffer,
        MetadataGuard(tweak_metadata_create(getVariantType(info), getSize(info), meta.c_str())));

    tweak_id id = addItem(uri, description, meta, std::move(value), std::move(listener));

    gilLoop.setHandler(id, callback);
    return id;
}

void TweakServer::set_item_value(tweak_id id, bool value)
{
    tweak_app_error_code error_code;
    VariantGuard variant;
    tweak_variant_assign_bool(&variant.get(), value);
    replaceCurrentValue(id, std::move(variant));
}

void TweakServer::set_item_value(tweak_id id, int64_t value)
{
    tweak_app_error_code error_code;
    VariantGuard variant;
    tweak_variant_assign_sint64(&variant.get(), value);
    replaceCurrentValue(id, std::move(variant));
}

void TweakServer::set_item_value(tweak_id id, double value)
{
    tweak_app_error_code error_code;
    VariantGuard variant;
    tweak_variant_assign_double(&variant.get(), value);
    replaceCurrentValue(id, std::move(variant));
}

bool TweakServer::get_item_bool_value(tweak_id id)
{
    tweak_app_error_code error_code;
    VariantGuard variant = copyCurrentValue(id);
    if (variant.type() != TWEAK_VARIANT_TYPE_BOOL) {
        throw tweak2::PyTweakException("ERROR: type mismatch");
    }
    return variant.get().value.b;
}

int64_t TweakServer::get_item_int_value(tweak_id id)
{
    tweak_app_error_code error_code;
    VariantGuard variant = copyCurrentValue(id);
    if (variant.type() != TWEAK_VARIANT_TYPE_SINT64) {
        throw tweak2::PyTweakException("ERROR: type mismatch");
    }
    return variant.get().value.sint64;
}

double TweakServer::get_item_float_value(tweak_id id)
{
    tweak_app_error_code error_code;
    VariantGuard variant = copyCurrentValue(id);
    if (variant.type() != TWEAK_VARIANT_TYPE_DOUBLE) {
        throw tweak2::PyTweakException("ERROR: type mismatch");
    }
    return variant.get().value.fp64;
}

void TweakServer::set_item_value(const std::string &uri, bool value)
{
    tweak_id id = tweak_app_find_id(getContext(), uri.c_str());
    if (id != TWEAK_INVALID_ID) {
        set_item_value(id, value);
    }
}

void TweakServer::set_item_value(const std::string &uri, int64_t value)
{
    tweak_id id = tweak_app_find_id(getContext(), uri.c_str());
    if (id != TWEAK_INVALID_ID) {
        set_item_value(id, value);
    }
}

void TweakServer::set_item_value(const std::string &uri, double value)
{
    tweak_id id = tweak_app_find_id(getContext(), uri.c_str());
    if (id != TWEAK_INVALID_ID) {
        set_item_value(id, value);
    }
}

bool TweakServer::get_item_bool_value(const std::string &uri)
{
    tweak_id id = tweak_app_find_id(getContext(), uri.c_str());
    if (id != TWEAK_INVALID_ID) {
        return get_item_bool_value(id);
    } else {
        throw py::index_error(uri);
    }
}

int64_t TweakServer::get_item_int_value(const std::string &uri)
{
    tweak_id id = tweak_app_find_id(getContext(), uri.c_str());
    if (id != TWEAK_INVALID_ID) {
        return get_item_int_value(id);
    } else {
        throw py::index_error(uri);
    }
}

double TweakServer::get_item_float_value(const std::string &uri)
{
    tweak_id id = tweak_app_find_id(getContext(), uri.c_str());
    if (id != TWEAK_INVALID_ID) {
        return get_item_float_value(id);
    } else {
        throw py::index_error(uri);
    }
}

void TweakServer::remove(tweak_id id)
{
    if (!tweak_app_server_remove_item(getContext(), id)) {
        throw py::index_error(std::string("Unknown id :") + std::to_string(id));
    }
    gilLoop.removeHandler(id);
    listeners.erase(id);
}

void TweakServerSingleton::initialize_library(const std::string &context_type,
    const std::string &params, const std::string &uri)
{
    tweak_initialize_library(context_type.c_str(), params.c_str(), uri.c_str());
    theTweakServer = std::unique_ptr<TweakServer>(new TweakServer);
}

tweak_id TweakServerSingleton::add_bool(const std::string &uri,
    const std::string &description, const std::string &meta,
    bool initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add_bool(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::add_int(const std::string &uri,
    const std::string &description, const std::string &meta,
    int64_t initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add_int(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::add_float(const std::string &uri,
    const std::string &description, const std::string &meta,
    double initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add_float(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    bool initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add_bool(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    int64_t initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add_int(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    double initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add_float(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    const std::string &initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::add(const std::string &uri,
    const std::string &description, const std::string &meta,
    const py::object &initial_value, ItemChangedCallback callback)
{
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->add(uri, description, meta, initial_value, callback);
}

tweak_id TweakServerSingleton::find(const std::string &uri) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->find(uri, tweak2::DEFAULT_WAIT_TIMEOUT);
}

bool TweakServerSingleton::get_item_bool_value(tweak_id id) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->get_item_bool_value(id);
}

bool TweakServerSingleton::get_item_bool_value(const std::string &uri) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->get_item_bool_value(uri);
}

int64_t TweakServerSingleton::get_item_int_value(tweak_id id) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->get_item_int_value(id);
}

int64_t TweakServerSingleton::get_item_int_value(const std::string &uri) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->get_item_int_value(uri);
}

double TweakServerSingleton::get_item_float_value(tweak_id id) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->get_item_float_value(id);
}

double TweakServerSingleton::get_item_float_value(const std::string &uri) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->get_item_float_value(uri);
}

py::object TweakServerSingleton::get(tweak_id id) {
    return theTweakServer->get(id);
}

py::object TweakServerSingleton::get(const std::string &uri) {
    return theTweakServer->get(uri);
}

void TweakServerSingleton::set_item_value(tweak_id id, bool value) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->set_item_value(id, value);
}

void TweakServerSingleton::set_item_value(const std::string &uri, bool value) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->set_item_value(uri, value);
}

void TweakServerSingleton::set_item_value(tweak_id id, int64_t value) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->set_item_value(id, value);
}

void TweakServerSingleton::set_item_value(const std::string &uri, int64_t value) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->set_item_value(uri, value);
}

void TweakServerSingleton::set_item_value(tweak_id id, double value) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->set_item_value(id, value);
}

void TweakServerSingleton::set_item_value(const std::string &uri, double value){
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->set_item_value(uri, value);
}

void TweakServerSingleton::set(tweak_id id, const py::object &value) {
    return theTweakServer->set(id, value);
}

void TweakServerSingleton::set(const std::string &uri, const py::object &value) {
    return theTweakServer->set(uri, value);
}

void TweakServerSingleton::remove(tweak_id id) {
    if (!theTweakServer) {
        throw tweak2::PyTweakException("initialize_library() hasn't been called");
    }
    return theTweakServer->remove(id);
}

void TweakServerSingleton::finalize_library() {
    theTweakServer.release();
    tweak_finalize_library();
}

}
