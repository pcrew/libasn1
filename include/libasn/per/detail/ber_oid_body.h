#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <vector>

namespace libasn {
namespace per_codec {

/** Decode BER OBJECT IDENTIFIER body (base-128 subidentifiers) into arc list. */
inline std::optional<std::vector<std::uint32_t>> ber_object_identifier_components(std::string_view body) {
    if (body.empty()) {
        return std::vector<std::uint32_t>{};
    }
    std::size_t off        = 0;
    auto        next_subid = [&](std::uint64_t &acc) -> bool {
        acc       = 0;
        bool done = false;
        while (off < body.size()) {
            auto b = static_cast<std::uint8_t>(body[off++]);
            acc    = (acc << 7U) | static_cast<std::uint64_t>(b & 0x7FU);
            if (acc > std::numeric_limits<std::uint32_t>::max()) {
                return false;
            }
            if ((b & 0x80U) == 0) {
                done = true;
                break;
            }
        }
        return done;
    };

    std::uint64_t v0 = 0;
    if (!next_subid(v0)) {
        return std::nullopt;
    }

    std::vector<std::uint32_t> arcs;
    if (v0 >= 80U) {
        arcs.push_back(2);
        arcs.push_back(static_cast<std::uint32_t>(v0 - 80U));
    } else if (v0 >= 40U) {
        arcs.push_back(1);
        arcs.push_back(static_cast<std::uint32_t>(v0 - 40U));
    } else {
        arcs.push_back(0);
        arcs.push_back(static_cast<std::uint32_t>(v0));
    }

    while (off < body.size()) {
        std::uint64_t v = 0;
        if (!next_subid(v)) {
            return std::nullopt;
        }
        arcs.push_back(static_cast<std::uint32_t>(v));
    }
    return arcs;
}

} // namespace per_codec
} // namespace libasn
