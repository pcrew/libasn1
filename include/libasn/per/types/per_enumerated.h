#pragma once

#include <cstdint>
#include <optional>
#include <type_traits>

#include <libasn/per/detail/x691.h>

namespace libasn {
namespace internal {

template <typename Enum, std::uint32_t NRoot, bool Extensible = false>
struct per_enumerated_type {
    static_assert(std::is_enum_v<Enum>, "PER enumerated expects enum");

    constexpr per_enumerated_type() = default;

    template <typename Reader>
    std::optional<Enum> read(Reader &reader) const {
        auto v = per_codec::enumerated_decode(reader, NRoot, Extensible);
        if (!v) {
            return std::nullopt;
        }
        return static_cast<Enum>(*v);
    }
};

} // namespace internal
} // namespace libasn
