#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include <libasn/per/types/per_integer.h>

namespace libasn {
namespace internal {

template <std::size_t MinN, std::size_t MaxN, typename Elem>
struct per_sequence_of_type {
    Elem elem;

    template <typename T>
    explicit constexpr per_sequence_of_type(T &&e)
        : elem(std::forward<T>(e)) {}

    template <typename Reader>
    auto read(Reader &reader) const -> std::optional<
        std::vector<typename decltype(std::declval<const Elem &>().read(std::declval<Reader &>()))::value_type>> {
        using value_type = typename decltype(std::declval<const Elem &>().read(std::declval<Reader &>()))::value_type;

        per_constrained_integer_type<static_cast<std::intmax_t>(MinN), static_cast<std::intmax_t>(MaxN)> count_codec{};
        auto n = count_codec.read(reader);
        if (!n || *n < 0) {
            return std::nullopt;
        }
        auto const              count = static_cast<std::size_t>(*n);
        std::vector<value_type> out;
        out.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            auto v = elem.read(reader);
            if (!v) {
                return std::nullopt;
            }
            out.push_back(std::move(*v));
        }
        return out;
    }
};

} // namespace internal
} // namespace libasn
