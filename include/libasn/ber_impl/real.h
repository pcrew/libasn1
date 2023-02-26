#pragma once

namespace libasn {
namespace ber {
namespace internal {

/* TODO: implement */
struct real_type {
    template <typename ValueType>
    int operator()(ValueType &&value) const {
        return std::forward<ValueType>(value);
    }

    template <typename Reader>
    std::optional<double> read(Reader &reader) const {
        auto length = reader.size();

        if (length == 0) {
            return 0;
        }

        auto first_byte = reader.read();
        if (!first_byte.has_value()) {
            return std::nullopt;
        }

        if ((*first_byte & 0x80) >> 7) {
            return decode_binary(reader, first_byte);
        } else if ((*first_byte & 0xC0) >> 6 == 0) {
            return decode_decimal(reader, first_byte);
        } else if ((*first_byte & 0xC0) >> 6 == 1) {
            return decode_special_value(reader, first_byte);
        }

        return std::nullopt;
    }

private:
    template <typename Reader>
    auto decode_binary(Reader &reader, uint8_t first_byte) {
        return 0.0;
    }

    template <typename Reader>
    auto decode_decimal(Reader &reader, uint8_t first_byte) {
        return 0.0;
    }

    template <typename Reader>
    auto decode_special_value(Reader &reader, uint8_t first_byte) {
        switch (first_byte) {
            case 0x40:
                return std::numeric_limits<double>::infinity();
            case 0x41:
                return -std::numeric_limits<double>::infinity();
            case 0x42:
                return std::numeric_limits<double>::quiet_NaN();
            case 0x43:
                return -0.0;
        }
    }
};

}  // namespace internal
}  // namespace ber
}  // namespace libasn
