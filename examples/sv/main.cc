
#include <arpa/inet.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>

#include <libasn/basic_reader.h>
#include <libasn/protocols/iec61850/sv/sv.h>

#include <string_view>

using namespace std::literals;

namespace {

#pragma pack(push, 1)
struct rgoose_header {
    std::uint16_t app_id_be;
    std::uint16_t length_be;
    std::uint16_t reserved1_be;
    std::uint16_t reserved2_be;
};
#pragma pack(pop)

static_assert(sizeof(rgoose_header) == 8, "");

/* See 1 packet: https://github.com/mgadelha/Sampled_Values/blob/master/SV_Normal_Traffic.cap */

std::uint8_t packet_bytes[] = {0x40, 0x01, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00, 0x60, 0x5c, 0x80, 0x01, 0x01, 0xa2, 0x57,
                               0x30, 0x55, 0x80, 0x04, 0x34, 0x30, 0x30, 0x31, 0x82, 0x02, 0x01, 0x18, 0x83, 0x04, 0x00,
                               0x00, 0x00, 0x01, 0x85, 0x01, 0x02, 0x87, 0x40, 0xff, 0xfe, 0x59, 0x82, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x04, 0x3d, 0xdc, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfd, 0x6f, 0x5c, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x06, 0xba, 0x00, 0x00, 0x20, 0x00, 0xff, 0x8d, 0xf4, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x01, 0x1d, 0xfb, 0xc2, 0x00, 0x00, 0x00, 0x00, 0xff, 0x55, 0x60, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4f, 0xce, 0x00, 0x00, 0x20, 0x00};

std::uint8_t data[] = {0xff, 0xfe, 0x59, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x3d, 0xdc, 0x00, 0x00, 0x00, 0x00,
                       0xff, 0xfd, 0x6f, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xba, 0x00, 0x00, 0x20, 0x00,
                       0xff, 0x8d, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1d, 0xfb, 0xc2, 0x00, 0x00, 0x00, 0x00,
                       0xff, 0x55, 0x60, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4f, 0xce, 0x00, 0x00, 0x20, 0x00};

} // namespace

int main() {
    auto *hdr = reinterpret_cast<rgoose_header *>(packet_bytes);

    assert(ntohs(hdr->length_be) == 102);
    assert(ntohs(hdr->reserved1_be) == 0);
    assert(ntohs(hdr->reserved2_be) == 0);

    size_t const payload_len = sizeof(packet_bytes) - sizeof(rgoose_header);
    auto         rd          = basic_reader{
        std::string_view{
                         reinterpret_cast<char const *>(packet_bytes + sizeof(rgoose_header)),
                         payload_len, }
    };

    using libasn::iec61850::sv::sampled_values_enum;

    auto outer = libasn::iec61850::sv::sampled_values.read(rd);
    assert(outer.has_value());
    assert(rd.empty());
    assert(outer->tag_number == sampled_values_enum::SAV_PDU);

    auto [no_asdu, seq_asdu] = outer->get<sampled_values_enum::SAV_PDU>();
    assert(no_asdu == 1);

    basic_reader seq_rd{seq_asdu.view()};
    assert(seq_rd.size() > 0);
    auto one = libasn::iec61850::sv::asdu.read(seq_rd);
    assert(one.has_value());
    assert(seq_rd.empty());

    auto [sv_id, dat_set, smp_cnt, conf_rev, refr_tm, smp_synch, smp_rate, seq_data, smp_mod, gmid_data] = *one;

    assert(sv_id.view() == "4001"sv);
    assert(!dat_set.has_value());
    assert(smp_cnt == 280);
    assert(conf_rev == 1);
    assert(!refr_tm.has_value());
    assert(smp_synch == 2);
    assert(!smp_rate.has_value());
    assert(seq_data.view() == std::string_view(reinterpret_cast<char const *>(data), sizeof(data)));
    assert(!smp_mod.has_value());
    assert(!gmid_data.has_value());

    return 0;
}
