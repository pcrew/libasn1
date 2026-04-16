#pragma once

#include <cstdint>
#include <optional>
#include <type_traits>

#include <libasn/per/detail/x691.h>

namespace libasn {
namespace internal {

template <std::intmax_t Lo, std::intmax_t Hi>
struct per_constrained_integer_type {
    static_assert(Lo <= Hi, "PER integer constraint");

    template <typename Reader>
    std::optional<std::intmax_t> read(Reader &reader) const {
        return per_codec::integer_constrained_root(Lo, Hi, reader);
    }
};

template <std::intmax_t Lo, std::intmax_t Hi>
struct per_constrained_integer_extensible_type {
    static_assert(Lo <= Hi, "PER integer constraint");

    bool aligned{false};

    explicit constexpr per_constrained_integer_extensible_type(bool ap = false)
        : aligned(ap) {}

    template <typename Reader>
    std::optional<std::intmax_t> read(Reader &reader) const {
        (void)aligned;
        return per_codec::integer_constrained_extensible(Lo, Hi, reader);
    }
};

template <std::intmax_t LowerBound>
struct per_semi_constrained_integer_type {
    bool aligned{false};

    explicit constexpr per_semi_constrained_integer_type(bool ap = false)
        : aligned(ap) {}

    template <typename Reader>
    std::optional<std::intmax_t> read(Reader &reader) const {
        if (aligned) {
            reader.align_to_octet();
        }
        return per_codec::integer_semi_constrained(reader, LowerBound);
    }
};

} // namespace internal
} // namespace libasn
