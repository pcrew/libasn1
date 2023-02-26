#pragma once

namespace libasn {
namespace ber {
namespace internal {

template <typename TagNumber, typename Types, TagNumber... tag_numbers>
struct choice_type {
    Types types;

    explicit constexpr choice_type(Types types)
        : types(std::forward<Types>(types)) {}

    template <typename T>
    constexpr auto with(T &&type) {
        constexpr auto tag_number = decltype(type._identifier)::dynamic.tag_number();
        auto types                = std::tuple_cat(this->types, std::tuple(type));

        return choice_type<TagNumber, decltype(types), tag_numbers..., tag_number>(std::move(types));
    }

    template <TagNumber tag_number, typename T>
    constexpr auto with(T &&type) {
        static_assert(decltype(type._identifier)::dynamic.tag_class() == tag_class_enum::UNIVERSAL);
        return with(type.template context_specific<tag_number>());
    }

    static constexpr auto tag_number(size_t i) {
        constexpr TagNumber a[]{tag_numbers...};
        return a[i];
    }

    template <TagNumber tag_number, size_t i = 0>
    static constexpr auto index_of() {
        static_assert(i < sizeof...(tag_numbers), "tag number not found among choices");

        if constexpr (tag_number == choice_type::tag_number(i)) {
            return i;
        } else if constexpr (i < sizeof...(tag_numbers)) {
            return index_of<tag_number, i + 1>();
        }
    }

    template <TagNumber tag_number>
    constexpr auto &&type() const {
        return std::get<index_of<tag_number>()>(types);
    }

    template <TagNumber tag_number, typename... Args>
    constexpr auto make(Args &&...args) const {
        return type<tag_number>()(std::forward<Args>(args)...);
    }

    template <typename ValueType>
    constexpr auto operator()(ValueType &&value) const {
        return std::forward<ValueType>(value);
    }

    template <typename... Values>
    struct Read {
        using Variant = typename std::variant<Values...>;
        TagNumber tag_number;
        Variant value;

        template <size_t i, typename ValueType>
        static auto indexed(ValueType &&value) {
            return Read{choice_type::tag_number(i),
                        Variant(std::in_place_index_t<i>(), std::forward<ValueType>(value))};
        }

        bool operator==(Read const &other) const {
            return this->tag_number == other.tag_number && this->value == other.value;
        }

        template <TagNumber tag_number>
        auto &&get() const {
            return std::get<index_of<tag_number>()>(value);
        }
    };

    template <size_t i, typename... Values, typename Reader, typename Identifier>
    auto read_choices(Reader &reader, Identifier &&identifier) const {
        if constexpr (i < sizeof...(tag_numbers)) {
            auto &type  = std::get<i>(types);
            using Value = std::decay_t<decltype(*type.read(reader))>;
            using Res   = decltype(read_choices<i + 1, Values..., Value>(reader, identifier));
            using Read  = typename Res::value_type;

            if (type._identifier == identifier) {
                auto &&result = type._serde.read(reader);

                if (!result) {
                    return Res(std::nullopt);
                }

                return Res(Read::template indexed<i>(*std::forward<decltype(result)>(result)));
            } else {
                return read_choices<i + 1, Values..., Value>(reader, std::forward<Identifier>(identifier));
            }
        } else {
            return std::optional<Read<Values...>>(std::nullopt);
        }
    }

    template <typename Reader>
    auto read(Reader &reader) const
        -> decltype(read_choices<0>(reader, *dynamic_identifier<TagNumber>::read(std::declval<Reader &>()))) {
        auto identifier = dynamic_identifier<TagNumber>::read(reader);
        if (!identifier) {
            return std::nullopt;
        }

        auto length = packet_length::read(reader);
        if (!length || length->is_undefine()) {
            return std::nullopt;
        }

        auto bytes = reader.read(*length->length);
        if (!bytes) {
            return std::nullopt;
        }

        auto bytes_reader = Reader{*bytes};

        return read_choices<0>(bytes_reader, std::forward<decltype(*identifier)>(*identifier));
    }
};

}  // namespace internal
}  // namespace ber
}  // namespace libasn
