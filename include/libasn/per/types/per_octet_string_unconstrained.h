#pragma once

#include <optional>
#include <string>

#include <libasn/per/detail/x691.h>

namespace libasn {
namespace internal {

struct per_unconstrained_octet_string_type {
    bool aligned{false};

    explicit constexpr per_unconstrained_octet_string_type(bool ap = false)
        : aligned(ap) {}

    template <typename Reader>
    std::optional<std::string> read(Reader &reader) const {
        if (aligned) {
            return per_codec::octet_string_unconstrained_aligned(reader);
        }
        return per_codec::octet_string_unconstrained(reader);
    }
};

} // namespace internal
} // namespace libasn
