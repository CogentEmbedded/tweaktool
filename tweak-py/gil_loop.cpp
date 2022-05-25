/**
 * @file gil_loop.cpp
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

#include <algorithm>
#include "gil_loop.hpp"
#include "py_tweak_common.hpp"
#include <sstream>

namespace tweak2 {

py::object GilLoop::QueueItem::makeObject() const {
    if (tweak2::typeHasDataLayout(value_.get().type)) {
        VariantGuard variant = value_.copy();
        MetadataGuard meta = MetadataGuard(tweak_metadata_copy(metadata_.get()));
        return py::cast(Buffer(std::move(variant), std::move(meta)));
    } else {
        return convertFromTweak(&value_.get());
    }
};

GilLoop::GilLoop()
    :shutdown(false)
{
    gilLoop = std::thread([this]() -> void { gilLoopProc(); });
}

GilLoop::~GilLoop()
{
    shutdown = true;
    cond.notify_all();
    if (gilLoop.joinable()) {
        gilLoop.join();
    }
}

void GilLoop::feed(tweak_id id, VariantGuard&& value, MetadataGuard&& metadata) {
    std::unique_lock<std::mutex> ulock(lock);
    auto itr = callbacks.find(id);
    if (itr == callbacks.end())
        return;

    changeQueue.erase(
        std::remove_if(changeQueue.begin(), changeQueue.end(), [id](const auto &item) -> bool {
            return item.id() == id;
        }), changeQueue.end());

    changeQueue.emplace_back(QueueItem(id, itr->second, std::move(value), std::move(metadata)));
    ulock.unlock();
    cond.notify_all();
}

void GilLoop::gilLoopProc() noexcept(true) {
    std::vector<QueueItem> threadLocalCopy;
    while (waitChanges(threadLocalCopy)) {
        py::gil_scoped_acquire gil_lock;
        for (const auto& item : threadLocalCopy) {
            executePythonCallback(item.callback(), item.id(), item.makeObject());
        }
        threadLocalCopy.clear();
    }
}

void GilLoop::executePythonCallback(ItemChangedCallback cb,
    tweak_id id, const py::object& value) noexcept(true)
{
    try {
        cb(id, value);
#if (1000 * PYBIND11_VERSION_MAJOR + PYBIND11_VERSION_MINOR) >= 2006
    } catch (py::error_already_set &eas) {
        // Discard the Python error using Python APIs, using the C++ magic
        // variable _func_. Python already knows the type and value and of the
        // exception object.
        eas.discard_as_unraisable(__func__);
#endif
    } catch (std::exception &e) {
        py::print(e.what());
    } catch (...) {
        py::print("Got generic C++ exception from callback");
    }
}

bool GilLoop::waitChanges(std::vector<QueueItem> &target) {
    std::unique_lock<std::mutex> ulock(lock);
    cond.wait(ulock, [this]{ return shutdown || !changeQueue.empty(); });
    if (shutdown) {
        return false;
    } else {
        target.swap(changeQueue);
        return true;
    }
}

void GilLoop::setHandler(tweak_id id, ItemChangedCallback callback) {
    std::unique_lock<std::mutex> ulock(lock);
    callbacks[id] = callback;
}

void GilLoop::removeHandler(tweak_id id) {
    std::unique_lock<std::mutex> ulock(lock);
    callbacks.erase(id);
}

}
