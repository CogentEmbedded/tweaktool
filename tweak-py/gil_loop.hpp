/**
 * @file gil_loop.h
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

#ifndef GIL_LOOP_HPP
#define GIL_LOOP_HPP

#include <tweak2/tweak2.h>
#include <tweak2/variant.h>
#include <tweak2/metadata.h>

#include <pybind11/pybind11.h>

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include "raii.hpp"

namespace py = pybind11;

namespace tweak2
{

using ItemChangedCallback = std::function<void(tweak_id, const py::object& value)>;

class GilLoop
{
public:
    GilLoop();
    ~GilLoop();
    void feed(tweak_id id, VariantGuard&& value, MetadataGuard&& metadata);
    void setHandler(tweak_id id, ItemChangedCallback callback);
    void removeHandler(tweak_id id);
private:
    void gilLoopProc() noexcept(true);

    static void executePythonCallback(ItemChangedCallback cb, tweak_id id, const py::object& value) noexcept(true);

    std::unordered_map<tweak_id, ItemChangedCallback> callbacks;

    std::mutex lock;

    std::condition_variable cond;

    std::thread gilLoop;

    bool shutdown;

    class QueueItem {
        tweak_id id_;
        ItemChangedCallback callback_;
        VariantGuard value_;
        MetadataGuard metadata_;
    public:
        QueueItem() = default;

        QueueItem& operator=(const QueueItem& arg) = delete;

        QueueItem(const QueueItem& arg) = delete;

        QueueItem(tweak_id id, ItemChangedCallback callback,
            VariantGuard&& value, MetadataGuard&& metadata)
            : id_(id), callback_(callback)
            , value_(std::move(value)), metadata_(std::move(metadata))
        {}

        QueueItem(QueueItem&& arg)
            : QueueItem(arg.id_, arg.callback_, std::move(arg.value_), std::move(arg.metadata_))
        {}

        QueueItem& operator=(QueueItem&& arg) {
            QueueItem tmp(std::move(arg));
            swap(tmp);
            return *this;
        }

        void swap(QueueItem& arg) {
            std::swap(id_, arg.id_);
            std::swap(callback_, arg.callback_);
            value_.swap(arg.value_);
            metadata_.swap(arg.metadata_);
        }

        tweak_id id() const {
            return id_;
        }

        ItemChangedCallback callback() const {
            return callback_;
        }

        py::object makeObject() const;
    };

    bool waitChanges(std::vector<QueueItem> &target);

    std::vector<QueueItem> changeQueue;
};

}

#endif
