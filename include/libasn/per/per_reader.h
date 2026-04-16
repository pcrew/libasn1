#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace libasn {

/** Bit-level reader for PER / APER (ITU-T X.691). Bits are read MSB-first within each octet. */
struct per_reader {
    std::string_view data{};
    std::size_t      bit_pos{0};

    constexpr per_reader() = default;
    constexpr explicit per_reader(std::string_view d)
        : data(d) {}

    std::size_t total_bits() const { return data.size() * 8; }
    std::size_t bits_remaining() const { return total_bits() > bit_pos ? total_bits() - bit_pos : 0; }
    bool        empty() const { return bits_remaining() == 0; }

    std::size_t byte_index() const { return (bit_pos + 7) / 8; }

    std::string_view remainder_octets() const { return data.substr(byte_index()); }

    void align_to_octet() {
        auto const pad = bit_pos % 8;
        if (pad != 0) {
            bit_pos += 8 - pad;
        }
    }

    std::optional<bool> read_bit() {
        if (bit_pos >= total_bits()) {
            return std::nullopt;
        }
        auto const byte_i = bit_pos / 8;
        auto const bit_i  = 7 - (bit_pos % 8);
        ++bit_pos;
        return ((static_cast<unsigned char>(data[byte_i]) >> bit_i) & 1U) != 0;
    }

    std::optional<std::uint64_t> read_bits(std::size_t n) {
        if (n > 64 || bits_remaining() < n) {
            return std::nullopt;
        }
        std::uint64_t v = 0;
        for (std::size_t i = 0; i < n; ++i) {
            auto b = read_bit();
            if (!b) {
                return std::nullopt;
            }
            v = (v << 1U) | (static_cast<std::uint64_t>(*b ? 1 : 0));
        }
        return v;
    }

    std::optional<std::string> read_octets(std::size_t n) {
        std::string out;
        out.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            auto o = read_bits(8);
            if (!o) {
                return std::nullopt;
            }
            out[i] = static_cast<char>(static_cast<unsigned char>(*o));
        }
        return out;
    }
};

} // namespace libasn
