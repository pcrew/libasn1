#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <variant>
#include <limits>
#include <cctype>

namespace libasn {

enum encoding_rules_enum {
    BER = 0,
    DER = 1,
};

} // namespace libasn
