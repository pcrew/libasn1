
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

#include "libasn/aper_reader.h"
#include "libasn/ber.h"
#include "libasn/der.h"
#include "libasn/basic_reader.h"
#include "libasn/per.h"
#include "libasn/per/detail/x691.h"
#include "libasn/per/types/per_choice.h"

using namespace std::literals;
using libasn::per_choice_extension;

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

void test_ber_real() {
    {
        auto viewer = basic_reader{"\x09\x00"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 0.0);
    }
    {
        auto viewer = basic_reader{"\x09\x01\x40"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(std::isinf(*r) && *r > 0);
    }
    {
        auto viewer = basic_reader{"\x09\x01\x41"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(std::isinf(*r) && *r < 0);
    }
    {
        auto viewer = basic_reader{"\x09\x01\x42"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(std::isnan(*r));
    }
    {
        auto viewer = basic_reader{"\x09\x01\x43"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 0.0);
        assert(std::signbit(*r));
    }
    {
        auto viewer = basic_reader{"\x09\x02\x40\x00"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(!r.has_value());
    }
    {
        auto viewer = basic_reader{"\x09\x01\x44"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(!r.has_value());
    }
    {
        auto viewer = basic_reader{"\x09\x04\x01\x31\x2E\x35"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 1.5);
    }
    {
        auto viewer = basic_reader{"\x09\x05\x01\x32\x2C\x35\x30"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 2.5);
    }
    {
        auto viewer = basic_reader{"\x09\x06\x01\x2D\x31\x2E\x32\x35"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == -1.25);
    }
    {
        auto viewer = basic_reader{"\x09\x02\x01\x30"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 0.0);
    }
    {
        auto viewer = basic_reader{"\x09\x01\x00"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(!r.has_value());
    }
    {
        auto viewer = basic_reader{"\x09\x04\x01\x58\x59\x5A"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(!r.has_value());
    }
    {
        auto viewer = basic_reader{"\x09\x03\x80\xF9\x80"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 1.0);
    }
    {
        auto viewer = basic_reader{"\x09\x03\x80\xFF\x03"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 1.5);
    }
    {
        auto viewer = basic_reader{"\x09\x03\xC0\xF9\x80"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == -1.0);
    }
    {
        auto viewer = basic_reader{"\x09\x04\x81\xFF\xFF\x03"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value());
        assert(*r == 1.5);
    }
    {
        auto viewer = basic_reader{"\x09\x03\xB0\xF9\x80"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(!r.has_value());
    }
    {
        auto viewer = basic_reader{"\x09\x02\x83\x00"sv};
        auto r      = libasn::ber::real.read(viewer);
        assert(viewer.size() == 0);
        assert(!r.has_value());
    }
}

void test_der_real() {
    {
        auto viewer = basic_reader{"\x09\x00"sv};
        auto r      = libasn::der::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value() && *r == 0.0);
    }
    {
        auto viewer = basic_reader{"\x09\x01\x40"sv};
        auto r      = libasn::der::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value() && std::isinf(*r) && *r > 0);
    }
    {
        auto viewer = basic_reader{"\x09\x04\x01\x31\x2E\x35"sv};
        auto r      = libasn::der::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value() && *r == 1.5);
    }
    {
        auto viewer = basic_reader{"\x09\x03\x80\xF9\x80"sv};
        auto r      = libasn::der::real.read(viewer);
        assert(viewer.size() == 0);
        assert(r.has_value() && *r == 1.0);
    }
    {
        auto viewer = basic_reader{"\x09\x01\x44"sv};
        auto r      = libasn::der::real.read(viewer);
        assert(viewer.size() == 0);
        assert(!r.has_value());
    }
}

void test_ber_utc_time() {
    auto viewer   = basic_reader{"\x17\x0D\x32\x30\x30\x39\x30\x32\x31\x33\x32\x35\x32\x36\x5A"sv};
    auto utc_time = libasn::ber::utc_time.read(viewer);
    assert(utc_time.has_value());
    assert(utc_time->view() == "200902132526Z"sv);
}

void test_ber_object_identifier_components() {
    auto arcs = libasn::ber::object_identifier_components("\x2B\x06\x01"sv);
    assert(arcs.has_value());
    assert(arcs->size() == 4U);
    assert((*arcs)[0] == 1U && (*arcs)[1] == 3U && (*arcs)[2] == 6U && (*arcs)[3] == 1U);
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

void test_per_reader_msb_order() {
    libasn::aper_reader r{"\x80"sv};
    auto                b0 = r.read_bit();
    auto                b1 = r.read_bit();
    assert(b0.has_value() && *b0 == true);
    assert(b1.has_value() && *b1 == false);
    assert(r.bit_pos == 2U);
}

void test_per_codec_length_and_integer() {
    {
        libasn::aper_reader r{"\x04"sv};
        auto                ld = libasn::per_codec::length_determinant(r, -1, 0);
        assert(ld.has_value() && !ld->second && ld->first == 4U);
    }
    {
        libasn::aper_reader r{"\x00\x55"sv};
        auto v = libasn::per_codec::integer_constrained_root(0, static_cast<std::intmax_t>(65535), r);
        assert(v.has_value() && *v == 85);
        assert(r.empty());
    }
}

enum class per_tri : std::uint8_t { A = 0, B = 1, C = 2 };

void test_per_types_dsl() {
    {
        libasn::aper_reader           r{"\x2A"sv};
        constexpr auto                codec = libasn::per::integer<0, 255>();
        std::optional<std::intmax_t> v      = codec.read(r);
        assert(v.has_value() && *v == 42);
        assert(r.empty());
    }
    {
        libasn::aper_reader r{"\x80"sv};
        auto                b = libasn::per::boolean.read(r);
        assert(b.has_value() && *b == true);
        assert(r.bit_pos == 1U);
    }
    {
        libasn::aper_reader r{"\x00"sv};
        auto                e = libasn::per::enumerated<per_tri, 3>().read(r);
        assert(e.has_value() && *e == per_tri::A);
        assert(r.bit_pos == 2U);
    }
    {
        constexpr auto pair = libasn::per::sequence(libasn::per::integer<0, 15>(), libasn::per::integer<0, 15>());
        libasn::aper_reader r{"\x85"sv};
        auto                t = pair.read(r);
        assert(t.has_value());
        assert(std::get<0>(*t) == 8);
        assert(std::get<1>(*t) == 5);
        assert(r.empty());
    }
    {
        constexpr auto    list = libasn::per::sequence_of<0, 7>(libasn::per::boolean);
        libasn::aper_reader r{"\x78"sv};
        auto                v = list.read(r);
        assert(v.has_value() && v->size() == 3U);
        assert((*v)[0] == true);
        assert((*v)[1] == true);
        assert((*v)[2] == false);
    }
}

enum class per_choice_demo : std::uint8_t { alpha = 0, beta = 1 };

void test_per_choice_two_booleans() {
    constexpr auto codec = libasn::per::choice<per_choice_demo>()
                               .with<per_choice_demo::alpha>(libasn::per::boolean)
                               .with<per_choice_demo::beta>(libasn::per::boolean);
    libasn::aper_reader r{"\x40"sv};
    auto                o = codec.read(r);
    assert(o.has_value());
    assert(o->first == per_choice_demo::alpha);
    using Arm0 = libasn::internal::per_choice_arm<0, bool>;
    assert(std::holds_alternative<Arm0>(o->second));
    assert(std::get<Arm0>(o->second).value == true);
}

void test_per_choice_extension_open_type() {
    constexpr auto codec = libasn::per::choice<per_choice_demo, true>()
                               .with<per_choice_demo::alpha>(libasn::per::boolean)
                               .with<per_choice_demo::beta>(libasn::per::boolean);
    libasn::aper_reader r{"\x80\x01\x42"sv};
    auto                o = codec.read(r);
    assert(o.has_value());
    assert(std::holds_alternative<per_choice_extension>(*o));
    auto const &ext = std::get<per_choice_extension>(*o);
    assert(ext.extension_index == 0U);
    assert(ext.open_type.size() == 1U);
    assert(static_cast<unsigned char>(ext.open_type[0]) == 0x42U);
}

} // namespace

int main() {
    test_ber_octet_utf_printable();
    test_ber_boolean_and_null();
    test_ber_integers();
    test_ber_real();
    test_der_real();
    test_ber_utc_time();
    test_ber_object_identifier_components();
    test_ber_sequence();
    test_ber_enumerated();

    test_der_primitives();
    test_der_enumerated();
    test_der_sequence_and_sequence_of();
    test_der_explicit_context_specific();
    test_der_explicit_application();
    test_der_context_specific_chaining();

    test_per_reader_msb_order();
    test_per_codec_length_and_integer();
    test_per_types_dsl();
    test_per_choice_two_booleans();
    test_per_choice_extension_open_type();

    return 0;
}
