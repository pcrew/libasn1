#pragma once

#include <optional>

#include <libasn/per/detail/x691.h>

namespace libasn {
namespace internal {

struct per_real_type {
    bool aligned{false};

    explicit constexpr per_real_type(bool ap = false)
        : aligned(ap) {}

    template <typename Reader>
    std::optional<double> read(Reader &reader) const {
        if (aligned) {
            return per_codec::real_unconstrained_aligned(reader);
        }
        return per_codec::real_unconstrained(reader);
    }
};

} // namespace internal
} // namespace libasn
