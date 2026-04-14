#pragma once

#include <cctype>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

#include <libasn/common.h>
#include <libasn/encoding.h>
#include <libasn/encoding_rules.h>
#include <libasn/identifier.h>
#include <libasn/packet_length.h>
#include <libasn/tag_class.h>

#include <libasn/types/bool.h>
#include <libasn/types/choice.h>
#include <libasn/types/enum.h>
#include <libasn/types/explicit.h>
#include <libasn/types/integer.h>
#include <libasn/types/null.h>
#include <libasn/types/optional.h>
#include <libasn/types/real.h>
#include <libasn/types/sequence.h>
#include <libasn/types/sequence_of.h>
#include <libasn/types/string.h>

namespace libasn {
namespace detail {

template <encoding_rules_enum Rules, typename Identifier, typename Serde>
struct tlv_common_type;

template <encoding_rules_enum Rules, typename Identifier, typename Serde>
struct tlv_common_type_base {
    Identifier _identifier;
    Serde      _serde;

    explicit constexpr tlv_common_type_base(Identifier identifier, Serde serde)
        : _identifier(std::forward<Identifier>(identifier))
        , _serde(std::forward<Serde>(serde)) {}

    template <auto tag_number>
    constexpr auto context_specific() const {
        auto id = _identifier.template tagged<tag_class_enum::CONTEXT_SPECIFIC, tag_number, Rules>();
        return tlv_common_type<Rules, std::decay_t<decltype(id)>, Serde>(std::move(id), _serde);
    }

    template <auto tag_number>
    constexpr auto application() const {
        auto id = _identifier.template tagged<tag_class_enum::APPLICATION, tag_number, Rules>();
        return tlv_common_type<Rules, std::decay_t<decltype(id)>, Serde>(std::move(id), _serde);
    }

    template <typename Reader>
    auto read(Reader &reader) const -> decltype(_serde.read(std::declval<Reader &>())) {
        auto const st         = reader;
        auto       identifier = Identifier::read(reader);
        if (!identifier) {
            return std::nullopt;
        }
        if (this->_identifier != *identifier) {
            reader = std::forward<decltype(st)>(st);
            return std::nullopt;
        }
        auto length = packet_length::read(reader);
        if (!length || length->is_undefine()) {
            return std::nullopt;
        }

        auto serde = reader.read(*length->length);
        if (!serde) {
            return std::nullopt;
        }

        auto bytes_reader = Reader{*serde};
        auto value        = _serde.read(bytes_reader);
        if (!value || !bytes_reader.empty()) {
            return std::nullopt;
        }

        return *value;
    }
};

template <typename Identifier, typename Serde>
struct tlv_common_type<encoding_rules_enum::BER, Identifier, Serde>
    : tlv_common_type_base<encoding_rules_enum::BER, Identifier, Serde> {
    using tlv_common_type_base<encoding_rules_enum::BER, Identifier, Serde>::tlv_common_type_base;
};

template <typename Identifier, typename Serde>
struct tlv_common_type<encoding_rules_enum::DER, Identifier, Serde>
    : tlv_common_type_base<encoding_rules_enum::DER, Identifier, Serde> {
    using tlv_common_type_base<encoding_rules_enum::DER, Identifier, Serde>::tlv_common_type_base;

    template <auto tag_number>
    constexpr auto specific_application() const {
        constexpr auto id =
            this->_identifier
                .template tagged<tag_class_enum::SPECIFIC_APPLICATION, tag_number, encoding_rules_enum::DER>();
        return tlv_common_type<encoding_rules_enum::DER, std::decay_t<decltype(id)>, Serde>(std::move(id),
                                                                                            this->_serde);
    }
};

template <encoding_rules_enum Rules>
struct tlv_codec {
    template <typename I, typename S>
    using common_type = tlv_common_type<Rules, I, S>;

