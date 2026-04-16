#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <libasn/per/detail/x691.h>

namespace libasn {

struct per_choice_extension {
    std::uint64_t extension_index{};
    std::string   open_type{};
};

namespace internal {

template <std::size_t I, typename T>
struct per_choice_arm {
    T value;
};

template <typename Reader, typename C>
using per_codec_value_t =
    std::decay_t<typename decltype(std::declval<const C &>().read(std::declval<Reader &>()))::value_type>;

template <typename Reader, typename Tuple, typename IndexSeq>
struct per_choice_variant_impl;

template <typename Reader, typename... Cs, std::size_t... Is>
struct per_choice_variant_impl<Reader, std::tuple<Cs...>, std::index_sequence<Is...>> {
    using type = std::variant<per_choice_arm<Is, per_codec_value_t<Reader, Cs>>...>;
};

template <typename Reader, typename... Cs>
using per_choice_variant_t =
    typename per_choice_variant_impl<Reader, std::tuple<Cs...>, std::index_sequence_for<Cs...>>::type;

template <auto E, typename Codec>
struct per_choice_alt {
    static constexpr decltype(E) discriminator = E;
    using codec_type                           = Codec;
    Codec codec{};
};

template <typename>
struct per_alt_codec;

template <auto E, typename Codec>
struct per_alt_codec<per_choice_alt<E, Codec>> {
    using type = Codec;
};

template <typename Reader, typename... Alts>
using per_choice_variant_from_alts_t = per_choice_variant_t<Reader, typename per_alt_codec<Alts>::type...>;

template <typename Enum, typename Reader, typename... Alts>
using per_choice_root_pair_t = std::pair<Enum, per_choice_variant_from_alts_t<Reader, Alts...>>;

template <typename Enum, typename Reader, typename... Alts>
using per_choice_read_result_t = std::variant<per_choice_root_pair_t<Enum, Reader, Alts...>, per_choice_extension>;

template <typename Enum, bool Extensible, typename... Alts>
struct per_choice_type;

template <typename Enum, typename... Alts>
struct per_choice_type<Enum, false, Alts...> {
    std::tuple<Alts...> alts{};

    static_assert(sizeof...(Alts) > 0);

    explicit constexpr per_choice_type(std::tuple<Alts...> t)
        : alts(std::move(t)) {}

    template <auto E, typename C>
    constexpr auto with(C &&c) {
        return per_choice_type<Enum, false, Alts..., per_choice_alt<E, std::decay_t<C>>>(
            std::tuple_cat(alts, std::tuple(per_choice_alt<E, std::decay_t<C>>{std::forward<C>(c)})));
    }

    template <typename Reader>
    auto read(Reader &r) const -> std::optional<per_choice_root_pair_t<Enum, Reader, Alts...>> {
        auto idx = per_codec::choice_root_index(r, static_cast<std::uint32_t>(sizeof...(Alts)));
        if (!idx || *idx >= sizeof...(Alts)) {
            return std::nullopt;
        }
        return read_at<0>(r, *idx);
    }

private:
    template <std::size_t I, typename Reader>
    auto read_at(Reader &r, std::uint32_t idx) const -> std::optional<per_choice_root_pair_t<Enum, Reader, Alts...>> {
        if (idx != I) {
            if constexpr (I + 1 < sizeof...(Alts)) {
                return read_at<I + 1>(r, idx);
            }
            return std::nullopt;
        }
        using AltI = std::tuple_element_t<I, std::tuple<Alts...>>;
        auto v     = std::get<I>(alts).codec.read(r);
        if (!v) {
            return std::nullopt;
        }
        using Var = per_choice_variant_from_alts_t<Reader, Alts...>;
        using Val = per_codec_value_t<Reader, typename AltI::codec_type>;
        return per_choice_root_pair_t<Enum, Reader, Alts...>{
            static_cast<Enum>(AltI::discriminator), Var(std::in_place_index<I>, per_choice_arm<I, Val>{std::move(*v)})};
    }
};

template <typename Enum, typename... Alts>
struct per_choice_type<Enum, true, Alts...> {
    std::tuple<Alts...> alts{};

    static_assert(sizeof...(Alts) > 0);

    explicit constexpr per_choice_type(std::tuple<Alts...> t)
        : alts(std::move(t)) {}

    template <auto E, typename C>
    constexpr auto with(C &&c) {
        return per_choice_type<Enum, true, Alts..., per_choice_alt<E, std::decay_t<C>>>(
            std::tuple_cat(alts, std::tuple(per_choice_alt<E, std::decay_t<C>>{std::forward<C>(c)})));
    }

    template <typename Reader>
    auto read(Reader &r) const -> std::optional<per_choice_read_result_t<Enum, Reader, Alts...>> {
        auto eb = r.read_bit();
        if (!eb) {
            return std::nullopt;
        }
        if (*eb) {
            auto ext_idx = per_codec::normally_small_non_negative_whole_number(r);
            if (!ext_idx) {
                return std::nullopt;
            }
            auto open = per_codec::octet_string_unconstrained_aligned(r);
            if (!open) {
                return std::nullopt;
            }
            return per_choice_read_result_t<Enum, Reader, Alts...>{
                per_choice_extension{*ext_idx, std::move(*open)}
            };
        }
        auto idx = per_codec::choice_root_index(r, static_cast<std::uint32_t>(sizeof...(Alts)));
        if (!idx || *idx >= sizeof...(Alts)) {
            return std::nullopt;
        }
        auto root = read_at<0>(r, *idx);
        if (!root) {
            return std::nullopt;
        }
        return per_choice_read_result_t<Enum, Reader, Alts...>{std::move(*root)};
    }

private:
    template <std::size_t I, typename Reader>
    auto read_at(Reader &r, std::uint32_t idx) const -> std::optional<per_choice_root_pair_t<Enum, Reader, Alts...>> {
        if (idx != I) {
            if constexpr (I + 1 < sizeof...(Alts)) {
                return read_at<I + 1>(r, idx);
            }
            return std::nullopt;
        }
        using AltI = std::tuple_element_t<I, std::tuple<Alts...>>;
        auto v     = std::get<I>(alts).codec.read(r);
        if (!v) {
            return std::nullopt;
        }
        using Var = per_choice_variant_from_alts_t<Reader, Alts...>;
        using Val = per_codec_value_t<Reader, typename AltI::codec_type>;
        return per_choice_root_pair_t<Enum, Reader, Alts...>{
            static_cast<Enum>(AltI::discriminator), Var(std::in_place_index<I>, per_choice_arm<I, Val>{std::move(*v)})};
    }
};

template <typename Enum, bool Extensible>
struct per_choice_builder {
    template <auto E, typename C>
    constexpr auto with(C &&c) {
        return per_choice_type<Enum, Extensible, per_choice_alt<E, std::decay_t<C>>>(
            std::tuple(per_choice_alt<E, std::decay_t<C>>{std::forward<C>(c)}));
    }
};

} // namespace internal
} // namespace libasn
