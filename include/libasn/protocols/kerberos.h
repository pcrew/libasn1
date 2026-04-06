#pragma once

#include <libasn/ber.h>
#include <libasn/der.h>
#include <libasn/basic_reader.h>

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

constexpr auto uint32          = der::integer;
constexpr auto int32           = der::integer;
constexpr auto microseconds    = der::integer;
constexpr auto kerberos_string = der::general_string;
constexpr auto cname_string    = der::general_string;
constexpr auto sname_string    = der::general_string;
constexpr auto realm           = kerberos_string;

constexpr auto principal_name = der::sequence(
    /* name-type */ der::explicit_context_specific<0>(int32),
    /* name-string */ der::explicit_context_specific<1>(der::sequence_of(kerberos_string)));

constexpr auto cname = principal_name;
constexpr auto sname = principal_name;

constexpr auto kerberos_time = der::generalized_time;

namespace details {
constexpr auto encrypted_data = der::sequence(
    /* etype */ der::explicit_context_specific<0>(int32),
    /* kvno */ der::optional(der::explicit_context_specific<1>(uint32)),
    /* cipher */ der::explicit_context_specific<2>(der::octet_string));

#if 0
constexpr auto encrypted_data_ber = ber::sequence(
    /* etype */ ber::integer.context_specific<0>(),
    /* kvno */ ber::integer.context_specific<1>(),
    /* cipher */ ber::octet_string.context_specific<2>());
#endif
} // namespace details

constexpr auto encrypted_ticket_data        = details::encrypted_data;
constexpr auto encrypted_authorization_data = details::encrypted_data;
constexpr auto encrypted_authenticator      = details::encrypted_data;
constexpr auto encrypted_kdc_rep_data       = details::encrypted_data;
constexpr auto encrypted_arp_rep_data       = details::encrypted_data;
constexpr auto encrypted_krb_priv_data      = details::encrypted_data;
constexpr auto encrypted_krb_kred_data      = details::encrypted_data;
constexpr auto pa_enc_timestamp             = details::encrypted_data;

constexpr auto host_address = der::sequence(
    /* addr-type */ der::explicit_context_specific<0>(int32),
    /* address */ der::explicit_context_specific<1>(der::octet_string));

constexpr auto host_addresses = der::sequence_of(host_address);

constexpr auto authorization_data = der::sequence_of(der::sequence(
    /* ad-type */ der::explicit_context_specific<0>(int32),
    /* ad-data */ der::explicit_context_specific<1>(der::octet_string)));

constexpr auto pa_data = der::sequence(
    /* padata-type */ der::explicit_context_specific<1>(int32),
    /* padata-value */ der::explicit_context_specific<2>(der::octet_string));

constexpr auto typed_data_entry = der::sequence(
    /* data-type */ der::explicit_context_specific<1>(der::integer),
    /* data-value */ der::optional(der::explicit_context_specific<2>(der::octet_string)));

constexpr auto typed_data = der::sequence_of(typed_data_entry);

constexpr auto etype_info2_entry = der::sequence(
    /* etype */ der::explicit_context_specific<0>(der::integer),
    /* salt */ der::optional(der::explicit_context_specific<1>(kerberos_string)),
    /* s2kparams */ der::optional(der::explicit_context_specific<2>(der::octet_string)));

constexpr auto etype_info2 = der::sequence_of(etype_info2_entry);

constexpr auto kerberos_flags = der::bit_string;

constexpr auto encryption_key = der::sequence(
    /* keytype */ der::explicit_context_specific<0>(int32),
    /* keyvalue */ der::explicit_context_specific<1>(der::octet_string));

constexpr auto checksum = der::sequence(
    /* cksumtype */ der::explicit_context_specific<0>(int32),
    /* checksum */ der::explicit_context_specific<1>(der::octet_string));

/// Внутреннее тело Ticket (SEQUENCE); на проводе обычно обёрнуто в APPLICATION[1].
constexpr auto ticket_sequence = der::sequence(
    /* tkt-vno */ der::explicit_context_specific<0>(der::integer),
    /* realm */ der::explicit_context_specific<1>(realm),
    /* sname */ der::explicit_context_specific<2>(principal_name),
    /* enc-part */ der::explicit_context_specific<3>(encrypted_ticket_data));

constexpr auto ticket = der::explicit_application<protocol_op_enum::TICKET>(ticket_sequence);

constexpr auto pac_request = der::sequence(
    /* include-pac */ der::explicit_context_specific<0>(der::boolean));

constexpr auto transited_encoding = der::sequence(
    /* tr-type */ der::explicit_context_specific<0>(int32),
    /* contents */ der::explicit_context_specific<1>(der::octet_string));

constexpr auto enc_ticket_part = der::sequence(
    /* flags */ der::explicit_context_specific<0>(kerberos_flags),
    /* key */ der::explicit_context_specific<1>(encryption_key),
    /* crealm */ der::explicit_context_specific<2>(realm),
    /* cname */ der::explicit_context_specific<3>(principal_name),
    /* transited */ der::explicit_context_specific<4>(transited_encoding),
    /* authtime */ der::explicit_context_specific<5>(kerberos_time),
    /* starttime */ der::optional(der::explicit_context_specific<6>(kerberos_time)),
    /* endtime */ der::explicit_context_specific<7>(kerberos_time),
    /* renew-till */ der::optional(der::explicit_context_specific<8>(kerberos_time)),
    /* caddr */ der::optional(der::explicit_context_specific<9>(host_addresses)),
    /* authorization-data */
    der::optional(der::explicit_context_specific<10>(authorization_data)));

