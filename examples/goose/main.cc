
#include <cassert>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

#include "libasn/protocols/iec61850.h"
#include "libasn/basic_reader.h"

using namespace std::literals;

/**
 * see 1 packet in https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/GOOSE.pcap.gz
 **/

uint8_t packet_bytes[] = {
    0x00, 0x01, 0x00, 0x91, 0x00, 0x00, 0x00, 0x00, 0x61, 0x81, 0x86, 0x80, 0x1a, 0x47, 0x45, 0x44, 0x65, 0x76, 0x69,
    0x63, 0x65, 0x46, 0x36, 0x35, 0x30, 0x2f, 0x4c, 0x4c, 0x4e, 0x30, 0x24, 0x47, 0x4f, 0x24, 0x67, 0x63, 0x62, 0x30,
    0x31, 0x81, 0x03, 0x00, 0x9c, 0x40, 0x82, 0x18, 0x47, 0x45, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x46, 0x36, 0x35,
    0x30, 0x2f, 0x4c, 0x4c, 0x4e, 0x30, 0x24, 0x47, 0x4f, 0x4f, 0x53, 0x45, 0x31, 0x83, 0x0b, 0x46, 0x36, 0x35, 0x30,
    0x5f, 0x47, 0x4f, 0x4f, 0x53, 0x45, 0x31, 0x84, 0x08, 0x38, 0x6e, 0xbb, 0xf3, 0x42, 0x17, 0x28, 0x0a, 0x85, 0x01,
    0x01, 0x86, 0x01, 0x0a, 0x87, 0x01, 0x00, 0x88, 0x01, 0x01, 0x89, 0x01, 0x00, 0x8a, 0x01, 0x08, 0xab, 0x20, 0x83,
    0x01, 0x00, 0x84, 0x03, 0x03, 0x00, 0x00, 0x83, 0x01, 0x00, 0x84, 0x03, 0x03, 0x00, 0x00, 0x83, 0x01, 0x00, 0x84,
    0x03, 0x03, 0x00, 0x00, 0x83, 0x01, 0x00, 0x84, 0x03, 0x03, 0x00, 0x00};

int main() {
    auto hdr = reinterpret_cast<libasn::iec61850::hdr *>(packet_bytes);

    assert(hdr->app_id() == 1);
    assert(hdr->length() == 145);
    assert(hdr->reserved_1() == 0);
    assert(hdr->reserved_2() == 0);

    auto rd = basic_reader{
        std::string_view{hdr->payload<char>(), hdr->payload_length()}
    };

    auto goose = libasn::iec61850::goose::pdu.read(rd);
    assert(goose.has_value());

    auto [gocb_ref, time_allowed_live, dat_set, go_id, t, st_num, sq_num, simulation, conf_rev, nds_com,
          num_dat_set_entries, all_data] = *goose;

    assert(gocb_ref.view() == "GEDeviceF650/LLN0$GO$gcb01"sv);
    assert(time_allowed_live == 40000);
    assert(dat_set.view() == "GEDeviceF650/LLN0$GOOSE1"sv);
    assert(go_id && go_id->view() == "F650_GOOSE1"sv);  // go_id is std::oprional<reader>
    assert(t.view() == "\x38\x6e\xbb\xf3\x42\x17\x28\x0a"sv);
    assert(st_num == 1);
    assert(sq_num == 10);
    assert(simulation == false);
    assert(conf_rev == 1);
    assert(num_dat_set_entries == 8);

    {
        assert(all_data.size() == 32);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BOOLEAN);
        assert(data->get<libasn::iec61850::goose::data_choice::BOOLEAN>() == false);
    }
    {
        assert(all_data.size() == 29);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BIT_STRING);
        auto string = data->get<libasn::iec61850::goose::data_choice::BIT_STRING>();
        assert(string.view() == "\x03\x00\x00"sv);
    }
    {
        assert(all_data.size() == 24);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BOOLEAN);
        assert(data->get<libasn::iec61850::goose::data_choice::BOOLEAN>() == false);
    }
    {
        assert(all_data.size() == 21);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BIT_STRING);
        auto string = data->get<libasn::iec61850::goose::data_choice::BIT_STRING>();
        assert(string.view() == "\x03\x00\x00"sv);
    }
    {
        assert(all_data.size() == 16);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BOOLEAN);
        assert(data->get<libasn::iec61850::goose::data_choice::BOOLEAN>() == false);
    }
    {
        assert(all_data.size() == 13);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BIT_STRING);
        auto string = data->get<libasn::iec61850::goose::data_choice::BIT_STRING>();
        assert(string.view() == "\x03\x00\x00"sv);
    }
    {
        assert(all_data.size() == 8);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BOOLEAN);
        assert(data->get<libasn::iec61850::goose::data_choice::BOOLEAN>() == false);
    }
    {
        assert(all_data.size() == 5);
        auto data = libasn::iec61850::goose::data.read(all_data);
        assert(data.has_value());
        assert(data->tag_number == libasn::iec61850::goose::data_choice::BIT_STRING);
        auto string = data->get<libasn::iec61850::goose::data_choice::BIT_STRING>();
        assert(string.view() == "\x03\x00\x00"sv);
    }
    { assert(all_data.size() == 0); }

    return 0;
}
