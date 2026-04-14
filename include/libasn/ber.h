#pragma once

#include <libasn/compiler.h>

#include <libasn/tlv.h>

namespace libasn {
namespace ber {

using codec = detail::tlv_codec<encoding_rules_enum::BER>;

template <typename I, typename S>
using common_type = typename codec::template common_type<I, S>;

template <encoding_enum encoding, auto tag_number, typename SerdeType>
constexpr auto type(SerdeType &&serde) {
    return codec::template type<encoding, tag_number>(std::forward<SerdeType>(serde));
}

template <typename T>
constexpr auto optional(T &&t) {
    return codec::optional(std::forward<T>(t));
}

template <typename TagNumber = int>
constexpr auto choice() {
    return codec::template choice<TagNumber>();
}

template <typename T>
constexpr auto explicit_(T &&t) {
    return codec::explicit_(std::forward<T>(t));
}

template <typename Enum>
constexpr auto enumerated() {
    return codec::template enumerated<Enum>();
}

constexpr auto boolean           = codec::boolean;
constexpr auto integer           = codec::integer;
constexpr auto bit_string        = codec::bit_string;
constexpr auto octet_string      = codec::octet_string;
constexpr auto null              = codec::null;
constexpr auto real              = codec::real;
constexpr auto object_identifier = codec::object_identifier;
constexpr auto utf_string        = codec::utf_string;
constexpr auto printable_string  = codec::printable_string;
constexpr auto ia5_string        = codec::ia5_string;
constexpr auto utc_time          = codec::utc_time;
constexpr auto generalized_time  = codec::generalized_time;
constexpr auto graphic_string    = codec::graphic_string;
constexpr auto visible_string    = codec::visible_string;
constexpr auto general_string    = codec::general_string;

template <typename... ElementsType>
constexpr auto sequence(ElementsType &&...elements) {
    return codec::sequence(std::forward<ElementsType>(elements)...);
}

template <typename T>
constexpr auto sequence_of(T &&t) {
    return codec::sequence_of(std::forward<T>(t));
}

template <typename T>
constexpr auto set_of(T &&t) {
    return codec::set_of(std::forward<T>(t));
}

} // namespace ber
} // namespace libasn
