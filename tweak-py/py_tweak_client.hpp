/**
 * @file py_tweak_client.hpp
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

#ifndef PY_TWEAK_CLIENT_INCLUDED
#define PY_TWEAK_CLIENT_INCLUDED

#include "gil_loop.hpp"
#include "py_tweak_common.hpp"

#include <tweak2/tweak2.h>
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

namespace py = pybind11;

namespace tweak2
{

/**
 * @brief This class encapsulates single tweak connection.
 *
 * @note snake_case is used for function names to be consistent
 *       with Python3 naming convention.
 */
class TweakClient : public TweakBase
{
public:
    tweak_app_context getContext() const override;

    using ItemChangedCallback = std::function<void(tweak_id, const py::object& value)>;

    using ItemSelectorPredicate = std::function<bool(const std::string&)>;

    TweakClient(const std::string &context_type,
                const std::string &params,
                const std::string &uri);

    py::list list();

    py::list list(ItemSelectorPredicate predicate);

    std::vector<tweak_id> collect(const std::vector<std::string>& uris, tweak2::TimeoutMillis timeout);

    tweak_id find(const std::string& uri, tweak2::TimeoutMillis timeout) override;

    void set_item_callback(tweak_id id, ItemChangedCallback callback);

    void remove_item_callback(tweak_id id);

    void set_item_callback(const std::string &uri, ItemChangedCallback callback, tweak2::TimeoutMillis timeout);

    void remove_item_callback(const std::string &uri);

    using TweakBase::get;
    using TweakBase::set;

    ~TweakClient();

private:
    tweak_app_client_context context;

    static void on_current_value_changed_callback(tweak_app_context context,
        tweak_id id, tweak_variant* value, void *cookie);

    void on_current_value_changed_callback(tweak_app_context context,
        tweak_id id, tweak_variant* value);
};

}

#endif
