
#pragma once

#include <libasn/der.h>

// TODO: implement der api
namespace libasn {
namespace k5 {

enum protocol_op_enum {
    TICKET            = 1,
    AUTHENTICATOR     = 2,
    ENC_TICKET_PART   = 3,
    AS_REQ            = 10,
    AS_REP            = 11,
    TGS_REQ           = 12,
    TGS_REP           = 13,
    AP_REQ            = 14,
    AP_REP            = 15,
    KRB_SAFE          = 20,
    KRB_PRIV          = 21,
    KRB_CRED          = 22,
    ENC_AS_REP_PART   = 25,
    ENC_TGS_REP_PART  = 26,
    ENC_AP_REP_PART   = 27,
    ENC_KRB_PRIV_PART = 28,
    ENC_KRB_CRED_PART = 29,
    KRB_ERROR         = 30,
};

constexpr auto uint32          = ber::integer;
constexpr auto int32           = ber::integer;
constexpr auto microseconds    = ber::integer;
constexpr auto kerberos_string = ber::general_string;
constexpr auto c_name_string   = ber::general_string;
constexpr auto s_name_string   = ber::general_string;
constexpr auto realm           = kerberos_string;

constexpr auto principal_name = ber::sequence(
    /* name-type */ int32.context_specific<0>(),
    /* name-string */ ber::sequence_of(kerberos_string).context_specific<1>());

constexpr auto c_name = principal_name;
constexpr auto s_name = principal_name;

#if 0
constexpr auto kerberos_time = ber::generalized_time;
#else
constexpr auto kerberos_time = ber::general_string;
#endif

constexpr auto host_address = ber::sequence(
    /* addr-type */ int32.context_specific<0>(),
    /* address */ ber::octet_string.context_specific<1>());

constexpr auto host_addresses = ber::sequence_of(host_address);

constexpr auto authorization_data = ber::sequence_of(ber::sequence(
    /* ad-type */ int32.context_specific<0>(),
    /* ad-data */ ber::octet_string.context_specific<1>()));

constexpr auto pa_data = ber::sequence(
    /* padata-type */ int32.context_specific<1>(),
    /* padata-value */ ber::octet_string.context_specific<2>());

constexpr auto kerberos_flags = ber::bit_string;

constexpr auto encrypted_data = ber::sequence(
    /* etype */ int32.context_specific<0>(),
    /* kvno */ uint32.context_specific<1>(),
    /* cipher */ ber::octet_string.context_specific<2>());

constexpr auto encryption_key = ber::sequence(
    /* keytype */ int32.context_specific<0>(),
    /* keyvalue */ ber::octet_string.context_specific<1>());

constexpr auto checksum = ber::sequence(
    /* cksumtype */ int32.context_specific<0>(),
    /* checksum */ ber::octet_string.context_specific<1>());

constexpr auto ticket = ber::sequence(
                            /* tkt-vno */ ber::integer.context_specific<0>(),
                            /* realm */ realm.context_specific<1>(),
                            /* sname */ principal_name.context_specific<2>(),
                            /* enc-part */ encrypted_data.context_specific<3>())
                            .application<protocol_op_enum::TICKET>();

constexpr auto transited_encoding = ber::sequence(
    /* tr-type */ int32.context_specific<0>(),
    /* contents */ ber::octet_string.context_specific<1>());

constexpr auto enc_ticket_part = ber::sequence(
    /* flags */ kerberos_flags.context_specific<0>(),
    /* key */ encryption_key.context_specific<1>(),
    /* crealm */ realm.context_specific<2>(),
    /* cname */ principal_name.context_specific<3>(),
    /* transited */ transited_encoding.context_specific<4>(),
    /* authtime */ kerberos_time.context_specific<5>(),
    /* starttime */ ber::optional(kerberos_time.context_specific<6>()),
    /* endtime */ kerberos_time.context_specific<7>(),
    /* renew-till */ ber::optional(kerberos_time.context_specific<8>()),
    /* caddr */ ber::optional(host_addresses.context_specific<9>()),
    /* authorization-data */
    ber::optional(authorization_data.context_specific<10>()));

constexpr auto kdc_req_body = ber::sequence(
    /* kdc-options */ kerberos_flags.context_specific<0>(),
    /* cname */ ber::optional(principal_name.context_specific<1>()),
    /* realm */ realm.context_specific<2>(),
    /* sname */ ber::optional(principal_name.context_specific<3>()),
    /* from */ ber::optional(kerberos_time.context_specific<4>()),
    /* till */ kerberos_time.context_specific<5>(),
    /* rtime */ ber::optional(kerberos_time.context_specific<6>()),
    /* nonce */ uint32.context_specific<7>(),
    /* etype */ ber::sequence_of(uint32).context_specific<8>(),
    /* addresses */ ber::optional(host_addresses.context_specific<9>()),
    /* enc-authorization-data */
    ber::optional(encrypted_data.context_specific<10>()),
    /* additional-tickets */
    ber::optional(ber::sequence_of(ticket).context_specific<11>()));

constexpr auto kdc_req = ber::sequence(
    /* pvno */ ber::integer.context_specific<1>(),
    /* msg-type */ ber::integer.context_specific<2>(),
    /* padata */ ber::optional(ber::sequence_of(pa_data).context_specific<3>()),
    /* req-body */ kdc_req_body.context_specific<4>());

constexpr auto as_req = kdc_req.application<protocol_op_enum::AS_REQ>();

} // namespace k5
} // namespace libasn
