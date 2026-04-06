
#include <cassert>
#include <cstdint>
#include <limits>

#include "libasn/ber.h"
#include "libasn/der.h"
#include "libasn/basic_reader.h"

using namespace std::literals;

namespace {

enum class small_enum : int { ALPHA = 0, BETA = 2 };

void test_ber_octet_utf_printable() {
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
}

void test_ber_boolean_and_null() {
    {
        auto viewer = basic_reader{"\x01\x01\xFF"sv};
        auto b      = libasn::ber::boolean.read(viewer);
        assert(viewer.size() == 0);
        assert(b.has_value());
        assert(*b == true);
    }
    {
        auto viewer = basic_reader{"\x01\x01\x00"sv};
        auto b      = libasn::ber::boolean.read(viewer);
        assert(viewer.size() == 0);
        assert(b.has_value());
        assert(*b == false);
    }
    {
        auto viewer = basic_reader{"\x05\x00"sv};
        auto n      = libasn::ber::null.read(viewer);
        assert(viewer.size() == 0);
        assert(n.has_value());
    }
}

void test_ber_integers() {
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
}

void test_ber_utc_time() {
    auto viewer   = basic_reader{"\x17\x0D\x32\x30\x30\x39\x30\x32\x31\x33\x32\x35\x32\x36\x5A"sv};
    auto utc_time = libasn::ber::utc_time.read(viewer);
    assert(utc_time.has_value());
    assert(utc_time->view() == "200902132526Z"sv);
}

void test_ber_sequence() {
    constexpr auto seq    = libasn::ber::sequence(libasn::ber::integer, libasn::ber::integer);
    auto           viewer = basic_reader{"\x30\x07\x02\x01\x48\x02\x02\x01\x00"sv};
    auto           r      = seq.read(viewer);
    assert(viewer.size() == 0);
    assert(r.has_value());
    auto [a, b] = *r;
    assert(a == 72);
    assert(b == 256);
}

void test_ber_enumerated() {
    auto viewer = basic_reader{"\x0A\x01\x02"sv};
    auto e      = libasn::ber::enumerated<small_enum>().read(viewer);
    assert(viewer.size() == 0);
    assert(e.has_value());
    assert(*e == small_enum::BETA);
}

void test_der_primitives() {
    {
        auto viewer  = basic_reader{"\x02\x01\x48"sv};
        auto integer = libasn::der::integer.read(viewer);
        assert(viewer.size() == 0);
        assert(integer.has_value());
        assert(integer == 72);
    }
    {
        auto viewer = basic_reader{"\x04\x05\x30\x03\x01\x01\xFF"sv};
        auto s      = libasn::der::octet_string.read(viewer);
        assert(viewer.size() == 0);
        assert(s.has_value());
        assert(s->view() == "\x30\x03\x01\x01\xFF"sv);
    }
    {
        auto viewer = basic_reader{"\x0C\x04\x54\x65\x73\x74"sv};
        auto s      = libasn::der::utf_string.read(viewer);
        assert(viewer.size() == 0);
        assert(s.has_value());
        assert(s->view() == "Test"sv);
    }
    {
        auto viewer = basic_reader{"\x13\x04\x54\x65\x73\x74"sv};
        auto s      = libasn::der::printable_string.read(viewer);
        assert(viewer.size() == 0);
        assert(s.has_value());
        assert(s->view() == "Test"sv);
    }
    {
        auto viewer = basic_reader{"\x01\x01\xFF"sv};
        auto b      = libasn::der::boolean.read(viewer);
        assert(viewer.size() == 0);
        assert(b.has_value() && *b == true);
    }
    {
        auto viewer = basic_reader{"\x05\x00"sv};
        auto n      = libasn::der::null.read(viewer);
        assert(viewer.size() == 0);
        assert(n.has_value());
    }
    {
        auto viewer = basic_reader{"\x17\x0D\x32\x30\x30\x39\x30\x32\x31\x33\x32\x35\x32\x36\x5A"sv};
        auto t      = libasn::der::utc_time.read(viewer);
        assert(viewer.size() == 0);
        assert(t.has_value());
        assert(t->view() == "200902132526Z"sv);
    }
    {
        auto viewer = basic_reader{"\x18\x0F\x31\x39\x37\x30\x30\x31\x30\x31\x30\x30\x30\x30\x30\x30\x5A"sv};
        auto gt     = libasn::der::generalized_time.read(viewer);
        assert(viewer.size() == 0);
        assert(gt.has_value());
        assert(gt->view() == "19700101000000Z"sv);
    }
}

void test_der_enumerated() {
    auto viewer = basic_reader{"\x0A\x01\x02"sv};
    auto e      = libasn::der::enumerated<small_enum>().read(viewer);
    assert(viewer.size() == 0);
    assert(e.has_value());
    assert(*e == small_enum::BETA);
}

void test_der_sequence_and_sequence_of() {
    {
        constexpr auto seq    = libasn::der::sequence(libasn::der::integer, libasn::der::integer);
        auto           viewer = basic_reader{"\x30\x07\x02\x01\x48\x02\x02\x01\x00"sv};
        auto           r      = seq.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        auto [a, b] = *r;
        assert(a == 72);
        assert(b == 256);
    }
    {
        constexpr auto so     = libasn::der::sequence_of(libasn::der::integer);
        auto           viewer = basic_reader{"\x30\x06\x02\x01\x01\x02\x01\x02"sv};
        auto           r      = so.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(r->size() == 6);
        auto inner = *r;
        auto a     = libasn::der::integer.read(inner);
        assert(a.has_value() && *a == 1);
        auto b = libasn::der::integer.read(inner);
        assert(b.has_value() && *b == 2);
        assert(inner.size() == 0);
    }
}

void test_der_explicit_context_specific() {
    constexpr auto wrapped = libasn::der::explicit_context_specific<0>(libasn::der::integer);
    auto           viewer  = basic_reader{"\xA0\x03\x02\x01\x48"sv};
    auto           r       = wrapped.read(viewer);
    assert(viewer.size() == 0);
    assert(r.has_value());
    assert(*r == 72);
}

void test_der_explicit_application() {
    constexpr auto wrapped = libasn::der::explicit_application<1>(libasn::der::octet_string);
    auto           viewer  = basic_reader{"\x61\x04\x04\x02\x12\x34"sv};
    auto           r       = wrapped.read(viewer);
    assert(viewer.size() == 0);
    assert(r.has_value());
    assert(r->view() == "\x12\x34"sv);
}

void test_der_context_specific_chaining() {
    constexpr auto t      = libasn::der::integer.context_specific<3>();
    auto           viewer = basic_reader{"\x83\x01\x2A"sv};
    auto           r      = t.read(viewer);
    assert(viewer.size() == 0);
    assert(r.has_value());
    assert(*r == 42);
}

} // namespace

int main() {
    test_ber_octet_utf_printable();
    test_ber_boolean_and_null();
    test_ber_integers();
    test_ber_utc_time();
    test_ber_sequence();
    test_ber_enumerated();

    test_der_primitives();
    test_der_enumerated();
    test_der_sequence_and_sequence_of();
    test_der_explicit_context_specific();
    test_der_explicit_application();
    test_der_context_specific_chaining();

    return 0;
}
