#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include <libasn/per/codec.h>
#include <libasn/per/per_reader.h>
#include <libasn/per/types/per_bool.h>
#include <libasn/per/types/per_choice.h>
#include <libasn/per/types/per_enumerated.h>
#include <libasn/per/types/per_integer.h>
#include <libasn/per/types/per_object_identifier.h>
#include <libasn/per/types/per_octet_string.h>
#include <libasn/per/types/per_octet_string_unconstrained.h>
#include <libasn/per/types/per_real.h>
#include <libasn/per/types/per_sequence_of.h>
#include <libasn/per/types/per_unconstrained_integer.h>
#include <libasn/types/sequence.h>

namespace libasn {
namespace per {

template <std::intmax_t Lo, std::intmax_t Hi>
constexpr auto integer() {
    return internal::per_constrained_integer_type<Lo, Hi>{};
}

template <std::intmax_t Lo, std::intmax_t Hi>
constexpr auto integer_extensible(bool aligned_aper = false) {
    return internal::per_constrained_integer_extensible_type<Lo, Hi>{aligned_aper};
}

template <std::intmax_t LowerBound>
constexpr auto integer_semi(bool aligned_aper = false) {
    return internal::per_semi_constrained_integer_type<LowerBound>{aligned_aper};
}

inline constexpr internal::per_unconstrained_integer_type integer_unconstrained{};
inline constexpr internal::per_unconstrained_integer_type integer_unconstrained_aper{true};

inline constexpr internal::per_boolean_type boolean{};

template <std::size_t MinLen, std::size_t MaxLen>
constexpr auto octet_string() {
    return internal::per_constrained_octet_string_type<MinLen, MaxLen>{};
}

inline constexpr internal::per_unconstrained_octet_string_type octet_string_unconstrained{};
inline constexpr internal::per_unconstrained_octet_string_type octet_string_unconstrained_aper{true};

inline constexpr internal::per_real_type real{};
inline constexpr internal::per_real_type real_aper{true};

inline constexpr internal::per_object_identifier_type object_identifier{};
inline constexpr internal::per_object_identifier_type object_identifier_aper{true};

template <typename Enum, std::uint32_t NRoot, bool Extensible = false>
constexpr auto enumerated() {
    return internal::per_enumerated_type<Enum, NRoot, Extensible>{};
}

template <typename Enum, bool Extensible = false>
constexpr auto choice() {
    return internal::per_choice_builder<Enum, Extensible>{};
}

template <typename... ElementsType>
constexpr auto sequence(ElementsType &&...elements) {
    return internal::sequence_type<std::decay_t<ElementsType>...>(std::forward<ElementsType>(elements)...);
}

template <std::size_t MinN, std::size_t MaxN, typename T>
constexpr auto sequence_of(T &&elem) {
    return internal::per_sequence_of_type<MinN, MaxN, std::decay_t<T>>(std::forward<T>(elem));
}

} // namespace per
} // namespace libasn
