#pragma once

struct basic_reader {
    using value_type = std::string_view;

    value_type string;

    auto view() { return string; }
    auto data() { return string.data(); }
    auto size() { return string.size(); }
    auto empty() { return string.empty(); }

    std::optional<uint8_t> view_byte() {
        if (empty()) {
            return std::nullopt;
        }

        return string.front();
    }

    std::optional<uint8_t> read() {
        if (empty()) {
            return std::nullopt;
        }

        auto result = string.front();
        string.remove_prefix(1);

        return result;
    }

    std::optional<value_type> read(size_t length) {
        if (length > string.size()) {
            return std::nullopt;
        }

        auto result = string.substr(0, length);
        remove(length);
        return result;
    }

    void remove(size_t bytes) {
        auto need_removed = bytes > string.size() ? string.size() : bytes;
        string.remove_prefix(need_removed);
    }
};

