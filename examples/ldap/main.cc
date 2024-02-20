
#include <cassert>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

#include "libasn/protocols/ldap.h"
#include "libasn/basic_reader.h"

using namespace std::literals;

uint8_t packet_bytes[] = {0x30, 0x25, 0x02, 0x01, 0x01, 0x63, 0x20, 0x04, 0x00, 0x0a, 0x01, 0x02, 0x0a,
                          0x01, 0x00, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00, 0x01, 0x01, 0x00, 0x87, 0x0b,
                          0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x43, 0x6c, 0x61, 0x73, 0x73, 0x30, 0x00};

int main() {
    auto rd = basic_reader{
        std::string_view{reinterpret_cast<const char *>(packet_bytes), std::size(packet_bytes)}
    };

    auto msg = libasn::ldap::message.read(rd);
    assert(msg.has_value());

    auto [message_id, command, optional] = *msg;
    assert(message_id == 1);
    assert(command.tag_number == libasn::ldap::protocol_op_enum::SEARCH_REQUEST);
    assert(!optional.has_value());

    auto [p1, p2, p3, p4, p5, p6, p7, p8] = command.get<libasn::ldap::protocol_op_enum::SEARCH_REQUEST>();

    assert(p1.view() == ""sv);
    assert(p2 == libasn::ldap::search_request_scope_enum::WHOLE_SUBTREE);
    assert(p3 == libasn::ldap::search_request_deref_aliases_enum::NEVER_DEREF_ALIASES);
    assert(p4 == 0);
    assert(p5 == 0);
    assert(p6 == false);

    return 0;
}
