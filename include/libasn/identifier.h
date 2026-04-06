#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <variant>
#include <limits>
#include <cctype>

#include <libasn/encoding.h>
#include <libasn/encoding_rules.h>
#include <libasn/tag_class.h>

namespace libasn {

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
    encoding_enum  _encoding;
    tag_class_enum _tag_class;
    TagNumber      _tag_number;
};

template <encoding_enum encoding, tag_class_enum tag_class, auto tag_number,
          auto encoding_rule = encoding_rules_enum::BER>
struct static_identifier {
    constexpr static auto IS_BER{encoding_rule == encoding_rules_enum::BER};
    constexpr static auto IS_DER{encoding_rule == encoding_rules_enum::DER};

    constexpr static dynamic_identifier dynamic{encoding, tag_class, tag_number};

    template <tag_class_enum tag_class_new, auto tag_number_new, auto encoding_rule_new = encoding_rules_enum::BER>
    constexpr auto tagged() const {
        return static_identifier<encoding, tag_class_new, tag_number_new, encoding_rule_new>{};
    }

    template <typename Reader>
    static auto read(Reader &reader) {
        constexpr auto expected = static_identifier{};
        auto           actual   = decltype(dynamic)::read(reader);

        return actual && (expected == *actual) ? std::optional(expected) : std::nullopt;
    }

    bool operator==(static_identifier const &other) const { return true; }
    bool operator!=(static_identifier const &other) const { return false; }
    bool operator==(decltype(dynamic) const &other) const {
        return encoding == other.encoding() && tag_class == other.tag_class() && tag_number == other.tag_number();
    }
    bool operator!=(decltype(dynamic) const &other) const { return !(*this == other); }
};

} // namespace libasn
