#pragma once

#include <libasn/ber.h>
#include <libasn/der.h>
#include <libasn/basic_reader.h>

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

constexpr auto uint32          = der::integer;
constexpr auto int32           = der::integer;
constexpr auto microseconds    = der::integer;
constexpr auto kerberos_string = der::general_string;
constexpr auto cname_string    = der::general_string;
constexpr auto sname_string    = der::general_string;
constexpr auto realm           = kerberos_string;

constexpr auto principal_name = der::sequence(
    /* name-type */ int32.context_specific<0>(),
    /* name-string */ der::sequence_of(kerberos_string).context_specific<1>());

constexpr auto cname = principal_name;
constexpr auto sname = principal_name;

#if 0
constexpr auto kerberos_time = der::generalized_time;
#else
constexpr auto kerberos_time = der::general_string;
#endif

namespace details {
constexpr auto encrypted_data = der::sequence(
    /* etype */ int32.context_specific<0>(),
    /* kvno */ der::optional(uint32.context_specific<1>()),
    /* cipher */ der::octet_string.context_specific<2>());

#if 0
constexpr auto encrypted_data_ber = ber::sequence(
    /* etype */ ber::integer.context_specific<0>(),
    /* kvno */ ber::integer.context_specific<1>(),
    /* cipher */ ber::octet_string.context_specific<2>());
#endif
}  // namespace details

constexpr auto encrypted_ticket_data        = details::encrypted_data;
constexpr auto encrypted_authorization_data = details::encrypted_data;
constexpr auto encrypted_authenticator      = details::encrypted_data;
constexpr auto encrypted_kdc_rep_data       = details::encrypted_data;
constexpr auto encrypted_arp_rep_data       = details::encrypted_data;
constexpr auto encrypted_krb_priv_data      = details::encrypted_data;
constexpr auto encrypted_krb_kred_data      = details::encrypted_data;
constexpr auto pa_enc_timestamp             = details::encrypted_data;

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

constexpr auto typed_data_entry = der::sequence(
    /* data-type */ der::integer.context_specific<1>(),
    /* data-value */ der::optional(der::octet_string.context_specific<2>()));

constexpr auto typed_data = der::sequence_of(typed_data_entry);

constexpr auto etype_info2_entry = der::sequence(
    /* etype */ der::integer.context_specific<0>(),
    /* salt */ der::optional(kerberos_string.context_specific<1>()),
    /* s2kparams */ der::optional(der::octet_string.context_specific<2>()));

constexpr auto etype_info2 = der::sequence_of(etype_info2_entry);

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

#if 0
constexpr auto ticket = der::sequence(
                            /* tkt-vno */ der::integer.context_specific<0>(),
                            /* realm */ realm.context_specific<1>(),
                            /* sname */ principal_name.context_specific<2>(),
                            /* enc-part */ encrypted_ticket_data.context_specific<3>())
                            .application<protocol_op_enum::TICKET>();
#else
constexpr auto ticket = der::sequence(
    /* tkt-vno */ der::integer.context_specific<0>(),
    /* realm */ realm.context_specific<1>(),
    /* sname */ principal_name.context_specific<2>(),
    /* enc-part */ encrypted_ticket_data.context_specific<3>());
#endif

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
    /* authorization-data */
    der::optional(authorization_data.context_specific<10>()));

constexpr auto kdc_req_body = der::sequence(
    /* kdc-options */ kerberos_flags.context_specific<0>(),
    /* cname */ der::optional(principal_name.context_specific<1>()),
    /* realm */ realm.context_specific<2>(),
    /* sname */ der::optional(principal_name.context_specific<3>()),
    /* from */ der::optional(kerberos_time.context_specific<4>()),
    /* till */ der::optional(kerberos_time.context_specific<5>()),
    /* rtime */ der::optional(kerberos_time.context_specific<6>()),
    /* nonce */ uint32.context_specific<7>(),
    /* etype */ der::sequence_of(uint32).context_specific<8>(),
    /* addresses */ der::optional(host_addresses.context_specific<9>()),
    /* enc-authorization-data */
    der::optional(encrypted_data.context_specific<10>()),
    /* additional-tickets */
    der::optional(der::sequence_of(ticket).context_specific<11>()));  // Должен ли в ticket быть application?