    template <encoding_enum encoding, auto tag_number, typename SerdeType>
    static constexpr auto type(SerdeType &&serde) {
        return common_type<static_identifier<encoding, tag_class_enum::UNIVERSAL, tag_number, Rules>,
                           std::decay_t<SerdeType>>(
            static_identifier<encoding, tag_class_enum::UNIVERSAL, tag_number, Rules>{},
            std::forward<SerdeType>(serde));
    }

    template <typename T>
    static constexpr auto optional(T &&t) {
        return libasn::internal::optional_type<std::decay_t<T>>(std::forward<T>(t));
    }

    template <typename TagNumber = int>
    static constexpr auto choice() {
        std::tuple ts;
        return libasn::internal::choice_type<TagNumber, decltype(ts)>(std::move(ts));
    }

    template <typename T>
    static constexpr auto explicit_(T &&t) {
        return type<encoding_enum::CONSTRUCTED, 0x00>(
            libasn::internal::explicit_type<std::decay_t<T>>(std::forward<T>(t)));
    }

    static constexpr auto boolean           = type<encoding_enum::PRIMITIVE, 0x01>(libasn::internal::boolean_type());
    static constexpr auto integer           = type<encoding_enum::PRIMITIVE, 0x02>(libasn::internal::integer_type());
    static constexpr auto bit_string        = type<encoding_enum::PRIMITIVE, 0x03>(libasn::internal::string_type());
    static constexpr auto octet_string      = type<encoding_enum::PRIMITIVE, 0x04>(libasn::internal::string_type());
    static constexpr auto null              = type<encoding_enum::PRIMITIVE, 0x05>(libasn::internal::null_type());
    static constexpr auto real              = type<encoding_enum::PRIMITIVE, 0x09>(libasn::internal::real_type());
    static constexpr auto object_identifier = type<encoding_enum::PRIMITIVE, 0x06>(libasn::internal::string_type());

    template <typename Enum>
    static constexpr auto enumerated() {
        return type<encoding_enum::PRIMITIVE, 0x0a>(libasn::internal::enum_type<Enum>());
    }

    static constexpr auto utf_string = type<encoding_enum::PRIMITIVE, 0x0C>(libasn::internal::string_type());

    template <typename... ElementsType>
    static constexpr auto sequence(ElementsType &&...elements) {
        return type<encoding_enum::CONSTRUCTED, 0x10>(
            libasn::internal::sequence_type<std::decay_t<ElementsType>...>(std::forward<ElementsType>(elements)...));
    }

    template <typename T>
    static constexpr auto sequence_of(T &&t) {
        return type<encoding_enum::CONSTRUCTED, 0x10>(
            libasn::internal::sequence_of_type<std::decay_t<decltype(t)>>(std::forward<T>(t)));
    }

    template <typename T>
    static constexpr auto set_of(T &&t) {
        return type<encoding_enum::CONSTRUCTED, 0x11>(
            libasn::internal::sequence_of_type<std::decay_t<decltype(t)>>(std::forward<T>(t)));
    }

    static constexpr auto printable_string = type<encoding_enum::PRIMITIVE, 0x13>(libasn::internal::string_type());
    static constexpr auto ia5_string       = type<encoding_enum::PRIMITIVE, 0x16>(libasn::internal::string_type());
    static constexpr auto utc_time         = type<encoding_enum::PRIMITIVE, 0x17>(libasn::internal::string_type());
    static constexpr auto generalized_time = type<encoding_enum::PRIMITIVE, 0x18>(libasn::internal::string_type());
    static constexpr auto graphic_string   = type<encoding_enum::PRIMITIVE, 0x19>(libasn::internal::string_type());
    static constexpr auto visible_string   = type<encoding_enum::PRIMITIVE, 0x1A>(libasn::internal::string_type());
    static constexpr auto general_string   = type<encoding_enum::PRIMITIVE, 0x1B>(libasn::internal::string_type());
};

} // namespace detail
} // namespace libasn
