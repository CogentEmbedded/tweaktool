/**
 * @file py_tweak_client.cpp
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

#include "py_tweak_client.hpp"

namespace tweak2 {

TweakClient::TweakClient(const std::string &context_type,
                         const std::string &params,
                         const std::string &uri)
    :context(NULL)
{
    tweak_app_client_callbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.cookie = this;
    callbacks.on_current_value_changed = &TweakClient::on_current_value_changed_callback;

    context = tweak_app_create_client_context(context_type.c_str(), params.c_str(),
        uri.c_str(), &callbacks);

    if (!context) {
        throw tweak2::PyTweakException(std::string("ERROR: tweak_app_create_client_context() returned NULL" ));
    }
}

tweak_app_context TweakClient::getContext() const {
    return context;
}

std::vector<tweak_id> TweakClient::collect(const std::vector<std::string>& uris, tweak2::TimeoutMillis timeout)
{
    if (uris.empty()) {
        return std::vector<tweak_id>();
    }

    std::vector<const char*> tmp;
    for (const auto &uri : uris) {
        tmp.emplace_back(uri.c_str());
    }

    std::vector<tweak_id> result(uris.size());
    tweak_app_error_code error_code =  tweak_app_client_wait_uris(context, tmp.data(), tmp.size(), &result[0], timeout);
    switch (error_code)
    {
    case TWEAK_APP_SUCCESS:
        return result;
    case TWEAK_APP_TIMEOUT:
        throw py::key_error(std::string("ERROR: Server haven't provided some of uris within given timeout"));
    default:
        throw tweak2::PyTweakException(
            std::string("ERROR: Internal tweak error :")
                + translate_app_error_code(error_code));
    }
}

void TweakClient::on_current_value_changed_callback(tweak_app_context context,
    tweak_id id, tweak_variant* value, void *cookie)
{
    static_cast<TweakClient*>(cookie)->on_current_value_changed_callback(context, id, value);
}

struct TraverseItemContext {
    py::list acc;
    TweakClient::ItemSelectorPredicate predicate;
};

bool traverse_items_callback(const tweak_app_item_snapshot* snapshot,
  void* cookie)
{
    TraverseItemContext *traverseItemContext = static_cast<TraverseItemContext*>(cookie);
    if (traverseItemContext->predicate(tweak_variant_string_c_str(&snapshot->uri)))
    {
        py::tuple row(6);
        row[0] = py::int_(snapshot->id);
        row[1] = py::str(tweak_variant_string_c_str(&snapshot->uri));
        row[2] = py::str(tweak_variant_string_c_str(&snapshot->meta));
        row[3] = py::str(tweak_variant_string_c_str(&snapshot->description));
        if (!tweak2::typeHasDataLayout(snapshot->current_value.type)) {
            row[4] = tweak2::convertFromTweak(&snapshot->default_value);
            row[5] = tweak2::convertFromTweak(&snapshot->current_value);
        }
        traverseItemContext->acc.append(row);
    }
    return true;
}

py::list TweakClient::list(ItemSelectorPredicate predicate) {
    TraverseItemContext traverseItemContext;
    traverseItemContext.predicate = predicate;
    if (tweak_app_traverse_items(context, traverse_items_callback, &traverseItemContext)) {
        return traverseItemContext.acc;
    } else {
        TWEAK_FATAL("Traverse function should never return false");
        return py::list();
    }
}

py::list TweakClient::list() {
    return list([](const std::string&) -> bool { return true; });
}

void TweakClient::on_current_value_changed_callback(tweak_app_context context,
    tweak_id id, tweak_variant* value)
{
    VariantGuard value0(value);
    MetadataGuard metadata;
    if (tweak2::typeHasDataLayout(value0.type())) {
        metadata = copyMetadata(id);
    }
    gilLoop.feed(id, std::move(value0), std::move(metadata));
}

tweak_id TweakClient::find(const std::string& uri, tweak2::TimeoutMillis timeout) {
    tweak_id retVal[1];
    if (timeout == 0) {
        retVal[0] = tweak_app_find_id(context, uri.c_str());
    } else {
        const char* uris[] = { uri.c_str() };
        tweak_app_error_code error_code =  tweak_app_client_wait_uris(context, uris, 1, retVal, timeout);
        switch (error_code)
        {
        case TWEAK_APP_TIMEOUT:
            retVal[0] = TWEAK_INVALID_ID;
        case TWEAK_APP_SUCCESS:
            break;
        default:
            throw tweak2::PyTweakException(
                std::string("ERROR: Internal tweak error :")
                    + translate_app_error_code(error_code));
        }
    }

    if (retVal[0] != TWEAK_INVALID_ID) {
        return retVal[0];
    } else {
        throw py::key_error(std::string("Missing URI: ") + uri);
    }
}

void TweakClient::set_item_callback(tweak_id id, ItemChangedCallback callback)
{
    gilLoop.setHandler(id, callback);
}

void TweakClient::remove_item_callback(tweak_id id) {
    gilLoop.removeHandler(id);
}

void TweakClient::set_item_callback(const std::string &uri,
    ItemChangedCallback callback, tweak2::TimeoutMillis timeout)
{
    set_item_callback(find(uri, timeout), callback);
}

void TweakClient::remove_item_callback(const std::string &uri) {
    remove_item_callback(find(uri, tweak2::DEFAULT_WAIT_TIMEOUT));
}

TweakClient::~TweakClient() {
    tweak_app_flush_queue((tweak_app_context)context);
    tweak_app_destroy_context((tweak_app_context)context);
}

}
