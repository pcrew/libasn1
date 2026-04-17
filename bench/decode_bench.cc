#include <benchmark/benchmark.h>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "libasn/basic_reader.h"
#include "libasn/ber.h"
#include <libasn/protocols/ldap/ldap.h>

namespace {

std::vector<std::uint8_t> g_bytes;
std::string_view          g_view;

bool load_fixture(const char *path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "open " << path << ": " << std::strerror(errno) << "\n";
        return false;
    }

    f.seekg(0, std::ios::end);
    auto n = static_cast<size_t>(f.tellg());
    f.seekg(0);
    g_bytes.resize(n);
    f.read(reinterpret_cast<char *>(g_bytes.data()), static_cast<std::streamsize>(n));
    g_view = std::string_view{reinterpret_cast<const char *>(g_bytes.data()), g_bytes.size()};
    return true;
}

[[gnu::noinline]] std::uint64_t libasn_full_decode_digest(std::string_view sv) {
    /* protocol_op is choice<int> with APPLICATION tag numbers as discriminants. */
    constexpr int k_search_result_entry = 4;
    constexpr auto partial_attribute    = libasn::ber::sequence(
        libasn::ldap::attribute_description, libasn::ber::set_of(libasn::ldap::attribute_value));

    basic_reader rd{sv};
    auto         msg = libasn::ldap::ldap_message.read(rd);
    if (unlikely(!msg.has_value() || !rd.empty())) {
        return 0;
    }
    auto [message_id, command, controls] = *msg;
    (void)message_id;
    (void)controls;

    if (unlikely(command.tag_number != k_search_result_entry)) {
        return 0;
    }

    auto [dn, attrs] = command.get<k_search_result_entry>();
    (void)dn;

    std::uint64_t ret = 0;
    basic_reader  ar{attrs.view()};

    while (!ar.empty()) {
        auto pa = partial_attribute.read(ar);
        if (unlikely(!pa.has_value())) {
            return 0;
        }
        auto [desc, vals] = *pa;
        ret ^= static_cast<std::uint64_t>(desc.view().size());
        basic_reader vr{vals.view()};
        while (!vr.empty()) {
            auto o = libasn::ber::octet_string.read(vr);
            if (unlikely(!o.has_value())) {
                return 0;
            }
            ret ^= static_cast<std::uint64_t>(o->view().size());
        }
    }
    return ret;
}

static std::vector<std::uint8_t> g_copy_buf;

void BM_libasn_pdu_header_only(benchmark::State &state) {
    for (auto _ : state) {
        basic_reader rd{g_view};
        auto         msg = libasn::ldap::ldap_message.read(rd);
        if (!msg.has_value() || !rd.empty()) {
            state.SkipWithError("decode failed or trailing bytes");
            break;
        }
        auto [message_id, command, controls] = *msg;
        benchmark::DoNotOptimize(message_id);
        benchmark::DoNotOptimize(command);
        benchmark::DoNotOptimize(controls);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(g_view.size()));
}

void BM_libasn_full_decode_all_attrs(benchmark::State &state) {
    std::uint64_t sink = 0;
    for (auto _ : state) {
        sink ^= libasn_full_decode_digest(g_view);
        benchmark::DoNotOptimize(sink);
    }
    benchmark::ClobberMemory();
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(g_view.size()));
}

void BM_memcpy_fixture(benchmark::State &state) {
    g_copy_buf.resize(g_bytes.size());
    for (auto _ : state) {
        std::memcpy(g_copy_buf.data(), g_bytes.data(), g_bytes.size());
        benchmark::DoNotOptimize(g_copy_buf.data());
    }
    benchmark::ClobberMemory();
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(g_view.size()));
}

BENCHMARK(BM_libasn_pdu_header_only);
BENCHMARK(BM_libasn_full_decode_all_attrs);
BENCHMARK(BM_memcpy_fixture);

} // namespace

int main(int argc, char **argv) {
    static const char *const fallback[] = {
        "../fixtures/search_result_entry_large.bin",
        nullptr,
    };

    bool loaded = false;
    for (std::size_t i = 0; fallback[i] != nullptr; ++i) {
        loaded = load_fixture(fallback[i]);
        break;
    }
    if (!loaded) {
        std::cerr << "Fixture not found." << std::endl;
        return 1;
    }

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
