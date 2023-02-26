
#pragma once

#include <libasn/ber.h>

// TODO: implement mms
namespace libasn {
namespace iec61850 {

struct hdr {
    auto inline app_id() { return ntohs(__app_id); }
    auto inline length() { return ntohs(__length); }
    auto inline reserved_1() { return __reserved_1; }
    auto inline reserved_2() { return __reserved_2; }

    template <typename T>
    auto inline payload() {
        return reinterpret_cast<const T *>(this + 1);
    }

    auto inline payload_length() { return length() - sizeof(*this); }

private:
    uint16_t __app_id;
    uint16_t __length;
    uint16_t __reserved_1;
    uint16_t __reserved_2;
} __attribute__((packed));

namespace sv {

constexpr auto utc_time  = ber::octet_string;
constexpr auto data      = ber::octet_string;
constexpr auto gmid_data = ber::octet_string;

enum class application_choice {
    SAV_PDU = 0,
};

constexpr auto asdu = ber::sequence(
    /* svId */ ber::visible_string.context_specific<0>(),
    /* datSet */ ber::optional(ber::visible_string.context_specific<1>()),
    /* smpCnt */ ber::integer.context_specific<2>(),
    /* confRev */ ber::integer.context_specific<3>(),
    /* refrTm */ ber::optional(utc_time.context_specific<4>()),
    /* smpSynch */ ber::optional(ber::integer.context_specific<5>()),
    /* smpRate */ ber::optional(ber::integer.context_specific<6>()),
    /* seqData */ data.context_specific<7>(),
    /* smpMod */ ber::optional(ber::integer.context_specific<8>()),
    /* gmidData */ ber::optional(gmid_data.context_specific<9>()));

constexpr auto sav_pdu = ber::sequence(
                             /* noASDU */ ber::integer.context_specific<0>(),
                             /* seqASDU */ ber::sequence_of(asdu).context_specific<2>())
                             .application<application_choice::SAV_PDU>();

constexpr auto sample_values = ber::choice<application_choice>().with(sav_pdu);

}  // namespace sv

namespace goose {

constexpr auto floating_point = ber::octet_string;
constexpr auto utc_time       = ber::octet_string;
constexpr auto mms_string     = ber::utf_string;
constexpr auto time_of_day    = ber::octet_string;

enum class data_choice {
    ARRAY            = 1,
    STRUCTURE        = 2,
    BOOLEAN          = 3,
    BIT_STRING       = 4,
    INTEGER          = 5,
    UNSIGNED         = 6,
    FLOATING_POINT   = 7,
    REAL             = 8,
    OCTET_STRING     = 9,
    VISIBLE_STRING   = 10,
    GENERALIZED_TIME = 11,
    BINARY_TIME      = 12,
    BCD              = 13,
    BOOLEAN_ARRAY    = 14,
    /* OBJ_ID = 15, */
    MMS_STRING = 16,
    UTC_TIME   = 17
};

// clang-format off
// TODO: generalized_time Не реализован.
constexpr auto data_base = ber::choice<data_choice>()
                               .with<data_choice::BOOLEAN>(ber::boolean)
                               .with<data_choice::BIT_STRING>(ber::bit_string)
                               .with<data_choice::INTEGER>(ber::integer)
                               .with<data_choice::UNSIGNED>(ber::integer)
                               .with<data_choice::FLOATING_POINT>(floating_point)
                               .with<data_choice::OCTET_STRING>(ber::octet_string)
                               .with<data_choice::VISIBLE_STRING>(ber::visible_string)
//                               .with<data_choice::GENERALIZED_TIME>(ber::octet_string)  // TODO
                               .with<data_choice::BINARY_TIME>(time_of_day)
//                               .with<data_choice::BCD>(ber::integer)
                               .with<data_choice::BOOLEAN_ARRAY>(ber::bit_string)
                               .with<data_choice::MMS_STRING>(mms_string)
                               .with<data_choice::UTC_TIME>(utc_time);

constexpr auto data = ber::choice<data_choice>()
                          .with<data_choice::ARRAY>(ber::sequence_of(data_base))
                          .with<data_choice::STRUCTURE>(ber::sequence_of(data_base))
                          .with<data_choice::BOOLEAN>(ber::boolean)
                          .with<data_choice::BIT_STRING>(ber::bit_string)
                          .with<data_choice::INTEGER>(ber::integer)
                          .with<data_choice::UNSIGNED>(ber::integer)
                          .with<data_choice::FLOATING_POINT>(floating_point)
                          .with<data_choice::OCTET_STRING>(ber::octet_string)
                          .with<data_choice::VISIBLE_STRING>(ber::visible_string)
//                          .with<data_choice::GENERALIZED_TIME>(ber::octet_string)  // TODO
                          .with<data_choice::BINARY_TIME>(time_of_day)
//                          .with<data_choice::BCD>(ber::integer)
//                          .with<data_choice::BOOLEAN_ARRAY>(ber::bit_string)
                          .with<data_choice::MMS_STRING>(mms_string)
                          .with<data_choice::UTC_TIME>(utc_time);
// clang-format on

// clang-format off
constexpr static auto pdu = ber::sequence(
                                /* gocbRef */ ber::visible_string.context_specific<0x00>(),
                                /* timeAllowedtoLive */ ber::integer.context_specific<0x01>(),
                                /* datSet */ ber::visible_string.context_specific<0x02>(),
                                /* goID */ ber::optional(ber::visible_string.context_specific<0x03>()),
                                /* t */ utc_time.context_specific<0x04>(),
                                /* stNum */ ber::integer.context_specific<0x05>(),
                                /* sqNum */ ber::integer.context_specific<0x06>(),
                                /* simulation */ ber::boolean.context_specific<0x07>(),
                                /* confRev */ ber::integer.context_specific<0x08>(),
                                /* ndsCom */ ber::boolean.context_specific<0x09>(),
                                /* numDatSetEntries */ ber::integer.context_specific<0x0a>(),
                                /* numDatSetEntries */ ber::sequence_of(data).context_specific<0x0b>()).application<1>();
// clang-format on

}  // namespace goose

}  // namespace iec61850
}  // namespace libasn
