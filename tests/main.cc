
#include <iostream>
#include <cassert>

#include "libasn/ber.h"
#include "libasn/basic_reader.h"

using namespace std::literals;

int main(int argc, char **argv) {
    {
        auto viewer       = basic_reader{"\x04\x05\x30\x03\x01\x01\xFF"sv};
        auto octet_string = libasn::ber::octet_string.read(viewer);

        assert(viewer.size() == 0);
        assert(octet_string.has_value());
        assert(octet_string->view() == "\x30\x03\x01\x01\xFF"sv);
    }
    {
        auto viewer     = basic_reader{"\x0C\x04\x54\x65\x73\x74"sv};
        auto utf_string = libasn::ber::utf_string.read(viewer);

        assert(viewer.size() == 0);
        assert(utf_string.has_value());
        assert(utf_string->view() == "Test"sv);
    }
    {
        auto viewer           = basic_reader{"\x13\x04\x54\x65\x73\x74"sv};
        auto printable_string = libasn::ber::printable_string.read(viewer);

        assert(viewer.size() == 0);
        assert(printable_string.has_value());
        assert(printable_string->view() == "Test"sv);
    }

    {
        auto viewer  = basic_reader{"\x02\x02\x01\x00"sv};
        auto integer = libasn::ber::integer.read(viewer);

        assert(viewer.size() == 0);
        assert(integer.has_value());

        assert(integer == 256);
    }
    {
        auto viewer  = basic_reader{"\x02\x01\x48"sv};
        auto integer = libasn::ber::integer.read(viewer);

        assert(viewer.size() == 0);
        assert(integer.has_value());

        assert(integer == 72);
    }
    {
        auto viewer  = basic_reader{"\x02\x04\xF0\xF0\x00\x00"sv};
        auto integer = libasn::ber::integer.read(viewer);

        assert(viewer.size() == 0);
        assert(integer.has_value());
        assert(integer == -252706816);
    }
    {
        auto viewer  = basic_reader{"\x02\x07\xF0\xF0\x00\x00\x00\x00\x00"sv};
        auto integer = libasn::ber::integer.read(viewer);

        assert(viewer.size() == 0);
        assert(integer.has_value());
        assert(integer == -4239716836704256);
    }
    {
        auto viewer  = basic_reader{"\x02\x08\x01\xF0\x00\x00\x00\x00\x00\x00"sv};
        auto integer = libasn::ber::integer.read(viewer);

        assert(viewer.size() == 0);
        assert(integer.has_value());
        assert(integer == 139611588448485376);
    }
    {
        auto viewer  = basic_reader{"\x02\x0A\x01\xF0\x00\x00\x00\x00\x00\x00\x00\x00"sv};
        auto integer = libasn::ber::integer.read(viewer);

        assert(viewer.size() == 0);
        assert(integer.has_value());
        assert(integer == std::numeric_limits<intmax_t>::max());
    }

    {
        auto viewer   = basic_reader{"\x17\x0D\x32\x30\x30\x39\x30\x32\x31\x33\x32\x35\x32\x36\x5A"sv};
        auto utc_time = libasn::ber::utc_time.read(viewer);

        assert(utc_time.has_value());
        assert(utc_time->view() == "200902132526Z"sv);
    }

    return 0;
}
