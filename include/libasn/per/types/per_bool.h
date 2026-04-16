#pragma once

#include <optional>

namespace libasn {
namespace internal {

struct per_boolean_type {
    template <typename Reader>
    std::optional<bool> read(Reader &reader) const {
        auto b = reader.read_bit();
        if (!b) {
            return std::nullopt;
        }
        return *b;
    }
};

} // namespace internal
} // namespace libasn
