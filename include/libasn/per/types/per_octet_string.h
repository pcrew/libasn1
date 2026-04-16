#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include <libasn/per/types/per_integer.h>

namespace libasn {
namespace internal {

template <std::size_t MinLen, std::size_t MaxLen>
struct per_constrained_octet_string_type {
    static_assert(MinLen <= MaxLen, "PER SIZE constraint");

    template <typename Reader>
    std::optional<std::string> read(Reader &reader) const {
        per_constrained_integer_type<static_cast<std::intmax_t>(MinLen), static_cast<std::intmax_t>(MaxLen)>
            len_codec{};
        auto n = len_codec.read(reader);
        if (!n || *n < 0) {
            return std::nullopt;
        }
        auto const count = static_cast<std::size_t>(*n);
        if (count * 8 > reader.bits_remaining()) {
            return std::nullopt;
        }
        return reader.read_octets(count);
    }
};

} // namespace internal
} // namespace libasn
