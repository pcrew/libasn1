
#include <cassert>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

#include <libasn/protocols/iec61850/mms/mms.h>
#include "libasn/basic_reader.h"

using namespace std::literals;

int main() {
#if 0
    /* initiaate request PDU */
    {
        uint8_t packet_bytes[] = {0xa8, 0x26, 0x80, 0x03, 0x00, 0xfa, 0x00, 0x81, 0x01, 0x0a, 0x82, 0x01, 0x0a, 0x83,
                                  0x01, 0x05, 0xa4, 0x16, 0x80, 0x01, 0x01, 0x81, 0x03, 0x05, 0xe1, 0x00, 0x82, 0x0c,
                                  0x03, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0x10};

        auto rd = basic_reader{
            std::string_view{reinterpret_cast<const char *>(packet_bytes), std::size(packet_bytes)}
        };
        auto mms_pdu = libasn::iec61850::mms::mms_pdu.read(rd);
        assert(mms_pdu.has_value());
        assert(mms_pdu->tag_number == libasn::iec61850::mms::mms_pdu_choice::INITIATE_REQUEST_PDU);
        auto [local_detail_calling, proposed_max_serv_outstanding_calling, proposed_max_serv_outstanding_called,
              proposed_data_structure_nesting_level, mms_init_request_detail] =
            mms_pdu->get<libasn::iec61850::mms::mms_pdu_choice::INITIATE_REQUEST_PDU>();

        assert(local_detail_calling.has_value() && local_detail_calling == 64000);
        assert(proposed_max_serv_outstanding_calling == 10);
        assert(proposed_max_serv_outstanding_called == 10);
        assert(proposed_data_structure_nesting_level.has_value() && proposed_data_structure_nesting_level == 5);

        auto [proposed_version_number, proposed_parameter_cbb, services_supported_calling] = mms_init_request_detail;

        assert(proposed_version_number == 1);

        assert(proposed_parameter_cbb.size() == 3);
        assert(static_cast<uint8_t>(proposed_parameter_cbb.view()[0]) == 0x05);
        assert(static_cast<uint8_t>(proposed_parameter_cbb.view()[1]) == 0xe1);
        assert(static_cast<uint8_t>(proposed_parameter_cbb.view()[2]) == 0x00);

        assert(services_supported_calling.size() == 12);
        assert(static_cast<uint8_t>(services_supported_calling.view()[0]) == 0x03);
        assert(static_cast<uint8_t>(services_supported_calling.view()[1]) == 0xa0);
        assert(static_cast<uint8_t>(services_supported_calling.view()[2]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[3]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[4]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[5]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[6]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[7]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[8]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[9]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_calling.view()[10]) == 0xe1);
        assert(static_cast<uint8_t>(services_supported_calling.view()[11]) == 0x10);
    }

    /* initiate reposonse PDU */
    {
        uint8_t packet_bytes[] = {0xa9, 0x25, 0x80, 0x02, 0x7d, 0x00, 0x81, 0x01, 0x0a, 0x82, 0x01, 0x08, 0x83,
                                  0x01, 0x05, 0xa4, 0x16, 0x80, 0x01, 0x01, 0x81, 0x03, 0x05, 0xe1, 0x00, 0x82,
                                  0x0c, 0x03, 0xee, 0x08, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0xed, 0x18};
        auto    rd             = basic_reader{
            std::string_view{reinterpret_cast<const char *>(packet_bytes), std::size(packet_bytes)}
        };
        auto mms_pdu = libasn::iec61850::mms::mms_pdu.read(rd);
        assert(mms_pdu.has_value());
        assert(mms_pdu->tag_number == libasn::iec61850::mms::mms_pdu_choice::INITIATE_RESPONSE_PDU);

        auto [local_detail_called, negociated_max_serv_outstanding_calling, negociated_max_serv_outstanding_called,
              negociated_data_structure_nesting_level, mms_init_response_detail] =
            mms_pdu->get<libasn::iec61850::mms::mms_pdu_choice::INITIATE_RESPONSE_PDU>();

        assert(local_detail_called.has_value() && local_detail_called == 32000);
        assert(negociated_max_serv_outstanding_calling == 10);
        assert(negociated_max_serv_outstanding_called == 8);
        assert(negociated_data_structure_nesting_level.has_value() && negociated_data_structure_nesting_level == 5);

        auto [negociated_version_number, negociated_parameter_cbb, services_supported_called] =
            mms_init_response_detail;

        assert(negociated_version_number == 1);

        assert(negociated_parameter_cbb.size() == 3);
        assert(static_cast<uint8_t>(negociated_parameter_cbb.view()[0]) == 0x05);
        assert(static_cast<uint8_t>(negociated_parameter_cbb.view()[1]) == 0xe1);
        assert(static_cast<uint8_t>(negociated_parameter_cbb.view()[2]) == 0x00);

        assert(services_supported_called.size() == 12);
        assert(static_cast<uint8_t>(services_supported_called.view()[0]) == 0x03);
        assert(static_cast<uint8_t>(services_supported_called.view()[1]) == 0xee);
        assert(static_cast<uint8_t>(services_supported_called.view()[2]) == 0x08);
        assert(static_cast<uint8_t>(services_supported_called.view()[3]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_called.view()[4]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_called.view()[5]) == 0x04);
        assert(static_cast<uint8_t>(services_supported_called.view()[6]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_called.view()[7]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_called.view()[8]) == 0x00);
        assert(static_cast<uint8_t>(services_supported_called.view()[9]) == 0x01);
        assert(static_cast<uint8_t>(services_supported_called.view()[10]) == 0xed);
        assert(static_cast<uint8_t>(services_supported_called.view()[11]) == 0x18);
    }

    /* confirmed request PDU */
    {
    }

#endif
    return 0;
}
