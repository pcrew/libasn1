#pragma once

#include <libasn/per/per_reader.h>

namespace libasn {
namespace per_codec {

/** APER: which fields require octet alignment before decoding (X.691 + 3GPP alignment rules). */
enum class aper_field_kind {
    boolean,
    constrained_integer,
    enumerated_root,
    normally_small,
    choice_root,
    component_presence_bits,
    unconstrained_integer,
    octet_string_unconstrained,
    bit_string_unconstrained,
    real,
    object_identifier,
    open_type,
};

inline void aper_align_before_if_needed(per_reader &r, aper_field_kind k, bool is_aper) {
    if (!is_aper) {
        return;
    }
    switch (k) {
    case aper_field_kind::boolean:
    case aper_field_kind::constrained_integer:
    case aper_field_kind::enumerated_root:
    case aper_field_kind::normally_small:
    case aper_field_kind::choice_root:
    case aper_field_kind::component_presence_bits:
        return;
    case aper_field_kind::unconstrained_integer:
    case aper_field_kind::octet_string_unconstrained:
    case aper_field_kind::bit_string_unconstrained:
    case aper_field_kind::real:
    case aper_field_kind::object_identifier:
    case aper_field_kind::open_type:
        r.align_to_octet();
        return;
    }
}

inline constexpr bool aper_needs_octet_alignment_before(aper_field_kind k) {
    switch (k) {
    case aper_field_kind::boolean:
    case aper_field_kind::constrained_integer:
    case aper_field_kind::enumerated_root:
    case aper_field_kind::normally_small:
    case aper_field_kind::choice_root:
    case aper_field_kind::component_presence_bits:
        return false;
    case aper_field_kind::unconstrained_integer:
    case aper_field_kind::octet_string_unconstrained:
    case aper_field_kind::bit_string_unconstrained:
    case aper_field_kind::real:
    case aper_field_kind::object_identifier:
    case aper_field_kind::open_type:
        return true;
    }
}

} // namespace per_codec
} // namespace libasn
