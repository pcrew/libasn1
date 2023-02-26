#pragma once

namespace libasn {
namespace ber {
namespace internal {

struct integer_type {
    template <typename ValueType>
    int operator()(ValueType &&value) const {
        return std::forward<ValueType>(value);
    }

    template <typename Reader>
    std::optional<intmax_t> read(Reader &reader) const {
        auto length = reader.size();

        if (length > sizeof(intmax_t)) {
            reader.remove(length);
            return std::numeric_limits<intmax_t>::max();
        }

        auto first_byte = reader.read();
        if (!first_byte) {
            return 0;
        }
        auto value = static_cast<uintmax_t>(*first_byte);

        for (auto s = length - 1; s; --s) {
            auto byte = reader.read();
            if (!byte) {
                return std::nullopt;
            }
            value = (value << 8) | *byte;
        }

        return (*first_byte & 0x80) == 0 ? value : ((~static_cast<uintmax_t>(0) << (length << 3)) | value);
    }
};

}  // namespace internal
}  // namespace ber
}  // namespace libasn
