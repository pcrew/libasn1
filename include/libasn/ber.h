#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <variant>
#include <limits>
#include <cctype>

namespace libasn {
namespace ber {

enum class tag_class_enum {
    UNIVERSAL        = 0,
    APPLICATION      = 1,
    CONTEXT_SPECIFIC = 2,
    PRIVATE          = 3,
};

enum class encoding_enum {
    PRIMITIVE   = 0,
    CONSTRUCTED = 1,
};

template <typename TagNumber>
struct dynamic_identifier {
    explicit constexpr dynamic_identifier(encoding_enum encoding, tag_class_enum tag_class, TagNumber tag_number)
        : _encoding(std::forward<encoding_enum>(encoding))
        , _tag_class(std::forward<tag_class_enum>(tag_class))
        , _tag_number(std::forward<TagNumber>(tag_number)) {}

    constexpr auto encoding() const { return _encoding; }
    constexpr auto tag_class() const { return _tag_class; }
    constexpr auto tag_number() const { return _tag_number; }

    template <typename Reader>
    static std::optional<dynamic_identifier> read(Reader &reader) {
        auto byte = reader.read();
        if (!byte) {
            return std::nullopt;
        }

        auto encoding   = static_cast<encoding_enum>((*byte & 0x20) >> 5);
        auto tag_class  = static_cast<tag_class_enum>((*byte & 0xC0) >> 6);
        auto tag_number = static_cast<TagNumber>((*byte & 0x1F));

        if (tag_number == static_cast<TagNumber>(0x1F)) {
            auto tag_number_int{0};
            do {
                byte = reader.read();
                if (!byte) {
                    return std::nullopt;
                }
                tag_number_int = (tag_number_int << 7) | (*byte & 0x7F);

            } while (*byte & 0x80);
            tag_number = static_cast<TagNumber>(std::move(tag_number_int));
        }

        return dynamic_identifier(encoding, tag_class, std::move(tag_number));
    }

private:
    encoding_enum _encoding;
    tag_class_enum _tag_class;
    TagNumber _tag_number;
};

template <encoding_enum encoding, tag_class_enum tag_class, auto tag_number>
struct static_identifier {
    constexpr static dynamic_identifier dynamic{encoding, tag_class, tag_number};

    template <tag_class_enum tag_class_new, auto tag_number_new>
    constexpr auto tagged() const {
        return static_identifier<encoding, tag_class_new, tag_number_new>{};
    }

    template <typename Reader>
    static auto read(Reader &reader) {
        constexpr auto expected = static_identifier{};
        auto actual             = decltype(dynamic)::read(reader);

        return actual && (expected == *actual) ? std::optional(expected) : std::nullopt;
    }

    bool operator==(static_identifier const &other) const { return true; }
    bool operator!=(static_identifier const &other) const { return false; }
    bool operator==(decltype(dynamic) const &other) const {
        return encoding == other.encoding() && tag_class == other.tag_class() && tag_number == other.tag_number();
    }
    bool operator!=(decltype(dynamic) const &other) const { return !(*this == other); }
};

struct packet_length {
    std::optional<std::size_t> length;
    explicit packet_length(decltype(length) &&length)
        : length(std::forward<decltype(length)>(length)) {}

    constexpr auto is_undefine() const { return !length.has_value(); }

    template <typename Reader>
    static std::optional<packet_length> read(Reader &reader) {
        auto byte = reader.read();
        if (!byte) {
            return std::nullopt;
        }

        auto form_len = (*byte & 0x80);
        if (form_len == 0) {
            return packet_length(*byte);
        }

        auto cnt = *byte & 0x7F;

        if (cnt == 0) {
            return packet_length(std::nullopt);
        }

        if (static_cast<size_t>(cnt) > sizeof(size_t)) {
            return std::nullopt;
        }

        size_t len{0};

        for (auto i = 0; i < cnt; ++i) {
            byte = reader.read();
            if (!byte) {
                return std::nullopt;
            }

            len = (len << 8) | *byte;
        }

        if (len == std::numeric_limits<std::size_t>::max()) {
            return std::nullopt;
        }

        return packet_length(len);
    }
};

template <typename Identifier, typename Serde>
struct common_type {
    Identifier _identifier;
    Serde _serde;

    explicit constexpr common_type(Identifier identifier, Serde serde)
        : _identifier(std::forward<Identifier>(identifier))
        , _serde(std::forward<Serde>(serde)) {}

    template <auto tag_number>
    constexpr auto context_specific() const {
        return ber::common_type(_identifier.template tagged<tag_class_enum::CONTEXT_SPECIFIC, tag_number>(), _serde);
    }

