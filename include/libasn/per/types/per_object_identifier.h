#pragma once

#include <optional>
#include <string>

#include <libasn/per/detail/x691.h>

namespace libasn {
namespace internal {

/** PER OID как OCTET STRING (тело BER); список дуг: `ber::object_identifier_components`. */
struct per_object_identifier_type {
    bool aligned{false};

    explicit constexpr per_object_identifier_type(bool ap = false)
        : aligned(ap) {}

    template <typename Reader>
    std::optional<std::string> read(Reader &reader) const {
        if (aligned) {
            return per_codec::object_identifier_unconstrained_aligned(reader);
        }
        return per_codec::object_identifier_unconstrained(reader);
    }
};

} // namespace internal
} // namespace libasn
