#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "libasn/basic_reader.h"
#include "libasn/protocols/ldap.h"

using namespace std::literals;

namespace {

static uint8_t const packet_bytes[] = {0x30, 0x25, 0x02, 0x01, 0x01, 0x63, 0x20, 0x04, 0x00, 0x0a, 0x01, 0x02, 0x0a,
                                       0x01, 0x00, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00, 0x01, 0x01, 0x00, 0x87, 0x0b,
                                       0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x43, 0x6c, 0x61, 0x73, 0x73, 0x30, 0x00};

static bool file_readable(char const *path) {
    std::ifstream f(path, std::ios::binary);
    return static_cast<bool>(f);
}

static char const *find_fixture_path() {
    static char const *const candidates[] = {
        "../../fixtures/search_result_entry_large.bin",
        "../fixtures/search_result_entry_large.bin",
        "fixtures/search_result_entry_large.bin",
    };
    for (char const *p : candidates) {
        if (file_readable(p)) {
            return p;
        }
    }
    return nullptr;
}

static bool read_file(char const *path, std::vector<std::uint8_t> &out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return false;
    }
    f.seekg(0, std::ios::end);
    auto n = static_cast<size_t>(f.tellg());
    f.seekg(0);
    out.resize(n);
    f.read(reinterpret_cast<char *>(out.data()), static_cast<std::streamsize>(n));
    return true;
}

int demo_large_search_result(char const *path) {
    using libasn::ldap::protocol_op_enum;

    std::vector<std::uint8_t> bytes;
    if (!read_file(path, bytes)) {
        std::cerr << "Failed to open " << path << ": " << std::strerror(errno) << std::endl;
        return 1;
    }

    std::string_view sv{reinterpret_cast<char const *>(bytes.data()), bytes.size()};
    basic_reader     rd{sv};
    auto             msg = libasn::ldap::message.read(rd);
    if (!msg.has_value() || !rd.empty()) {
        std::cerr << "Failed to parse LDAPMessage" << std::endl;
        return 1;
    }

    auto [message_id, command, controls] = *msg;
    std::cout << "LDAP messageID=" << message_id << ", controls=" << (controls.has_value() ? "yes" : "no") << std::endl;

    if (command.tag_number != protocol_op_enum::SEARCH_RESULT_ENTRY) {
        std::cerr << "Expected SearchResultEntry" << std::endl;
        return 1;
    }

    auto [dn, attrs] = command.get<protocol_op_enum::SEARCH_RESULT_ENTRY>();
    std::cout << "SearchResultEntry DN (" << dn.view().size() << " bytes): " << dn.view() << std::endl;

    int          count = 0;
    basic_reader ar{attrs.view()};
    while (!ar.empty()) {
        auto pa = libasn::ldap::partial_attribute.read(ar);
        if (!pa.has_value()) {
            std::cerr << "partial_attribute at index " << count << std::endl;
            return 1;
        }
        auto [desc, vals] = *pa;
        if (count < 3) {
            std::cout << "  attr[" << count << "] type=" << desc.view() << std::endl;
        }
        basic_reader vr{vals.view()};
        int          v = 0;
        while (!vr.empty()) {
            auto o = libasn::ber::octet_string.read(vr);
            if (!o.has_value()) {
                return 1;
            }
            if (count < 3 && v == 0) {
                std::cout << "    value[0] len=" << o->view().size() << std::endl;
            }
            v++;
        }
        count++;
    }
    std::cout << "PartialAttribute count: " << count << " (PDU " << bytes.size() << " bytes)" << std::endl;
    return 0;
}

int demo_embedded_search_request() {
    auto rd = basic_reader{
        std::string_view{reinterpret_cast<char const *>(packet_bytes), std::size(packet_bytes)}
    };

    auto msg = libasn::ldap::message.read(rd);
    assert(msg.has_value());

    auto [message_id, command, optional] = *msg;
    assert(message_id == 1);
    assert(command.tag_number == libasn::ldap::protocol_op_enum::SEARCH_REQUEST);
    assert(!optional.has_value());

    auto [p1, p2, p3, p4, p5, p6, p7, p8] = command.get<libasn::ldap::protocol_op_enum::SEARCH_REQUEST>();
    (void)p7;
    (void)p8;

    assert(p1.view() == ""sv);
    assert(p2 == libasn::ldap::search_request_scope_enum::WHOLE_SUBTREE);
    assert(p3 == libasn::ldap::search_request_deref_aliases_enum::NEVER_DEREF_ALIASES);
    assert(p4 == 0);
    assert(p5 == 0);
    assert(p6 == false);

    std::cout << "Embedded example: SearchRequest parsed (messageID=" << message_id << ")." << std::endl;
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    const auto r_embed = demo_embedded_search_request();
    if (r_embed != 0) {
        return r_embed;
    }

    if (const auto fixture = find_fixture_path()) {
        return demo_large_search_result(fixture);
    }

    return 0;
}
