#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <variant>
#include <limits>
#include <cctype>

#include <libasn/compiler.h>
#include <libasn/tag_class.h>
#include <libasn/encoding.h>

namespace libasn {
struct packet_length {
    std::optional<std::size_t> length;
    explicit packet_length(decltype(length) &&length)
        : length(std::forward<decltype(length)>(length)) {}

    constexpr auto is_undefine() const { return !length.has_value(); }

    template <typename Reader>
    static std::optional<packet_length> read(Reader &reader) {
        auto byte = reader.read();
        if (unlikely(!byte)) {
            return std::nullopt;
        }

        if (likely((*byte & 0x80) == 0)) {
            return packet_length(*byte);
        }

        auto cnt = *byte & 0x7F;

        if (cnt == 0) {
            return packet_length(std::nullopt);
        }

        if (static_cast<size_t>(cnt) > sizeof(size_t)) {
            return std::nullopt;
        }

        size_t len{0};

        for (auto i = 0; i < cnt; ++i) {
            byte = reader.read();
            if (!byte) {
                return std::nullopt;
            }

            len = (len << 8) | *byte;
        }

        if (len == std::numeric_limits<std::size_t>::max()) {
            return std::nullopt;
        }

        return packet_length(len);
    }
};

} // namespace libasn
