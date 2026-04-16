#pragma once

#include <cstdint>

namespace libasn {
namespace internal {

/** Bit width to encode a value in `0 .. range_size - 1` (PER whole number). */
constexpr int per_range_bit_width(std::uint64_t range_size) {
    if (range_size <= 1) {
        return 0;
    }
    std::uint64_t const v = range_size - 1;
    int                 w = 0;
    auto                x = v;
    while (x != 0) {
        x >>= 1U;
        ++w;
    }
    return w;
}

} // namespace internal
} // namespace libasn
