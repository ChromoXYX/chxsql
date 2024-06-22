#pragma once

#include <cstring>
#include <cstdint>

#include "./exception.hpp"

namespace chx::sql::mysql {
using fixed_length_integer = std::uint64_t;
using length_encoded_integer = std::uint64_t;

template <std::size_t N, typename CharT>
inline std::uint64_t
fixed_length_integer_from_network(const CharT* ptr) noexcept(true) {
    std::uint64_t r = {};
    std::memcpy(&r, ptr, N);
    return r;
}
template <std::size_t N, typename CharT>
inline CharT* fixed_length_integer_to_network(std::uint64_t n,
                                              const CharT* ptr) noexcept(true) {
    return (CharT*)std::memcpy(ptr, &n, N);
}

template <typename CharT>
inline CharT* length_encoded_integer_from_network(length_encoded_integer& n,
                                                  const CharT* begin,
                                                  const CharT* end) {
    std::uint8_t first_dig = *(begin++);
    if (first_dig < 251) {
        n = first_dig;
        return begin;
    }
    length_encoded_integer ret = {};
    switch (first_dig) {
    case 0xfc: {
        if (end - begin >= 2) {
            end = begin + 2;
        } else {
            throw malformed_packet();
        }
        break;
    }
    case 0xfd: {
        if (end - begin >= 3) {
            end = begin + 3;
        } else {
            throw malformed_packet();
        }
        break;
    }
    case 0xfe: {
        if (end - begin >= 8) {
            end = begin + 8;
        } else {
            throw malformed_packet();
        }
        break;
    }
    default: {
        throw malformed_packet();
    }
    }
    std::memcpy(&n, begin, end - begin);
    return end;
}

constexpr inline std::size_t
length_encoded_integer_to_network_len(length_encoded_integer n) noexcept(true) {
    if (n < 251) {
        return 1;
    } else if (n < (1 << 16)) {
        return 3;
    } else if (n < (1 << 24)) {
        return 3;
    } else {
        return 8;
    }
}
template <typename CharT>
inline CharT* length_encoded_integer_to_network(length_encoded_integer n,
                                                CharT* ptr) noexcept(true) {
    if (n < 251) {
        return (CharT*)std::memcpy(ptr, &n, 1);
    } else if (n < (1 << 16)) {
        return (CharT*)std::memcpy(ptr + 1, &n, 2);
    } else if (n < (1 << 24)) {
        return (CharT*)std::memcpy(ptr + 1, &n, 3);
    } else {
        return (CharT*)std::memcpy(ptr + 1, &n, 8);
    }
}

template <std::size_t N>
using fixed_length_string = std::array<std::uint8_t, N>;
}  // namespace chx::sql::mysql