    template <auto tag_number>
    constexpr auto application() const {
        return ber::common_type(_identifier.template tagged<tag_class_enum::APPLICATION, tag_number>(), _serde);
    }

    template <typename Reader>
    auto read(Reader &reader) const -> decltype(_serde.read(std::declval<Reader &>())) {
        auto identifier = Identifier::read(reader);
        if (!identifier || this->_identifier != *identifier) {
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
template <encoding_enum encoding, auto tag_number, typename SerdeType>
constexpr auto type(SerdeType &&serde) {
    return common_type(static_identifier<encoding, tag_class_enum::UNIVERSAL, tag_number>{},
                       std::forward<SerdeType>(serde));
}

#include <libasn/ber_impl/null.h>
#include <libasn/ber_impl/bool.h>
#include <libasn/ber_impl/string.h>
#include <libasn/ber_impl/integer.h>
#include <libasn/ber_impl/enum.h>
#include <libasn/ber_impl/choice.h>
#include <libasn/ber_impl/optional.h>
#include <libasn/ber_impl/sequence.h>
#include <libasn/ber_impl/sequence_of.h>
#include <libasn/ber_impl/explicit.h>
#include <libasn/ber_impl/real.h>

template <typename T>
constexpr auto optional(T &&type) {
    return libasn::ber::internal::optional_type<std::decay_t<T>>(std::forward<T>(type));
}

template <typename TagNumber = int>
constexpr auto choice() {
    std::tuple types;
    return libasn::ber::internal::choice_type<TagNumber, decltype(types)>(std::move(types));
}

template <typename T>
constexpr auto explicit_(T &&type) {
    return ber::type<encoding_enum::CONSTRUCTED, 0x00>(
        libasn::ber::internal::explicit_type<std::decay_t<T>>(std::forward<T>(type)));
}

constexpr auto boolean           = type<encoding_enum::PRIMITIVE, 0x01>(libasn::ber::internal::boolean_type());
constexpr auto integer           = type<encoding_enum::PRIMITIVE, 0x02>(libasn::ber::internal::integer_type());
constexpr auto bit_string        = type<encoding_enum::PRIMITIVE, 0x03>(libasn::ber::internal::string_type());
constexpr auto octet_string      = type<encoding_enum::PRIMITIVE, 0x04>(libasn::ber::internal::string_type());
constexpr auto null              = type<encoding_enum::PRIMITIVE, 0x05>(libasn::ber::internal::null_type());
constexpr auto object_identifier = type<encoding_enum::PRIMITIVE, 0x06>(libasn::ber::internal::string_type());

template <typename Enum>
constexpr auto enumerated() {
    return type<encoding_enum::PRIMITIVE, 0x0a>(libasn::ber::internal::enum_type<Enum>());
}

constexpr auto utf_string = type<encoding_enum::PRIMITIVE, 0x0C>(libasn::ber::internal::string_type());

template <typename... ElementsType>
constexpr auto sequence(ElementsType &&...elements) {
    return type<encoding_enum::CONSTRUCTED, 0x10>(
        libasn::ber::internal::sequence_type<std::decay_t<ElementsType>...>(std::forward<ElementsType>(elements)...));
}

template <typename T>
constexpr auto sequence_of(T &&type) {
    return ber::type<encoding_enum::CONSTRUCTED, 0x10>(
        libasn::ber::internal::sequence_of_type<std::decay_t<decltype(type)>>(std::forward<T>(type)));
}

template <typename T>
constexpr auto set_of(T &&type) {
    return ber::type<encoding_enum::CONSTRUCTED, 0x11>(
        libasn::ber::internal::sequence_of_type<std::decay_t<decltype(type)>>(std::forward<T>(type)));
}

constexpr auto printable_string = type<encoding_enum::PRIMITIVE, 0x13>(libasn::ber::internal::string_type());
constexpr auto ia5_string       = type<encoding_enum::PRIMITIVE, 0x16>(libasn::ber::internal::string_type());
constexpr auto utc_time         = type<encoding_enum::PRIMITIVE, 0x17>(libasn::ber::internal::string_type());
constexpr auto graphic_string   = type<encoding_enum::PRIMITIVE, 0x17>(libasn::ber::internal::string_type());
constexpr auto visible_string   = type<encoding_enum::PRIMITIVE, 0x1A>(libasn::ber::internal::string_type());
constexpr auto general_string   = type<encoding_enum::PRIMITIVE, 0x1B>(libasn::ber::internal::string_type());

}  // namespace ber
}  // namespace libasn
