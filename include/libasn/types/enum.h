#pragma once

#include <optional>

#include "integer.h"

namespace libasn {
namespace internal {

template <typename Enum>
struct enum_type : public integer_type {
    template <typename ValueType>
    auto operator()(ValueType &&value) const {
        return std::forward<ValueType>(value);
    }

    template <typename Reader>
    std::optional<Enum> read(Reader &reader) const {
        auto e = integer_type::read(reader);
        return e ? std::optional(static_cast<Enum>(*e)) : std::nullopt;
    }
};

}  // namespace internal
}  // namespace libasn
