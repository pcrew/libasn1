#pragma once

#include <optional>

namespace libasn {
namespace internal {

template <typename Type>
struct optional_type {
    Type type;

    template <typename T>
    explicit constexpr optional_type(T &&type)
        : type(std::forward<T>(type)) {}

    template <typename Reader>
    auto read(Reader &reader) const {
        auto st  = reader;
        auto res = type.read(reader);

        if (!res) {
            reader = std::forward<decltype(st)>(st);
        }
        return std::optional<decltype(res)>(std::forward<decltype(res)>(res));
    }
};

} // namespace internal
} // namespace libasn
