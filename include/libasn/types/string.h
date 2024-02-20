#pragma once

#include <optional>

namespace libasn {
namespace internal {

struct string_type {
    template <typename ValueType>
    auto operator()(ValueType &&value) const {
        return std::forward<ValueType>(value);
    }

    template <typename Reader>
    auto read(Reader &reader) const {
        auto str = reader.read(reader.size());
        return str ? std::optional(Reader{*str}) : std::nullopt;
    }
};

}  // namespace internal
}  // namespace libasn