constexpr auto kdc_req = der::sequence(
    /* pvno */ der::integer.context_specific<1>(),
    /* msg-type */ der::integer.context_specific<2>(),
    /* padata */ der::optional(der::sequence_of(pa_data).context_specific<3>()),
    /* req-body */ kdc_req_body.context_specific<4>());

constexpr auto kdc_rep = der::sequence(
    /* pvno */ der::integer.context_specific<0>(),
    /* msg-type */ der::integer.context_specific<1>(),
    /* padata */ der::optional(der::sequence_of(pa_data).context_specific<2>()),
    /* crealm */ realm.context_specific<3>(),
    /* cname */ cname.context_specific<4>(),
    /* ticket */ kerberos_string.context_specific<5>(),  // must be read as kerberos ticket;
    /* enc-part */ encrypted_kdc_rep_data.context_specific<6>());

constexpr auto authentificator = der::sequence(
                                     /* authentificator-vno */ der::integer.context_specific<0>(),
                                     /* crealm */ realm.context_specific<1>(),
                                     /* cname */ principal_name.context_specific<2>(),
                                     /* cksum */ der::optional(checksum.context_specific<3>()),
                                     /* cusec */ microseconds.context_specific<4>(),
                                     /* ctime */ kerberos_time.context_specific<5>(),
                                     /* subkey */ der::optional(encryption_key.context_specific<6>()),
                                     /* seq-number */ der::optional(uint32.context_specific<7>()),
                                     /* authorization-data */ der::optional(authorization_data.context_specific<8>()))
                                     .application<protocol_op_enum::AUTHENTICATOR>();

constexpr auto as_req  = kdc_req.application<protocol_op_enum::AS_REQ>();
constexpr auto as_rep  = kdc_rep.application<protocol_op_enum::AS_REP>();
constexpr auto tgs_req = kdc_req.application<protocol_op_enum::TGS_REQ>();
constexpr auto tgs_rep = kdc_rep.application<protocol_op_enum::TGS_REP>();
constexpr auto ap_req  = der::sequence(
                            /* pvno */ der::integer.context_specific<0>(),
                            /* msg-type */ der::integer.context_specific<1>(),
                            /* ap-options */ kerberos_flags.context_specific<2>(),
                            /* ticket */ ticket.context_specific<3>(),
                            /* authenticator */ encrypted_authenticator.context_specific<4>())
                            .application<protocol_op_enum::AP_REQ>();

constexpr auto krb_error = der::sequence(
                               /* pvno */ der::integer.context_specific<0>(),
                               /* msg-type */ der::integer.context_specific<1>(),
                               /* ctime */ der::optional(kerberos_time.context_specific<2>()),
                               /* cusec */ der::optional(microseconds.context_specific<3>()),
                               /* stime */ kerberos_time.context_specific<4>(),
                               /* susec */ microseconds.context_specific<5>(),
                               /* error-code */ der::integer.context_specific<6>(),
                               /* crealm */ der::optional(realm.context_specific<7>()),
                               /* cname */ der::optional(cname.context_specific<8>()),
                               /* realm */ realm.context_specific<9>(),
                               /* sname */ sname.context_specific<10>(),
                               /* e-text */ der::optional(kerberos_string.context_specific<11>()),
                               /* e-data */ der::optional(der::octet_string.context_specific<12>()),
                               /* e-checksum */ der::optional(checksum.context_specific<13>()))
                               .application<protocol_op_enum::KRB_ERROR>();

}  // namespace k5
}  // namespace libasn

