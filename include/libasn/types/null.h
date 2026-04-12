#pragma once

#include <optional>

namespace libasn {
namespace internal {

struct null_type {
    constexpr auto operator()(std::nullptr_t value = nullptr) const { return value; }

    template <typename Reader>
    std::optional<std::nullptr_t> read(Reader &reader) const {
        if (!reader.empty()) {
            return std::nullopt;
        }
        return nullptr;
    }
};

} // namespace internal
} // namespace libasn
