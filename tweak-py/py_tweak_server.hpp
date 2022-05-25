/**
 * @file py_tweak_server.hpp
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

#ifndef PY_TWEAK_SERVER_INCLUDED
#define PY_TWEAK_SERVER_INCLUDED

#include <functional>
#include <tweak2/appserver.h>
#include <tweak2/tweak2.h>
#include "py_tweak_common.hpp"
#include "raii.hpp"
#include <unordered_map>

namespace tweak2
{
/**
 * @brief Static class that adapts all methods from singleton \
 *        tweak server defined in tweak2.h
 *
 * @note snake_case is used for function names to be consistent
 *       with Python3 naming convention.
 */
struct TweakServer : public TweakBase {
private:
     std::unordered_map<tweak_id, ListenerGuard> listeners;

    void on_value_changed(tweak_id id);

    tweak_app_server_context server_context;

    static void current_value_changed_callback_tweak_app(tweak_app_context context, tweak_id id,
        tweak_variant* value, void *cookie);

    static void current_value_changed_callback_tweak2h(tweak_id tweak_id, void* cookie);
public:
    TweakServer();

    TweakServer(const std::string &context_type,
        const std::string &params, const std::string &uri);

    ~TweakServer();

    tweak_id addItem(const std::string& uri, const std::string& description,
        const std::string& meta, VariantGuard&& value,
        ListenerGuard listener);

    tweak_app_context getContext() const override;

    tweak_id find(const std::string& uri);

    tweak_id find(const std::string& uri, tweak2::TimeoutMillis timeout) override;

    tweak_id add_bool(const std::string &uri,
        const std::string &description, const std::string &meta,
        bool initial_value, ItemChangedCallback callback);

    tweak_id add_int(const std::string &uri,
        const std::string &description, const std::string &meta,
        int64_t initial_value, ItemChangedCallback callback);

    tweak_id add_float(const std::string &uri,
        const std::string &description, const std::string &meta,
        double initial_value, ItemChangedCallback callback);

    tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        bool initial_value, ItemChangedCallback callback);

    tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        int64_t initial_value, ItemChangedCallback callback);

    tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        double initial_value, ItemChangedCallback callback);

    tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        const std::string &initial_value, ItemChangedCallback callback);

    tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        const py::object &initial_value, ItemChangedCallback callback);

    bool get_item_bool_value(tweak_id id);
    bool get_item_bool_value(const std::string &uri);

    int64_t get_item_int_value(tweak_id id);
    int64_t get_item_int_value(const std::string &uri);

    double get_item_float_value(tweak_id id);
    double get_item_float_value(const std::string &uri);

    void set_item_value(tweak_id id, bool value);
    void set_item_value(tweak_id id, int64_t value);
    void set_item_value(tweak_id id, double value);
    void set_item_value(const std::string &uri, bool value);
    void set_item_value(const std::string &uri, int64_t value);
    void set_item_value(const std::string &uri, double value);

    using TweakBase::get;
    using TweakBase::set;

    void remove(tweak_id id);
};

struct TweakServerSingleton {
    static void initialize_library(const std::string &context_type,
        const std::string &params, const std::string &uri);

    static tweak_id add_bool(const std::string &uri,
        const std::string &description, const std::string &meta,
        bool initial_value, ItemChangedCallback callback);

    static tweak_id add_int(const std::string &uri,
        const std::string &description, const std::string &meta,
        int64_t initial_value, ItemChangedCallback callback);

    static tweak_id add_float(const std::string &uri,
        const std::string &description, const std::string &meta,
        double initial_value, ItemChangedCallback callback);

    static tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        bool initial_value, ItemChangedCallback callback);

    static tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        int64_t initial_value, ItemChangedCallback callback);

    static tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        double initial_value, ItemChangedCallback callback);

    static tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        const std::string &initial_value, ItemChangedCallback callback);

    static tweak_id add(const std::string &uri,
        const std::string &description, const std::string &meta,
        const py::object &initial_value, ItemChangedCallback callback);

    static tweak_id find(const std::string &uri);

    static bool get_item_bool_value(tweak_id id);
    static bool get_item_bool_value(const std::string &uri);

    static int64_t get_item_int_value(tweak_id id);
    static int64_t get_item_int_value(const std::string &uri);

    static double get_item_float_value(tweak_id id);
    static double get_item_float_value(const std::string &uri);

    static py::object get(tweak_id id);
    static py::object get(const std::string &uri);

    static void set_item_value(tweak_id id, bool value);
    static void set_item_value(tweak_id id, int64_t value);
    static void set_item_value(tweak_id id, double value);
    static void set_item_value(const std::string &uri, bool value);
    static void set_item_value(const std::string &uri, int64_t value);
    static void set_item_value(const std::string &uri, double value);

    static void set(tweak_id id, const py::object &value);
    static void set(const std::string &uri, const py::object &value);

    static void remove(tweak_id id);

    static void finalize_library();
};

}

#endif
