#pragma once

namespace libasn {
namespace ber {
namespace internal {

template <typename... Types>
struct sequence_type {
    std::tuple<Types...> types;

    template <typename... Ts>
    explicit constexpr sequence_type(Ts &&...types)
        : types(std::forward<Ts>(types)...) {}

    template <typename... Args>
    auto operator()(Args &&...args) const {
        static_assert(sizeof...(Types) == sizeof...(args));
        return std::tuple(std::forward<Args>(args)...);
    }

    template <typename Reader>
    auto read(Reader &reader) const {
        return read_helper<0>(reader);
    }

private:
    template <size_t i, typename Reader, typename... ValuesType>
    auto read_helper(Reader &reader, ValuesType &&...values) const {
        if constexpr (i == sizeof...(Types)) {
            return std::optional(std::tuple(std::forward<ValuesType>(values)...));
        } else {
            auto value = std::get<i>(types).read(reader);
            if (!value) {
                return decltype(read_helper<i + 1>(reader, std::forward<ValuesType>(values)...,
                                                   std::forward<decltype(*value)>(*value))){};
            }

            return read_helper<i + 1>(reader, std::forward<ValuesType>(values)...,
                                      std::forward<decltype(*value)>(*value));
        }
    }
};

}  // namespace internal
}  // namespace ber
}  // namespace libasn
