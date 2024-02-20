#pragma once

#include <optional>

namespace libasn {
namespace internal {
struct boolean_type {
    auto operator()(bool value) const { return value; }

    template <typename Reader>
    auto read(Reader &reader) const {
        auto value = reader.read();
        return value ? std::optional(*value != 0x00) : std::nullopt;
    }
};

}  // namespace internal
}  // namespace libasn
