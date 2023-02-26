#pragma once

namespace libasn {
namespace ber {
namespace internal {

struct null_type {
    constexpr auto operator()(nullptr_t value = nullptr) const { return value; }

    template <typename Reader>
    auto read(Reader &reader) const {
        return std::optional(nullptr);
    }
};

}  // namespace internal
}  // namespace ber
}  // namespace libasn
