
#include <cassert>
#include <cstdint>
#include <vector>

#include <libasn/basic_reader.h>
#include <libasn/ber.h>
#include <libasn/protocols/snmp/snmp.h>

#include <string_view>

using namespace std::literals;

namespace {

std::uint8_t packet_bytes_request[] = {0x30, 0x28, 0x02, 0x01, 0x01, 0x04, 0x05, 0x74, 0x69, 0x6d, 0x65,
                                       0x72, 0xa1, 0x1c, 0x02, 0x04, 0x21, 0xa3, 0x5d, 0xb3, 0x02, 0x01,
                                       0x00, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x30, 0x0c, 0x06, 0x08, 0x2b,
                                       0x06, 0x01, 0x02, 0x01, 0x01, 0x06, 0x00, 0x05, 0x00};

std::uint8_t packet_bytes_response[] = {0x30, 0x32, 0x02, 0x01, 0x01, 0x04, 0x05, 0x74, 0x69, 0x6d, 0x65, 0x72, 0xa2,
                                        0x26, 0x02, 0x04, 0x21, 0xa3, 0x5d, 0xae, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00,
                                        0x30, 0x18, 0x30, 0x16, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x02, 0x01, 0x01, 0x02,
                                        0x00, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0xbf, 0x08, 0x03, 0x02, 0x0a};

} // namespace

int main() {
    {
        auto rd = basic_reader{
            std::string_view{reinterpret_cast<char const *>(packet_bytes_response), std::size(packet_bytes_response)}
        };

        auto snmp_msg = libasn::snmp::message.read(rd);
        assert(snmp_msg.has_value());
        assert(rd.empty());

        auto [version, community, data] = *snmp_msg;
        assert(version == 1);
        assert(community.view() == "timer"sv);
        assert(data.tag_number == 2);
        auto [request_id, error_status, error_index, variable_bindings] = data.get<2>();
        assert(request_id == 564354478);
        assert(error_status == 0);
        assert(error_index == 0);

        std::size_t n_bindings = 0;
        while (!variable_bindings.empty()) {
            auto vb = libasn::snmp::var_bind.read(variable_bindings);
            assert(vb.has_value());
            auto [name, val] = *vb;

            auto name_arcs = libasn::ber::object_identifier_components(name.view());
            assert(name_arcs.has_value());
            assert(*name_arcs == (std::vector<std::uint32_t>{1, 3, 6, 1, 2, 1, 1, 2, 0}));

            assert(val.tag_number == 6);
            auto value_oid = val.get<6>();
            auto val_arcs  = libasn::ber::object_identifier_components(value_oid.view());
            assert(val_arcs.has_value());
            assert(*val_arcs == (std::vector<std::uint32_t>{1, 3, 6, 1, 4, 1, 8072, 3, 2, 10}));

            ++n_bindings;
        }
        assert(n_bindings == 1);
    }

    {
        auto rd = basic_reader{
            std::string_view{reinterpret_cast<char const *>(packet_bytes_request), std::size(packet_bytes_request)}
        };

        auto snmp_msg = libasn::snmp::message.read(rd);
        assert(snmp_msg.has_value());
        assert(rd.empty());

        auto [version, community, data] = *snmp_msg;
        assert(version == 1);
        assert(community.view() == "timer"sv);
        assert(data.tag_number == 1);
        auto [request_id, error_status, error_index, variable_bindings] = data.get<1>();
        assert(request_id == 564354483);
        assert(error_status == 0);
        assert(error_index == 0);

        std::size_t n_bindings = 0;
        while (!variable_bindings.empty()) {
            auto vb = libasn::snmp::var_bind.read(variable_bindings);
            assert(vb.has_value());
            auto [name, val] = *vb;

            auto name_arcs = libasn::ber::object_identifier_components(name.view());
            assert(name_arcs.has_value());
            assert(*name_arcs == (std::vector<std::uint32_t>{1, 3, 6, 1, 2, 1, 1, 6, 0}));

            assert(val.tag_number == 5);
            val.get<5>(); // unSpecified NULL (RFC 3416 VarBind)

            ++n_bindings;
        }
        assert(n_bindings == 1);
    }

    return 0;
}
