#pragma once

namespace libasn {
namespace ber {
namespace internal {

template <typename Type>
struct explicit_type {
    Type type;

    template <typename T>
    explicit constexpr explicit_type(T &&type)
        : type(std::forward<T>(type)) {}

    template <typename ValueType>
    auto operator()(ValueType &&value) const {
        return std::forward<ValueType>(value);
    }

    template <typename Reader>
    auto read(Reader &reader) const {
        return type.read(reader);
    }
};

}  // namespace internal
}  // namespace ber
}  // namespace libasn
