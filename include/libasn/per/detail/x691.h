#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <libasn/per/detail/ber_real_body.h>
#include <libasn/per/detail/per_common.h>
#include <libasn/per/per_reader.h>

namespace libasn {
namespace per_codec {

using libasn::internal::per_range_bit_width;

std::optional<std::uint64_t> constrained_whole_number(per_reader &r, int nbits) {
    if (nbits <= 0) {
        return 0;
    }
    if (nbits <= 31) {
        return r.read_bits(static_cast<std::size_t>(nbits));
    }
    if (static_cast<std::size_t>(nbits) > 8 * sizeof(std::uint64_t)) {
        return std::nullopt;
    }
    auto high = r.read_bits(31);
    if (!high) {
        return std::nullopt;
    }
    auto low = constrained_whole_number(r, nbits - 31);
    if (!low) {
        return std::nullopt;
    }
    return (*high << static_cast<unsigned>(nbits - 31)) | *low;
}

std::optional<std::pair<std::size_t, bool>> length_determinant(per_reader &r, int ebits, std::size_t lower_bound) {
    if (ebits >= 0 && ebits <= 16) {
        auto v = r.read_bits(static_cast<std::size_t>(ebits));
        if (!v) {
            return std::nullopt;
        }
        return std::pair<std::size_t, bool>{lower_bound + static_cast<std::size_t>(*v), false};
    }

    auto b0 = r.read_bits(8);
    if (!b0) {
        return std::nullopt;
    }
    auto value = static_cast<int>(*b0);

    if ((value & 0x80) == 0) {
        return std::pair<std::size_t, bool>{static_cast<std::size_t>(value & 0x7F), false};
    }
    if ((value & 0x40) == 0) {
        auto b1 = r.read_bits(8);
        if (!b1) {
            return std::nullopt;
        }
        auto len = (static_cast<std::size_t>(value & 0x3F) << 8) | static_cast<std::size_t>(*b1);
        return std::pair<std::size_t, bool>{len, false};
    }
    value &= 0x3F;
    if (value < 1 || value > 4) {
        return std::nullopt;
    }
    auto chunk = static_cast<std::size_t>(16384U) * static_cast<std::size_t>(value);
    return std::pair<std::size_t, bool>{chunk, true};
}

std::optional<std::size_t> normally_small_length(per_reader &r) {
    auto h = r.read_bits(1);
    if (!h) {
        return std::nullopt;
    }
    if (*h == 0) {
        auto v = r.read_bits(6);
        if (!v) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(*v) + 1U;
    }
    auto ld = length_determinant(r, -1, 0);
    if (!ld || ld->second) {
        return std::nullopt;
    }
    return ld->first;
}

std::optional<std::uint64_t> normally_small_non_negative_whole_number(per_reader &r) {
    auto ov = r.read_bits(7);
    if (!ov) {
        return std::nullopt;
    }
    int value = static_cast<int>(*ov);
    if ((value & 64) == 0) {
        return static_cast<std::uint64_t>(value);
    }
    value &= 63;
    value <<= 2;
    auto t = r.read_bits(2);
    if (!t) {
        return std::nullopt;
    }
    value |= static_cast<int>(*t);
    if (value & 128) {
        return std::nullopt;
    }
    if (value == 0) {
        return 0;
    }
    if (value >= 3) {
        return std::nullopt;
    }
    return r.read_bits(static_cast<std::size_t>(8 * value));
}

std::intmax_t twos_complement_bytes(std::string_view bytes) {
    if (bytes.empty()) {
        return 0;
    }
    if (bytes.size() > 8) {
        return std::numeric_limits<std::intmax_t>::max();
    }
    std::uint64_t u = 0;
    for (unsigned char c : bytes) {
        u = (u << 8U) | static_cast<std::uint64_t>(c);
    }
    auto const nbits = static_cast<unsigned>(bytes.size() * 8U);
    auto const sext  = u << (64U - nbits);
    return static_cast<std::intmax_t>(static_cast<std::int64_t>(sext) >> (64U - nbits));
}

std::optional<std::intmax_t> integer_unconstrained(per_reader &r) {
    std::string acc;
    for (;;) {
        auto ld = length_determinant(r, -1, 0);
        if (!ld) {
            return std::nullopt;
        }
        auto len    = ld->first;
        bool repeat = ld->second;
        if (len * 8 > r.bits_remaining()) {
            return std::nullopt;
        }
        auto chunk = r.read_octets(len);
        if (!chunk) {
            return std::nullopt;
        }
        acc.append(*chunk);
        if (!repeat) {
            break;
        }
    }
    return twos_complement_bytes(acc);
}

std::optional<std::intmax_t> integer_semi_constrained(per_reader &r, std::intmax_t lb) {
    auto v = integer_unconstrained(r);
    if (!v) {
        return std::nullopt;
    }
    return *v + lb;
}

std::optional<std::intmax_t> integer_constrained_root(std::intmax_t lb, std::intmax_t ub, per_reader &r) {
    if (lb > ub) {
        return std::nullopt;
    }
    auto const range = static_cast<std::uint64_t>(ub - lb + 1);
    int const  nbits = per_range_bit_width(range);
    if (nbits == 0) {
        return lb;
    }
    auto raw = constrained_whole_number(r, nbits);
    if (!raw || *raw >= range) {
        return std::nullopt;
    }
    return lb + static_cast<std::intmax_t>(*raw);
}

std::optional<std::intmax_t> integer_constrained_extensible(std::intmax_t lb, std::intmax_t ub, per_reader &r) {
    auto ext = r.read_bit();
    if (!ext) {
        return std::nullopt;
    }
    if (!*ext) {
        return integer_constrained_root(lb, ub, r);
    }
    return integer_unconstrained(r);
}

std::optional<std::uint32_t> enumerated_decode(per_reader &r, std::uint32_t n_root, bool extensible) {
    if (extensible) {
        auto eb = r.read_bit();
        if (!eb) {
            return std::nullopt;
        }
        if (*eb) {
            auto v = integer_unconstrained(r);
            if (!v || *v < 0) {
                return std::nullopt;
            }
            return static_cast<std::uint32_t>(*v);
        }
    }
    if (n_root < 1) {
        return std::nullopt;
    }
    auto nbits = per_range_bit_width(static_cast<std::uint64_t>(n_root));
    auto idx   = constrained_whole_number(r, nbits);
    if (!idx || *idx >= n_root) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(*idx);
}

std::optional<std::vector<bool>> bit_string_unconstrained(per_reader &r) {
    std::vector<bool> out;
    for (;;) {
        auto ld = length_determinant(r, -1, 0);
        if (!ld) {
            return std::nullopt;
        }
        auto nbits = ld->first;
        bool rep   = ld->second;
        if (nbits > r.bits_remaining()) {
            return std::nullopt;
        }
        for (std::size_t i = 0; i < nbits; ++i) {
            auto b = r.read_bit();
            if (!b) {
                return std::nullopt;
            }
            out.push_back(*b);
        }
        if (!rep) {
            break;
        }
    }
    return out;
}

std::optional<std::string> octet_string_unconstrained(per_reader &r) {
    std::string acc;
    for (;;) {
        auto ld = length_determinant(r, -1, 0);
        if (!ld) {
            return std::nullopt;
        }
        auto len = ld->first;
        bool rep = ld->second;
        if (len * 8 > r.bits_remaining()) {
            return std::nullopt;
        }
        auto chunk = r.read_octets(len);
        if (!chunk) {
            return std::nullopt;
        }
        acc.append(*chunk);
        if (!rep) {
            break;
        }
    }
    return acc;
}

std::optional<std::string> octet_string_unconstrained_aligned(per_reader &r) {
    r.align_to_octet();
    return octet_string_unconstrained(r);
}

std::optional<std::intmax_t> integer_unconstrained_aligned(per_reader &r) {
    r.align_to_octet();
    return integer_unconstrained(r);
}

std::optional<std::uint32_t> choice_root_index(per_reader &r, std::uint32_t n_alternatives) {
    return enumerated_decode(r, n_alternatives, false);
}

std::optional<std::vector<bool>> component_presence_bits(per_reader &r, std::size_t n) {
    std::vector<bool> out;
    out.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        auto b = r.read_bit();
        if (!b) {
            return std::nullopt;
        }
        out.push_back(*b);
    }
    return out;
}

std::optional<double> real_unconstrained(per_reader &r) {
    auto s = octet_string_unconstrained(r);
    if (!s) {
        return std::nullopt;
    }
    return ber_real_body_to_double(s->data(), s->size());
}

std::optional<double> real_unconstrained_aligned(per_reader &r) {
    r.align_to_octet();
    return real_unconstrained(r);
}

std::optional<std::string> object_identifier_unconstrained(per_reader &r) { return octet_string_unconstrained(r); }

std::optional<std::string> object_identifier_unconstrained_aligned(per_reader &r) {
    r.align_to_octet();
    return object_identifier_unconstrained(r);
}

} // namespace per_codec
} // namespace libasn
