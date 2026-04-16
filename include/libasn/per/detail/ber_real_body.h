#pragma once

#include <cstddef>

#include <libasn/basic_reader.h>
#include <libasn/types/real.h>

namespace libasn {
namespace per_codec {

inline std::optional<double> ber_real_body_to_double(char const *octets, std::size_t n) {
    basic_reader r{{octets, n}};
    return internal::real_type{}.read(r);
}

} // namespace per_codec
} // namespace libasn
