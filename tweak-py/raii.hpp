/**
 * @file raii.h
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

#ifndef RAII_HPP_INCLUDED
#define RAII_HPP_INCLUDED

#include <tweak2/tweak2.h>
#include <tweak2/variant.h>
#include <tweak2/metadata.h>

#include <memory>

namespace tweak2
{

namespace detail {

struct MetadataDeleter {
    void operator()(tweak_metadata arg) const {
        tweak_metadata_destroy(arg);
    };
};

struct ListenerDeleter {
    void operator()(tweak_default_client_item_changed_listener_t arg) const {
        free(arg);
    };
};

}

using MetadataGuard = std::unique_ptr<typename std::remove_pointer<tweak_metadata>::type, detail::MetadataDeleter>;

using ListenerGuard =
    std::unique_ptr<
        typename std::remove_pointer<tweak_default_client_item_changed_listener_t
    >::type, detail::ListenerDeleter>;

struct VariantGuard {
    VariantGuard()
        : instance_(TWEAK_VARIANT_INIT_EMPTY)
    {}

    VariantGuard(tweak_variant *instance)
        : VariantGuard()
    {
        tweak_variant_swap(&instance_, instance);
    }

    VariantGuard(const VariantGuard&) = delete;

    VariantGuard& operator=(const VariantGuard&) = delete;

    void swap(VariantGuard& o) {
        tweak_variant_swap(&instance_, &o.instance_);
    }

    VariantGuard(VariantGuard&& o)
        : VariantGuard(&o.instance_)
    {}

    VariantGuard& operator=(VariantGuard&& o) {
        VariantGuard tmp(std::move(o));
        swap(tmp);
        return *this;
    }

    tweak_variant& get() {
        return instance_;
    }

    const tweak_variant& get() const {
        return instance_;
    }

    VariantGuard copy() const {
        tweak_variant instance = tweak_variant_copy(&instance_);
        return VariantGuard(&instance);
    }

    tweak_variant_type type() const {
        return instance_.type;
    }

    void release_to(tweak_variant *dst) {
        tweak_variant_swap(dst, &instance_);
    }

private:
    tweak_variant instance_;
};

}

#endif
