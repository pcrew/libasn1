#pragma once

namespace libasn {
namespace ber {
namespace internal {

template <typename Type>
struct sequence_of_type {
    Type type;

    template <typename T>
    explicit constexpr sequence_of_type(T &&type)
        : type(std::forward<T>(type)) {}

    template <typename... Args>
    auto operator()(Args &&...args) const {
        return std::tuple(std::forward<Args>(args)...);
    }

    template <typename Reader>
    auto read(Reader &reader) const {
        auto view = reader.read(reader.size());
        return view ? std::optional(Reader{*view}) : std::nullopt;
    }
};

}  // namespace internal
}  // namespace ber
}  // namespace libasn