constexpr auto kdc_req_body = der::sequence(
    /* kdc-options */ der::explicit_context_specific<0>(kerberos_flags),
    /* cname */ der::optional(der::explicit_context_specific<1>(principal_name)),
    /* realm */ der::explicit_context_specific<2>(realm),
    /* sname */ der::optional(der::explicit_context_specific<3>(principal_name)),
    /* from */ der::optional(der::explicit_context_specific<4>(kerberos_time)),
    /* till */ der::optional(der::explicit_context_specific<5>(kerberos_time)),
    /* rtime */ der::optional(der::explicit_context_specific<6>(kerberos_time)),
    /* nonce */ der::explicit_context_specific<7>(uint32),
    /* etype */ der::explicit_context_specific<8>(der::sequence_of(uint32)),
    /* addresses */ der::optional(der::explicit_context_specific<9>(host_addresses)),
    /* enc-authorization-data */
    der::optional(der::explicit_context_specific<10>(details::encrypted_data)),
    /* additional-tickets */
    der::optional(der::explicit_context_specific<11>(der::sequence_of(ticket))));

constexpr auto kdc_req = der::sequence(
    /* pvno */ der::explicit_context_specific<1>(der::integer),
    /* msg-type */ der::explicit_context_specific<2>(der::integer),
    /* padata */ der::optional(der::explicit_context_specific<3>(der::sequence_of(pa_data))),
    /* req-body */ der::explicit_context_specific<4>(kdc_req_body));

constexpr auto kdc_rep = der::sequence(
    /* pvno */ der::explicit_context_specific<0>(der::integer),
    /* msg-type */ der::explicit_context_specific<1>(der::integer),
    /* padata */ der::optional(der::explicit_context_specific<2>(der::sequence_of(pa_data))),
    /* crealm */ der::explicit_context_specific<3>(realm),
    /* cname */ der::explicit_context_specific<4>(cname),
    /* ticket */ der::explicit_context_specific<5>(ticket),
    /* enc-part */ der::explicit_context_specific<6>(encrypted_kdc_rep_data));

constexpr auto authentificator = der::explicit_application<protocol_op_enum::AUTHENTICATOR>(der::sequence(
    /* authentificator-vno */ der::explicit_context_specific<0>(der::integer),
    /* crealm */ der::explicit_context_specific<1>(realm),
    /* cname */ der::explicit_context_specific<2>(principal_name),
    /* cksum */ der::optional(der::explicit_context_specific<3>(checksum)),
    /* cusec */ der::explicit_context_specific<4>(microseconds),
    /* ctime */ der::explicit_context_specific<5>(kerberos_time),
    /* subkey */ der::optional(der::explicit_context_specific<6>(encryption_key)),
    /* seq-number */ der::optional(der::explicit_context_specific<7>(uint32)),
    /* authorization-data */ der::optional(der::explicit_context_specific<8>(authorization_data))));

constexpr auto as_req  = der::explicit_application<protocol_op_enum::AS_REQ>(kdc_req);
constexpr auto as_rep  = der::explicit_application<protocol_op_enum::AS_REP>(kdc_rep);
constexpr auto tgs_req = der::explicit_application<protocol_op_enum::TGS_REQ>(kdc_req);
constexpr auto tgs_rep = der::explicit_application<protocol_op_enum::TGS_REP>(kdc_rep);
constexpr auto ap_req  = der::explicit_application<protocol_op_enum::AP_REQ>(der::sequence(
    /* pvno */ der::explicit_context_specific<0>(der::integer),
    /* msg-type */ der::explicit_context_specific<1>(der::integer),
    /* ap-options */ der::explicit_context_specific<2>(kerberos_flags),
    /* ticket */ der::explicit_context_specific<3>(ticket),
    /* authenticator */ der::explicit_context_specific<4>(encrypted_authenticator)));

constexpr auto krb_error = der::explicit_application<protocol_op_enum::KRB_ERROR>(der::sequence(
    /* pvno */ der::explicit_context_specific<0>(der::integer),
    /* msg-type */ der::explicit_context_specific<1>(der::integer),
    /* ctime */ der::optional(der::explicit_context_specific<2>(kerberos_time)),
    /* cusec */ der::optional(der::explicit_context_specific<3>(microseconds)),
    /* stime */ der::explicit_context_specific<4>(kerberos_time),
    /* susec */ der::explicit_context_specific<5>(microseconds),
    /* error-code */ der::explicit_context_specific<6>(der::integer),
    /* crealm */ der::optional(der::explicit_context_specific<7>(realm)),
    /* cname */ der::optional(der::explicit_context_specific<8>(cname)),
    /* realm */ der::explicit_context_specific<9>(realm),
    /* sname */ der::explicit_context_specific<10>(sname),
    /* e-text */ der::optional(der::explicit_context_specific<11>(kerberos_string)),
    /* e-data */ der::optional(der::explicit_context_specific<12>(der::octet_string)),
    /* e-checksum */ der::optional(der::explicit_context_specific<13>(checksum))));

} // namespace k5
} // namespace libasn
