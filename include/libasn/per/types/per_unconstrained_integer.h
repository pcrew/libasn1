#pragma once

#include <optional>

#include <libasn/per/detail/x691.h>

namespace libasn {
namespace internal {

struct per_unconstrained_integer_type {
    bool aligned{false};

    explicit constexpr per_unconstrained_integer_type(bool ap = false)
        : aligned(ap) {}

    template <typename Reader>
    std::optional<std::intmax_t> read(Reader &reader) const {
        if (aligned) {
            return per_codec::integer_unconstrained_aligned(reader);
        }
        return per_codec::integer_unconstrained(reader);
    }
};

} // namespace internal
} // namespace libasn
