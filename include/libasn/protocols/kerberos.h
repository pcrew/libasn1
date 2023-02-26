
#pragma once

#include <libasn/der.h>

// TODO: implement der api
namespace libasn {
namespace k5 {

enum protocol_op_enum {
    TICKET          = 1,
    ENC_TICKET_PART = 3,
    AS_REQ          = 10,
    TGS_REQ         = 12,
    AS_REP          = 11,
    TGS_REP         = 13,
};

constexpr auto uint32          = der::integer;
constexpr auto int32           = der::integer;
constexpr auto microseconds    = der::integer;
constexpr auto kerberos_string = der::general_string;
constexpr auto realm           = kerberos_string;

constexpr auto principal_name = der::sequence(
    /* name-type */ int32.context_specific<0>(),
    /* name-string */ der::sequence_of(kerberos_string).context_specific<1>());

#if 0
constexpr auto kerberos_time = der::generalized_time;
#else
constexpr auto kerberos_time = der::general_string;
#endif

constexpr auto host_address = der::sequence(
    /* addr-type */ int32.context_specific<0>(),
    /* address */ der::octet_string.context_specific<1>());

constexpr auto host_addresses = der::sequence_of(host_address);

constexpr auto authorization_data = der::sequence_of(der::sequence(
    /* ad-type */ int32.context_specific<0>(),
    /* ad-data */ der::octet_string.context_specific<1>()));

constexpr auto pa_data = der::sequence(
    /* padata-type */ int32.context_specific<1>(),
    /* padata-value */ der::octet_string.context_specific<2>());

constexpr auto kerberos_flags = der::bit_string;

constexpr auto encrypted_data = der::sequence(
    /* etype */ int32.context_specific<0>(),
    /* kvno */ uint32.context_specific<1>(),
    /* cipher */ der::octet_string.context_specific<2>());

constexpr auto encryption_key = der::sequence(
    /* keytype */ int32.context_specific<0>(),
    /* keyvalue */ der::octet_string.context_specific<1>());

constexpr auto checksum = der::sequence(
    /* cksumtype */ int32.context_specific<0>(),
    /* checksum */ der::octet_string.context_specific<1>());

constexpr auto ticket = der::sequence(
                            /* tkt-vno */ der::integer.context_specific<0>(),
                            /* realm */ realm.context_specific<1>(),
                            /* sname */ principal_name.context_specific<2>(),
                            /* enc-part */ encrypted_data.context_specific<3>())
                            .application<protocol_op_enum::TICKET>();

constexpr auto transited_encoding = der::sequence(
    /* tr-type */ int32.context_specific<0>(),
    /* contents */ der::octet_string.context_specific<1>());

constexpr auto enc_ticket_part = der::sequence(
    /* flags */ kerberos_flags.context_specific<0>(),
    /* key */ encryption_key.context_specific<1>(),
    /* crealm */ realm.context_specific<2>(),
    /* cname */ principal_name.context_specific<3>(),
    /* transited */ transited_encoding.context_specific<4>(),
    /* authtime */ kerberos_time.context_specific<5>(),
    /* starttime */ der::optional(kerberos_time.context_specific<6>()),
    /* endtime */ kerberos_time.context_specific<7>(),
    /* renew-till */ der::optional(kerberos_time.context_specific<8>()),
    /* caddr */ der::optional(host_addresses.context_specific<9>()),
    /* authorization-data */ der::optional(authorization_data.context_specific<10>()));

constexpr auto kdc_req_body = der::sequence(
    /* kdc-options */ kerberos_flags.context_specific<0>(),
    /* cname */ der::optional(principal_name.context_specific<1>()),
    /* realm */ realm.context_specific<2>(),
    /* sname */ der::optional(principal_name.context_specific<3>()),
    /* from */ der::optional(kerberos_time.context_specific<4>()),
    /* till */ kerberos_time.context_specific<5>(),
    /* rtime */ der::optional(kerberos_time.context_specific<6>()),
    /* nonce */ uint32.context_specific<7>(),
    /* etype */ der::sequence_of(uint32).context_specific<8>(),
    /* addresses */ der::optional(host_addresses.context_specific<9>()),
    /* enc-authorization-data */ der::optional(encrypted_data.context_specific<10>()),
    /* additional-tickets */ der::optional(der::sequence_of(ticket).context_specific<11>()));

constexpr auto kdc_req = der::sequence(
    /* pvno */ der::integer.context_specific<1>(),
    /* msg-type */ der::integer.context_specific<2>(),
    /* padata */ der::optional(der::sequence_of(pa_data).context_specific<3>()),
    /* req-body */ kdc_req_body.context_specific<4>());

constexpr auto as_req = kdc_req.application<protocol_op_enum::AS_REQ>();

}  // namespace k5
}  // namespace libasn

