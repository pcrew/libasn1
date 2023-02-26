
#pragma once

#include <libasn/ber.h>

namespace libasn {
namespace rtse {

constexpr auto abort_reason                     = ber::integer;
constexpr auto additional_reference_information = ber::t61_string;
constexpr auto common_reference                 = ber::utc_time;

enum class calling_ss_user_reference_choice {
    T61_STRING   = 0,
    OCTET_STRING = 1,
};

constexpr auto calling_ss_user_reference = ber::choice<calling_ss_user_reference_choice>()
                                               .with<calling_ss_user_reference_choice::T61_STRIBNG>(ber::t61_string)
                                               .with<calling_ss_user_reference_choice::OCTET_STRING>(ber::octet_string);

constexpr auto refuse_reason == ber::integer;

constexpr auto session_connection_identifier = ber::sequence(
    /* callingSSuserReference */ callingSSuserReference,
    /* CommonReference */ common_reference,
    /* additionalReferenceInformation */ ber::optional(additional_reference_information.context_specific<0>()));

enum class connection_data_choice {
    OPEN    = 0,
    RECOVER = 1,
};

constexpr auto connection_data = ber::choice<connection_data_choice>()
                                     .with<connection_data_choice::OPEN>(ber::object_identifier)
                                     .with<connection_data_choice::RECOVER>(session_connection_identifier);

constexpr auto rtab_apdu = ber::set(
    /* abortReason */ ber::optional(abort_reason.context_specific<0>()),
    /* reflectedParameter */ ber::optional(ber::bit_string.context_specific<1>()),
    /* userdataAB */ ber::optional(ber::object_identifier.context_specific<2>()));

constexpr auto rttr_apdu = ber::octet_string;

constexpr auto rttp_apdu = ber::integer;

constexpr auto rtorj_apdu = ber::set(
    /* refuseReason */ ber::optional(refuse_reason.context_specific<0>()),
    /* userDataRJ */ ber::optional(ber::object_identifier.context_specific<1>()));

constexpr auto rtorq_apdu = ber::set(
    /* checkpointSize */ ber::integer.context_specific<0>(),
    /* windowSize */ ber::integer.context_specific<1>(),
    /* dialogueMode */ ber::integer.context_specific<2>(),
    /* connectionDataRQ */ connection_data.context_specific<3>(),
    /* applicationProtocol */ ber::integer.context_specific<4>());

enum class rtse_apdus_choice {
    RTORQ_APDU = 16,
    RTOAC_APDU = 17,
    RTORJ_APDU = 18,
    RTAB_APDU  = 22,
};

constexpr auto rtse_apdus = ber::choice<rtse_apdus_choice>()
                                .with<rtse_apdus_choice::RTORQ_APDU>(rtorq_apdu)
                                .with<rtse_apdus_choice::RTOAC_APDU>(rtoac_apdu)
                                .with<rtse_apdus_choice::RTORJ_APDU>(rtorj_apdu)
                                .with<rtse_apdus_choice::Rtab_APDU>(rtab_apdu)
                                .with(rttp_apdu)
                                .with(rtab_apdu);

}  // namespace rtse
}  // namespace libasn
