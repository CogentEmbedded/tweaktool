/**
 * @file py_tweak.cpp
 * @ingroup tweak-external-interfaces
 *
 * @brief Tweak 2 Python 3 bindings.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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
#include "py_tweak_common.hpp"
#include "py_tweak_server.hpp"
#include "py_tweak_client.hpp"

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

namespace
{
static const char *initialize_library_docstring =
    R"_(
Initialize server instance

    Args:
        context_type (string): Context type
        params (string): parameters
        uri (string): server interface uri

    Returns:
        None
)_";

static const char *create_vector_docstring = "";

static const char *find_item_docstring =
    R"_(
Find tweak_id of an item with the given uri

    Args:
        uri (string): Item uri

    Returns:
        object: tweak_id of the item
)_";

static const char *set_item_by_uri_docstring =
    R"_(
Set item value

    Args:
        item(string): uri registered with TweakServer.add_(...)
        value: New value

    Returns:
        none
)_";
static const char *get_item_by_id_docstring =
    R"_(
Get item value

    Args:
        item(tweak_id): tweak_id returned by TweakServer.add_(...) or TweakServer.find(...)

    Returns:
        Current item value
)_";

static const char *remove_item_docstring =
    R"_(
Remove item

    Args:
        item(tweak_id): tweak_id returned by TweakServer.add(...) or TweakServer.find(...)

    Returns:
        None
)_";

static const char *finalize_library_docstring =
    R"_(
Remove all tweaks and finalize tweak library

    Args:
        None

    Returns:
        None
)_";

static const char *add_item_docstring =
    R"_(
Create new tweak item

    Args:
        uri (string): Item uri e.g. '/folder/tweak1'
        description (string): Item description e.g. 'Image height'
        meta (string): Item metadata in json format
        initial_value: Item initial (default) value

    Returns:
        object: tweak_id of the newly created item
)_";

static const char *set_item_by_id_docstring =
    R"_(
Set item value

    Args:
        item(tweak_id): tweak_id returned by TweakServer.add_(...) or TweakServer.find(...)
        value: New value

    Returns:
        none
)_";
static const char *get_item_by_uri_docstring =
    R"_(
Get item value

    Args:
        item(string): uri registered with TweakServer.add_(...)

    Returns:
        Current item value
)_";

static const char *client_constructor_docstring =
    R"_(
Construct client instance.

    Args:
        contexttype (string): Context type. One of "nng", "serial". Type is case-sensitive.
        params (string): parameters Additional params for backend separated by semicolon ';'.
    Only mutually exclusive "role=server" and "role=client"
    are currently recognized for IP-based connections.
        uri (string): server interface uri. Connection URI for the given network backend.

    Returns:
        None
)_";

static const char *client_collect_items_docstring =
    R"_(
Collects tweak_id identifiers for items specified by array of uris
Blocks until remote side provides item with the given uri or timeout elapses.

    Args:
        uris (array of string): Item uri array
        timeout (integer): timeout in milliseconds. Default is 10_000.

    Returns:
        array of tweak_id: tweak_id identifiers for items specified by array of uris
    in respective order.

    Throws:
        key_error: When server haven't provided all given uris within given timeout
    if server is offline or it doesn't provide some of uris or time to bring it up
    take longer than timeout specified.
)_";

static const char *client_find_item_docstring =
    R"_(
Find tweak_id of an item with the given uri.

    Args:
        uri (string): Item uri
        timeout (integer): timeout in milliseconds. Default is 10_000.

    Returns:
        object: tweak_id of the item

    Throws:
        key_error: When server doesn't have item with given uri.
)_";

static const char *client_set_item_by_id_docstring =
    R"_(
Set item value

    Args:
        item(tweak_id): tweak_id returned by TweakClient.collect(...) or TweakClient.find(...)
        value: New value

    Returns:
        none

    Throws:
        key_error: when server doesn't have an item with given tweak_id.
)_";

static const char *client_get_item_by_id_docstring =
    R"_(
Get item value.

    Args:
        item(tweak_id): tweak_id returned by TweakClient.collect(...) or TweakClient.find(...)

    Returns:
        Current item value

   Throws:
        key_error: when server doesn't have an item with given tweak_id.
)_";

static const char *client_get_item_by_uri_docstring =
    R"_(
Equivalent to get(find(uri, timeout)).

    Args:
        item(string): Item uri
        timeout (integer): timeout in milliseconds. Default is 10_000.

    Returns:
        Current item value

    Throws:
        key_error: When server doesn't have item with given uri.
)_";

static const char *client_get_item_by_uri_index_docstring =
    R"_(
Equivalent to get(find(uri, 10_000)).

    Args:
        item(string): Item uri

    Returns:
        Current item value

    Throws:
        key_error: When server doesn't have item with given uri.
)_";

static const char *client_set_item_by_uri_docstring =
    R"_(
Equivalent to set(find(uri, timeout), value).

    Args:
        item(string): Item uri
        value: New value
        timeout (integer): timeout in milliseconds. Default is 10_000.

    Returns:
        none

    Throws:
        key_error: When server doesn't have item with given uri.
)_";

static const char *client_set_item_by_uri_index_docstring =
    R"_(
Equivalent to set(find(uri, 10_000), value).

    Args:
        item(string): Item uri
        value: New value

    Returns:
        none

    Throws:
        key_error: When server doesn't have item with given uri.
)_";

static const char *client_set_item_callback_by_id_docstring =
    R"_(
Sets callback function to monitor changes of an item specified by
tweak_id as asynchronous push events.

    Args:
        item(tweak_id): Item id tweak_id returned by TweakClient.find(...)
          or TweakClient.collect(...)
        callback(Callable[[int, object], None]): Callback

    Returns:
        None

    Throws:
        key_error: When server doesn't have item with given id.
)_";

static const char *client_set_item_callback_by_uri_docstring =
    R"_(
Equivalent to set_item_callback(find(uri, timeout), callback)

    Args:
        item(uri): Item uri
        callback(Callable[[int, object], None]): Callback
        timeout (integer): timeout in milliseconds. Default is 10_000.

    Returns:
        None

    Throws:
        key_error: When server doesn't have item with given uri.
)_";

static const char *client_remove_item_callback_by_id_docstring =
    R"_(
Clears all callbacks attached to an item specified by
tweak_id.

    Args:
        item(tweak_id): Item id tweak_id returned by TweakClient.find(...)
          or TweakClient.collect(...)

    Returns:
        None

    Throws:
        key_error: When server doesn't have item with given id.
)_";

static const char *client_remove_item_callback_by_uri_docstring =
    R"_(
Clears all callbacks attached to an item specified by uri.

    Args:
        item(uri): Item uri

    Returns:
        None

    Throws:
        key_error: When there's no item with given uri.
)_";

static const char *client_list_docstring_pred =
    R"_(
Retrieves items provided to this client by server matching optional predicate.
Intentended for interactive use, since protocol could only guarantee that
after successful wait(["/example_uri/a", "/example_uri/a"]),
list returned by this call should contain items with uris "/example_uri/a"
and "/example_uri/a" among others, assuming that server haven't removed items
with uris "/example_uri/a" or "/example_uri/b" between wait() and list() calls.

    Args:
        predicate (Callable[[str], bool]): predicate taking
    uri and returning boolean indicating whether to include given
    item to resulting list. Default predicate always return true.

    Returns:
        List of tuples each having six elements:
            - tweak_id (integer)
            - uri (string)
            - meta (string)
            - description (string)
            - default_value (variant)
            - current_value (variant)

    Throws:
        None.
)_";

static const char *client_list_docstring =
    R"_(
Equivalent to list(lambda uri: True)

    Returns:
        List of tuples each having six elements:
            - tweak_id (integer)
            - uri (string)
            - meta (string)
            - description (string)
            - default_value (variant)
            - current_value (variant)

    Throws:
        None.
)_";

} // namespace

#if !defined (_MSC_BUILD)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
PYBIND11_PLUGIN(tweak2) {
    using tweak2::TweakClient;
    using tweak2::TweakServer;
    using tweak2::TweakServerSingleton;
    using tweak2::Buffer;

    py::module m("tweak2", "pbdoc(Python wrapper for libtweak2)");
    py::class_<tweak_id>(m, "tweak_id");
    py::class_<Buffer>(m, "Buffer", py::buffer_protocol())
        .def_buffer([](Buffer &b) -> py::buffer_info {
            return b.makeBufferInfo();
        });
    py::class_<TweakServerSingleton>(m, "server")
        .def_static("initialize_library", &TweakServerSingleton::initialize_library,
                    initialize_library_docstring,
                    py::arg("context_type"),
                    py::arg("params"),
                    py::arg("uri"))
        .def_static("add_bool", &TweakServerSingleton::add_bool,
                    add_item_docstring,
                    py::arg("uri"),
                    py::arg("description") = std::string(),
                    py::arg("meta") = std::string("{}"),
                    py::arg("initial_value") = false,
                    py::arg("callback") = nullptr)
        .def_static("add_int", &TweakServerSingleton::add_int,
                    add_item_docstring,
                    py::arg("uri"),
                    py::arg("description") = std::string(),
                    py::arg("meta") = std::string("{}"),
                    py::arg("initial_value") = 0,
                    py::arg("callback") = nullptr)
        .def_static("add_float", &TweakServerSingleton::add_float,
                    add_item_docstring,
                    py::arg("uri"),
                    py::arg("description") = std::string(),
                    py::arg("meta") = std::string("{}"),
                    py::arg("initial_value") = 0.,
                    py::arg("callback") = nullptr)
        .def_static("add", static_cast<
                tweak_id (&)(const std::string &uri,
                             const std::string &description,
                             const std::string &meta,
                             bool initial_value,
                             tweak2::ItemChangedCallback callback)
                            >(TweakServerSingleton::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = false,
            py::arg("callback") = nullptr)
        .def_static("add", static_cast<
                tweak_id (&)(const std::string &uri,
                             const std::string &description,
                             const std::string &meta,
                             int64_t initial_value,
                             tweak2::ItemChangedCallback callback)
                            >(TweakServerSingleton::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = 0,
            py::arg("callback") = nullptr)
        .def_static("add", static_cast<
                tweak_id (&)(const std::string &uri,
                             const std::string &description,
                             const std::string &meta,
                             double initial_value,
                             tweak2::ItemChangedCallback callback)
                            >(TweakServerSingleton::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = 0.,
            py::arg("callback") = nullptr)
        .def_static("add", static_cast<
                tweak_id (&)(const std::string &uri,
                             const std::string &description,
                             const std::string &meta,
                             const std::string &initial_value,
                             tweak2::ItemChangedCallback callback)
                            >(TweakServerSingleton::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = "",
            py::arg("callback") = nullptr)
        .def_static("add", static_cast<
                tweak_id (&)(const std::string &uri,
                             const std::string &description,
                             const std::string &meta,
                             const py::object &initial_value,
                             tweak2::ItemChangedCallback callback)
                            >(TweakServerSingleton::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = py::none(),
            py::arg("callback") = nullptr)
        .def_static("find", &TweakServerSingleton::find,
                    find_item_docstring,
                    py::arg("uri"))
        .def_static("set_bool", static_cast<void (&)(tweak_id id, bool value)>(TweakServerSingleton::set_item_value),
                    set_item_by_id_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("set_bool", static_cast<void (&)(const std::string &, bool value)>(TweakServerSingleton::set_item_value),
                    set_item_by_uri_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("set_int", static_cast<void (&)(tweak_id id, int64_t value)>(TweakServerSingleton::set_item_value),
                    set_item_by_id_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("set_int", static_cast<void (&)(const std::string &, int64_t value)>(TweakServerSingleton::set_item_value),
                    set_item_by_uri_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("set_float", static_cast<void (&)(tweak_id id, double value)>(TweakServerSingleton::set_item_value),
                    set_item_by_id_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("set_float", static_cast<void (&)(const std::string &, double value)>(TweakServerSingleton::set_item_value),
                    set_item_by_uri_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("set", static_cast<void (&)(tweak_id id, const py::object &value)>(TweakServerSingleton::set),
                    set_item_by_id_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("set", static_cast<void (&)(const std::string &, const py::object &value)>(TweakServerSingleton::set),
                    set_item_by_uri_docstring,
                    py::arg("item"),
                    py::arg("value"))
        .def_static("get_bool", static_cast<bool (&)(tweak_id id)>(TweakServerSingleton::get_item_bool_value),
                    get_item_by_id_docstring,
                    py::arg("item"))
        .def_static("get_bool", static_cast<bool (&)(const std::string &)>(TweakServerSingleton::get_item_bool_value),
                    get_item_by_uri_docstring,
                    py::arg("item"))
        .def_static("get_int", static_cast<int64_t (&)(tweak_id id)>(TweakServerSingleton::get_item_int_value),
                    get_item_by_id_docstring,
                    py::arg("item"))
        .def_static("get_int", static_cast<int64_t (&)(const std::string &)>(TweakServerSingleton::get_item_int_value),
                    get_item_by_uri_docstring,
                    py::arg("item"))
        .def_static("get_float", static_cast<double (&)(tweak_id id)>(TweakServerSingleton::get_item_float_value),
                    get_item_by_id_docstring,
                    py::arg("item"))
        .def_static("get_float", static_cast<double (&)(const std::string &)>(TweakServerSingleton::get_item_float_value),
                    get_item_by_uri_docstring,
                    py::arg("item"))
        .def_static("get", static_cast<py::object (&)(tweak_id id)>(TweakServerSingleton::get),
                    set_item_by_id_docstring,
                    py::arg("item"))
        .def_static("get", static_cast<py::object (&)(const std::string &uri)>(TweakServerSingleton::get),
                    set_item_by_uri_docstring,
                    py::arg("item"))
        .def_static("remove", &TweakServerSingleton::remove,
                    remove_item_docstring,
                    py::arg("item"));
    py::class_<TweakServer>(m, "Server")
        .def(py::init<
                const std::string &,
                const std::string &,
                const std::string &>(),
            client_constructor_docstring,
                py::arg("context_type"),
                py::arg("params"),
                py::arg("uri"))
        .def("add_bool", &TweakServer::add_bool,
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = false,
            py::arg("callback") = nullptr)
        .def("add_int", &TweakServer::add_int,
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = 0,
            py::arg("callback") = nullptr)
        .def("add_float", &TweakServer::add_float,
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = 0.,
            py::arg("callback") = nullptr)
        .def("add", static_cast<
                tweak_id (TweakServer::*)(const std::string &uri,
                                          const std::string &description,
                                          const std::string &meta,
                                          bool initial_value,
                                          tweak2::ItemChangedCallback callback)
                                >(&TweakServer::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = false,
            py::arg("callback") = nullptr)
        .def("add", static_cast<
                tweak_id (TweakServer::*)(const std::string &uri,
                                          const std::string &description,
                                          const std::string &meta,
                                          int64_t initial_value,
                                          tweak2::ItemChangedCallback callback)
                                >(&TweakServer::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = 0,
            py::arg("callback") = nullptr)
        .def("add", static_cast<
                tweak_id (TweakServer::*)(const std::string &uri,
                                          const std::string &description,
                                          const std::string &meta,
                                          double initial_value,
                                          tweak2::ItemChangedCallback callback)
                                >(&TweakServer::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = 0.,
            py::arg("callback") = nullptr)
        .def("add", static_cast<
                tweak_id (TweakServer::*)(const std::string &uri,
                                          const std::string &description,
                                          const std::string &meta,
                                          const std::string &initial_value,
                                          tweak2::ItemChangedCallback callback)
                                >(&TweakServer::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = "",
            py::arg("callback") = nullptr)
        .def("add", static_cast<
                tweak_id (TweakServer::*)(const std::string &uri,
                                          const std::string &description,
                                          const std::string &meta,
                                          const py::object &initial_value,
                                          tweak2::ItemChangedCallback callback)
                                >(&TweakServer::add),
            add_item_docstring,
            py::arg("uri"),
            py::arg("description") = std::string(),
            py::arg("meta") = std::string("{}"),
            py::arg("initial_value") = py::none(),
            py::arg("callback") = nullptr)
        .def("find", static_cast<tweak_id (TweakServer::*)(const std::string& uri)>(&TweakServer::find),
            find_item_docstring,
            py::arg("uri"))
        .def("set_bool", static_cast<void (TweakServer::*)(tweak_id id, bool value)>(&TweakServer::set_item_value),
            set_item_by_id_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("set_bool", static_cast<void (TweakServer::*)(const std::string &, bool value)>(&TweakServer::set_item_value),
            set_item_by_uri_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("set_int", static_cast<void (TweakServer::*)(tweak_id id, int64_t value)>(&TweakServer::set_item_value),
            set_item_by_id_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("set_int", static_cast<void (TweakServer::*)(const std::string &, int64_t value)>(&TweakServer::set_item_value),
            set_item_by_uri_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("set_float", static_cast<void (TweakServer::*)(tweak_id id, double value)>(&TweakServer::set_item_value),
            set_item_by_id_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("set_float", static_cast<void (TweakServer::*)(const std::string &, double value)>(&TweakServer::set_item_value),
            set_item_by_uri_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("set", static_cast<void (TweakServer::*)(tweak_id id, const py::object &value)>(&TweakServer::set),
            set_item_by_id_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("set", static_cast<void (TweakServer::*)(const std::string &, const py::object &value)>(&TweakServer::set),
            set_item_by_uri_docstring,
            py::arg("item"),
            py::arg("value"))
        .def("get_bool", static_cast<bool (TweakServer::*)(tweak_id id)>(&TweakServer::get_item_bool_value),
            get_item_by_id_docstring,
            py::arg("item"))
        .def("get_bool", static_cast<bool (TweakServer::*)(const std::string &)>(&TweakServer::get_item_bool_value),
            get_item_by_uri_docstring,
            py::arg("item"))
        .def("get_int", static_cast<int64_t (TweakServer::*)(tweak_id id)>(&TweakServer::get_item_int_value),
            get_item_by_id_docstring,
            py::arg("item"))
        .def("get_int", static_cast<int64_t (TweakServer::*)(const std::string &)>(&TweakServer::get_item_int_value),
            get_item_by_uri_docstring,
            py::arg("item"))
        .def("get_float", static_cast<double (TweakServer::*)(tweak_id id)>(&TweakServer::get_item_float_value),
            get_item_by_id_docstring,
            py::arg("item"))
        .def("get_float", static_cast<double (TweakServer::*)(const std::string &)>(&TweakServer::get_item_float_value),
            get_item_by_uri_docstring,
            py::arg("item"))
        .def("get", static_cast<py::object (TweakServer::*)(tweak_id id)>(&TweakServer::get),
            set_item_by_id_docstring,
            py::arg("item"))
        .def("get", static_cast<py::object (TweakServer::*)(const std::string &)>(&TweakServer::get),
            set_item_by_uri_docstring,
            py::arg("item"))
        .def("remove", &TweakServer::remove,
            remove_item_docstring,
            py::arg("item"));

    py::class_<TweakClient>(m, "Client")
        .def(py::init<
                const std::string &,
                const std::string &,
                const std::string &>(),
            client_constructor_docstring,
                py::arg("contexttype"),
                py::arg("params"),
                py::arg("uri"))
        .def("collect", &TweakClient::collect,
            client_collect_items_docstring,
                py::arg("uris"),
                py::arg("timeout") = tweak2::DEFAULT_WAIT_TIMEOUT)
        .def("find", &TweakClient::find,
            find_item_docstring,
                py::arg("uri"),
                py::arg("timeout") = tweak2::DEFAULT_WAIT_TIMEOUT)
        .def("list", static_cast<py::list(TweakClient::*)(TweakClient::ItemSelectorPredicate)>(&TweakClient::list),
            client_list_docstring_pred,
                py::arg("predicate"))
        .def("list", static_cast<py::list(TweakClient::*)()>(&TweakClient::list),
            client_list_docstring)
        .def("set",
            static_cast<void (TweakClient::*)(tweak_id, const py::object&)>(&TweakClient::set),
                client_set_item_by_id_docstring,
                py::arg("tweak_id"),
                py::arg("value"))
        .def("set",
            static_cast
                <
                    void (TweakClient::*)(const std::string&, const py::object&, tweak2::TimeoutMillis timeout)
                >(&TweakClient::set),
                client_set_item_by_uri_docstring,
                py::arg("uri"),
                py::arg("value"),
                py::arg("timeout") = tweak2::DEFAULT_WAIT_TIMEOUT)
        .def("__setitem__",
            static_cast<void (TweakClient::*)(tweak_id, const py::object&)>(&TweakClient::set),
                client_set_item_by_id_docstring,
                py::arg("tweak_id"),
                py::arg("value"))
        .def("__setitem__",
            static_cast<void (TweakClient::*)(const std::string&, const py::object&)>(&TweakClient::set),
                client_set_item_by_uri_index_docstring,
                py::arg("uri"),
                py::arg("value"))
        .def("set_item_callback",
            static_cast
                <
                    void (TweakClient::*)(tweak_id id, TweakClient::ItemChangedCallback callback)
                >(&TweakClient::set_item_callback),
                client_set_item_callback_by_id_docstring,
                py::arg("id"),
                py::arg("callback"))
        .def("remove_item_callback",
            static_cast<void (TweakClient::*)(tweak_id id)>(&TweakClient::remove_item_callback),
                client_remove_item_callback_by_id_docstring,
                py::arg("id"))
        .def("set_item_callback",
            static_cast
                <
                    void (TweakClient::*)(const std::string&, TweakClient::ItemChangedCallback callback, tweak2::TimeoutMillis timeout)
                >(&TweakClient::set_item_callback),
                client_set_item_callback_by_uri_docstring,
                py::arg("id"),
                py::arg("callback"),
                py::arg("timeout") = tweak2::DEFAULT_WAIT_TIMEOUT)
        .def("remove_item_callback",
            static_cast<void (TweakClient::*)(const std::string&)>(&TweakClient::remove_item_callback),
                client_remove_item_callback_by_uri_docstring,
                py::arg("id"))
        .def("get",
            static_cast<py::object (TweakClient::*)(tweak_id)>(&TweakClient::get),
                client_get_item_by_id_docstring,
                py::arg("tweak_id"))
        .def("get",
            static_cast<py::object (TweakClient::*)(const std::string &uri, tweak2::TimeoutMillis timeout)>(&TweakClient::get),
                client_get_item_by_uri_docstring,
                py::arg("uri"),
                py::arg("timeout") = tweak2::DEFAULT_WAIT_TIMEOUT)
        .def("__getitem__",
            static_cast<py::object (TweakClient::*)(tweak_id)>(&TweakClient::get),
                client_get_item_by_id_docstring,
                py::arg("tweak_id"))
        .def("__getitem__",
            static_cast<py::object (TweakClient::*)(const std::string &uri)>(&TweakClient::get),
                client_get_item_by_uri_index_docstring,
                py::arg("uri"));
        m.add_object("_cleanup", py::capsule(&TweakServerSingleton::finalize_library));
    return m.ptr();
}
#if !defined (_MSC_BUILD)
#pragma GCC diagnostic pop
#endif
