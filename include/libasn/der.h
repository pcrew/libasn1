#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <variant>
#include <limits>
#include <cctype>
#include <iostream>
#include <stdio.h>

#include <libasn/common.h>
#include <libasn/tag_class.h>
#include <libasn/encoding.h>
#include <libasn/packet_length.h>
#include <libasn/identifier.h>
#include <libasn/encoding_rules.h>

#include <libasn/types/null.h>
#include <libasn/types/bool.h>
#include <libasn/types/string.h>
#include <libasn/types/integer.h>
#include <libasn/types/enum.h>
#include <libasn/types/choice.h>
#include <libasn/types/optional.h>
#include <libasn/types/sequence.h>
#include <libasn/types/sequence_of.h>
#include <libasn/types/explicit.h>
#include <libasn/types/real.h>

namespace libasn {
namespace der {

template <typename Identifier, typename Serde>
struct common_type {
    Identifier _identifier;
    Serde _serde;

    explicit constexpr common_type(Identifier identifier, Serde serde)
        : _identifier(std::forward<Identifier>(identifier))
        , _serde(std::forward<Serde>(serde)) {}

    template <auto tag_number>
    constexpr auto context_specific() const {
        return der::common_type(
            _identifier.template tagged<tag_class_enum::CONTEXT_SPECIFIC, tag_number, encoding_rules_enum::DER>(),
            _serde);
    }

    template <auto tag_number>
    constexpr auto application() const {
        return der::common_type(
            _identifier.template tagged<tag_class_enum::APPLICATION, tag_number, encoding_rules_enum::DER>(), _serde);
    }

    template <auto tag_number>
    constexpr auto specific_application() const {
        return der::common_type(
            _identifier.template tagged<tag_class_enum::SPECIFIC_APPLICATION, tag_number, encoding_rules_enum::DER>(),
            _serde);
    }

    template <typename Reader>
    auto read(Reader &reader) const -> decltype(_serde.read(std::declval<Reader &>())) {
        auto identifier = Identifier::read(reader);
        if (!identifier) {
            return std::nullopt;
        }

        if (this->_identifier != *identifier) {
            return std::nullopt;
        }
        auto length = packet_length::read(reader);
        if (!length || length->is_undefine()) {
            return std::nullopt;
        }

        uint64_t len{0};

        if constexpr (Identifier::IS_DER) {
            reader.read();

            auto length_der = packet_length::read(reader);
            if (!length_der || length_der->is_undefine()) {
                return std::nullopt;
            }

            len = *length_der->length;
        } else {
            len = *length->length;
        }

        auto serde = reader.read(len);
        if (!serde) {
            return std::nullopt;
        }

#if 0
        printf("\n\n\n");
        hexdump(serde->data(), serde->size(), std::cout);
        printf("\n\n\n");
#endif
        auto bytes_reader = Reader{*serde};
        auto value        = _serde.read(bytes_reader);
        if (!value || !bytes_reader.empty()) {
            return std::nullopt;
        }

        return *value;
    }
};
template <encoding_enum encoding, auto tag_number, typename SerdeType>
constexpr auto type(SerdeType &&serde) {
    return common_type(static_identifier<encoding, tag_class_enum::UNIVERSAL, tag_number>{},
                       std::forward<SerdeType>(serde));
}

template <typename T>
constexpr auto optional(T &&type) {
    return libasn::internal::optional_type<std::decay_t<T>>(std::forward<T>(type));
}

template <typename TagNumber = int>
constexpr auto choice() {
    std::tuple types;
    return libasn::internal::choice_type<TagNumber, decltype(types)>(std::move(types));
}

template <typename T>
constexpr auto explicit_(T &&type) {
    return der::type<encoding_enum::CONSTRUCTED, 0x00>(
        libasn::internal::explicit_type<std::decay_t<T>>(std::forward<T>(type)));
}

constexpr auto boolean           = type<encoding_enum::CONSTRUCTED, 0x01>(libasn::internal::boolean_type());
constexpr auto integer           = type<encoding_enum::CONSTRUCTED, 0x02>(libasn::internal::integer_type());
constexpr auto bit_string        = type<encoding_enum::CONSTRUCTED, 0x03>(libasn::internal::string_type());
constexpr auto octet_string      = type<encoding_enum::CONSTRUCTED, 0x04>(libasn::internal::string_type());
constexpr auto null              = type<encoding_enum::CONSTRUCTED, 0x05>(libasn::internal::null_type());
constexpr auto object_identifier = type<encoding_enum::CONSTRUCTED, 0x06>(libasn::internal::string_type());

template <typename Enum>
constexpr auto enumerated() {
    return type<encoding_enum::CONSTRUCTED, 0x0a>(libasn::internal::enum_type<Enum>());
}

constexpr auto utf_string = type<encoding_enum::CONSTRUCTED, 0x0C>(libasn::internal::string_type());

template <typename... ElementsType>
constexpr auto sequence(ElementsType &&...elements) {
    return type<encoding_enum::CONSTRUCTED, 0x10>(
        libasn::internal::sequence_type<std::decay_t<ElementsType>...>(std::forward<ElementsType>(elements)...));
}

template <typename T>
constexpr auto sequence_of(T &&type) {
    return der::type<encoding_enum::CONSTRUCTED, 0x10>(
        libasn::internal::sequence_of_type<std::decay_t<decltype(type)>>(std::forward<T>(type)));
}

template <typename T>
constexpr auto set_of(T &&type) {
    return der::type<encoding_enum::CONSTRUCTED, 0x11>(
        libasn::internal::sequence_of_type<std::decay_t<decltype(type)>>(std::forward<T>(type)));
}

constexpr auto printable_string = type<encoding_enum::CONSTRUCTED, 0x13>(libasn::internal::string_type());
constexpr auto ia5_string       = type<encoding_enum::CONSTRUCTED, 0x16>(libasn::internal::string_type());
constexpr auto utc_time         = type<encoding_enum::CONSTRUCTED, 0x17>(libasn::internal::string_type());
constexpr auto graphic_string   = type<encoding_enum::CONSTRUCTED, 0x17>(libasn::internal::string_type());
constexpr auto visible_string   = type<encoding_enum::CONSTRUCTED, 0x1A>(libasn::internal::string_type());
constexpr auto general_string   = type<encoding_enum::CONSTRUCTED, 0x1B>(libasn::internal::string_type());

}  // namespace der
}  // namespace libasn
