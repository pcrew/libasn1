#pragma once

#include <optional>

namespace libasn {
namespace internal {

struct null_type {
    constexpr auto operator()(std::nullptr_t value = nullptr) const { return value; }

    template <typename Reader>
    auto read(Reader &reader) const {
        return std::optional(nullptr);
    }
};

}  // namespace internal
}  // namespace libasn
