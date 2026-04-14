#pragma once

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace libasn {
namespace internal {

struct real_type {
    template <typename ValueType>
    int operator()(ValueType &&value) const {
        return std::forward<ValueType>(value);
    }

    template <typename Reader>
    std::optional<double> read(Reader &reader) const {
        if (reader.empty()) {
            return 0.0;
        }

        auto total = reader.size();
        auto ob0   = reader.read();
        if (!ob0) {
            return std::nullopt;
        }
        auto const octv = *ob0;

        if ((octv & 0xC0U) == 0x40U) {
            return __from_special_ieee754(octv, reader);
        }
        if ((octv & 0xC0U) == 0x00U) {
            if (octv == 0 || (octv & 0x3CU) != 0) {
                return std::nullopt;
            }

            return __from_nr1_decimal(reader);
        }

        auto sign  = (octv & 0x40) != 0;
        auto baseF = __binary_exponent_base_factor(octv);
        if (!baseF) {
            return std::nullopt;
        }
        int scaleF = static_cast<int>((octv & 0x0CU) >> 2);

        auto elen_flag = octv & 3U;
        if (total <= 1U + elen_flag) {
            return std::nullopt;
        }

        int32_t expval = 0;
        if (elen_flag == 3U) {
            auto oec = reader.read();
            if (!oec) {
                return std::nullopt;
            }
            auto ecount = *oec;
            if (ecount == 0 || total <= 2U + ecount) {
                return std::nullopt;
            }
            auto oe0 = reader.read();
            if (!oe0) {
                return std::nullopt;
            }
            expval = static_cast<int32_t>(static_cast<int8_t>(*oe0));
            for (uint8_t j = 0; j < ecount; ++j) {
                auto const bx = reader.read();
                if (!bx) {
                    return std::nullopt;
                }
                expval = static_cast<int32_t>(static_cast<int64_t>(expval) * 256 + *bx);
            }
        } else {
            auto const ob1 = reader.read();
            if (!ob1) {
                return std::nullopt;
            }
            expval = static_cast<int32_t>(static_cast<int8_t>(*ob1));
            for (uint8_t j = 0; j < elen_flag; ++j) {
                auto const bx = reader.read();
                if (!bx) {
                    return std::nullopt;
                }
                expval = static_cast<int32_t>(static_cast<int64_t>(expval) * 256 + *bx);
            }
        }

        double m = 0.0;
        while (!reader.empty()) {
            auto const bx = reader.read();
            if (!bx) {
                return std::nullopt;
            }
            m = std::ldexp(m, 8) + *bx;
        }

        m = std::ldexp(m, expval * *baseF + scaleF);
        if (!std::isfinite(m)) {
            return std::nullopt;
        }
        return sign ? -m : m;
    }

private:
    static std::optional<int> __binary_exponent_base_factor(unsigned char first_octet) {
        switch ((first_octet & 0x30U) >> 4) {
        case 0:
            return 1;
        case 1:
            return 3;
        case 2:
            return 4;
        default:
            return std::nullopt;
        }
    }

    template <typename Reader>
    std::optional<double> __from_special_ieee754(unsigned char octv, Reader &reader) const {
        double ret = 0;
        switch (octv) {
        case 0x40:
            ret = std::numeric_limits<double>::infinity();
            break;
        case 0x41:
            ret = -std::numeric_limits<double>::infinity();
            break;
        case 0x42:
            ret = std::numeric_limits<double>::quiet_NaN();
            break;
        case 0x43:
            ret = -0.0;
            break;
        default:
            return std::nullopt;
        }
        if (!reader.empty()) {
            return std::nullopt;
        }
        return ret;
    }

    // TODO: remove copy
    template <typename Reader>
    std::optional<double> __from_nr1_decimal(Reader &reader) const {
        std::vector<char> tmp;
        tmp.reserve(reader.size());
        while (!reader.empty()) {
            auto const bx = reader.read();
            if (!bx) {
                return std::nullopt;
            }
            tmp.push_back(static_cast<char>(*bx));
        }
        for (char &c : tmp) {
            if (c == ',') {
                c = '.';
            }
        }
        double     d{};
        auto const res = std::from_chars(tmp.data(), tmp.data() + tmp.size(), d);
        if (res.ec != std::errc{} || res.ptr != tmp.data() + tmp.size()) {
            return std::nullopt;
        }

        return d;
    }
};

} // namespace internal
} // namespace libasn
