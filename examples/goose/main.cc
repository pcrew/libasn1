
#include <arpa/inet.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>

#include <libasn/basic_reader.h>
#include <libasn/protocols/iec61850/goose/goose.h>

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

std::uint8_t packet_bytes[] = {
    0x00, 0x01, 0x00, 0x91, 0x00, 0x00, 0x00, 0x00, 0x61, 0x81, 0x86, 0x80, 0x1a, 0x47, 0x45, 0x44, 0x65, 0x76, 0x69,
    0x63, 0x65, 0x46, 0x36, 0x35, 0x30, 0x2f, 0x4c, 0x4c, 0x4e, 0x30, 0x24, 0x47, 0x4f, 0x24, 0x67, 0x63, 0x62, 0x30,
    0x31, 0x81, 0x03, 0x00, 0x9c, 0x40, 0x82, 0x18, 0x47, 0x45, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x46, 0x36, 0x35,
    0x30, 0x2f, 0x4c, 0x4c, 0x4e, 0x30, 0x24, 0x47, 0x4f, 0x4f, 0x53, 0x45, 0x31, 0x83, 0x0b, 0x46, 0x36, 0x35, 0x30,
    0x5f, 0x47, 0x4f, 0x4f, 0x53, 0x45, 0x31, 0x84, 0x08, 0x38, 0x6e, 0xbb, 0xf3, 0x42, 0x17, 0x28, 0x0a, 0x85, 0x01,
    0x01, 0x86, 0x01, 0x0a, 0x87, 0x01, 0x00, 0x88, 0x01, 0x01, 0x89, 0x01, 0x00, 0x8a, 0x01, 0x08, 0xab, 0x20, 0x83,
    0x01, 0x00, 0x84, 0x03, 0x03, 0x00, 0x00, 0x83, 0x01, 0x00, 0x84, 0x03, 0x03, 0x00, 0x00, 0x83, 0x01, 0x00, 0x84,
    0x03, 0x03, 0x00, 0x00, 0x83, 0x01, 0x00, 0x84, 0x03, 0x03, 0x00, 0x00};

} // namespace

int main() {
    auto *hdr = reinterpret_cast<rgoose_header *>(packet_bytes);

    assert(ntohs(hdr->app_id_be) == 1);
    assert(ntohs(hdr->length_be) == 145);
    assert(ntohs(hdr->reserved1_be) == 0);
    assert(ntohs(hdr->reserved2_be) == 0);

    const auto  payload_len = sizeof(packet_bytes) - sizeof(rgoose_header);
    const auto *payload     = reinterpret_cast<char const *>(packet_bytes + sizeof(rgoose_header));

    auto rd = basic_reader{
        std::string_view{payload, payload_len}
    };

    using libasn::iec61850::goose::goos_epdu_enum;
    auto apdu = libasn::iec61850::goose::goos_epdu.read(rd);
    assert(apdu.has_value());
    assert(rd.empty());
    assert(apdu->tag_number == goos_epdu_enum::GOOSE_PDU);

    auto [gocb_ref, time_allowed_live, dat_set, go_id, t, st_num, sq_num, simulation, conf_rev, nds_com,
          num_dat_set_entries, all_data] = apdu->get<goos_epdu_enum::GOOSE_PDU>();

    assert(gocb_ref.view() == "GEDeviceF650/LLN0$GO$gcb01"sv);
    assert(time_allowed_live == 40000);
    assert(dat_set.view() == "GEDeviceF650/LLN0$GOOSE1"sv);
    assert(go_id.has_value() && go_id->view() == "F650_GOOSE1"sv);
    assert(t.view() == "\x38\x6e\xbb\xf3\x42\x17\x28\x0a"sv);
    assert(st_num == 1);
    assert(sq_num == 10);
    assert(simulation == false);
    assert(conf_rev == 1);
    assert(num_dat_set_entries == 8);

    using D = libasn::iec61850::goose::data_enum;

    auto slice = all_data.view();
    auto take  = [&](basic_reader &r) {
        auto el = libasn::iec61850::goose::data.read(r);
        assert(el.has_value());
        return *el;
    };

    {
        basic_reader br{slice};
        assert(br.size() == 32);
        auto v = take(br);
        assert(v.tag_number == D::BOOLEAN);
        assert(v.get<D::BOOLEAN>() == false);
        slice = br.view();
    }
    {
        basic_reader br{slice};
        assert(br.size() == 29);
        auto v = take(br);
        assert(v.tag_number == D::BIT_STRING);
        assert(v.get<D::BIT_STRING>().view() == "\x03\x00\x00"sv);
        slice = br.view();
    }
    {
        basic_reader br{slice};
        assert(br.size() == 24);
        auto v = take(br);
        assert(v.tag_number == D::BOOLEAN);
        assert(v.get<D::BOOLEAN>() == false);
        slice = br.view();
    }
    {
        basic_reader br{slice};
        assert(br.size() == 21);
        auto v = take(br);
        assert(v.tag_number == D::BIT_STRING);
        assert(v.get<D::BIT_STRING>().view() == "\x03\x00\x00"sv);
        slice = br.view();
    }
    {
        basic_reader br{slice};
        assert(br.size() == 16);
        auto v = take(br);
        assert(v.tag_number == D::BOOLEAN);
        assert(v.get<D::BOOLEAN>() == false);
        slice = br.view();
    }
    {
        basic_reader br{slice};
        assert(br.size() == 13);
        auto v = take(br);
        assert(v.tag_number == D::BIT_STRING);
        assert(v.get<D::BIT_STRING>().view() == "\x03\x00\x00"sv);
        slice = br.view();
    }
    {
        basic_reader br{slice};
        assert(br.size() == 8);
        auto v = take(br);
        assert(v.tag_number == D::BOOLEAN);
        assert(v.get<D::BOOLEAN>() == false);
        slice = br.view();
    }
    {
        basic_reader br{slice};
        assert(br.size() == 5);
        auto v = take(br);
        assert(v.tag_number == D::BIT_STRING);
        assert(v.get<D::BIT_STRING>().view() == "\x03\x00\x00"sv);
        slice = br.view();
    }
    assert(slice.size() == 0);

    return 0;
}
