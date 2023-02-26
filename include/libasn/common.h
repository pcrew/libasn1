#pragma once

#include <iostream>
#include <iomanip>
#include <string>

template <typename T>
auto bytes_to_hex(const T *bytes, std::size_t size) {
    std::stringstream ss;
    ss << std::hex;

    for (std::size_t i{0}; i < size; i++) {
        ss << " " << std::setw(2) << std::setfill('0') << std::uppercase
           << static_cast<int>(static_cast<uint8_t>(bytes[i]));
    }

    return ss.str();
}

template <typename E, typename T>
inline auto hexdump(const void *data, std::size_t size, std::basic_ostream<E, T> &stream, std::size_t width = 16) {
    const auto *start = reinterpret_cast<const char *>(data);
    const auto *end   = start + size;
    const auto *line  = start;

    while (line != end) {
        stream.width(4);
        stream.fill('0');
        stream << std::hex << line - start << " : ";

        auto line_length = std::min(width, static_cast<std::size_t>(end - line));
        for (std::size_t pass{1}; pass <= 2; pass++) {
            for (const auto *next = line; next != end && next != line + width; next++) {
                auto ch = *next;
                switch (pass) {
                    case 1:
                        stream << (ch < 32 ? '.' : ch);
                        break;
                    case 2:
                        if (next != line) stream << " ";

                        stream.width(2);
                        stream.fill('0');
                        stream << std::hex << std::uppercase << static_cast<int>(static_cast<uint8_t>(ch));
                        break;
                }
            }

            if (pass == 1 && line_length != width) {
                stream << std::string(width - line_length, ' ');
            }
            stream << " ";
        }

        stream << std::endl;
        line += line_length;
    }
}

#define __DEBUG__(msg) printf("%s() - %s\n", __PRETTY_FUNCTION__, msg)
