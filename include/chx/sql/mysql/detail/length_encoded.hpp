#pragma once

#include <cstddef>
#include <cstring>
#include <iterator>

namespace chx::sql::mysql::detail {
constexpr std::size_t lenenc_int_bound(std::size_t n) noexcept(true) {
    if (n < 251) {
        return 1;
    } else if (n < (1 << 16)) {
        return 3;
    } else if (n < (1 << 24)) {
        return 4;
    } else {
        return 9;
    }
}
constexpr unsigned char* lenenc_integer(std::size_t n,
                                        unsigned char* dest) noexcept(true) {
    if (n < 251) {
        *(dest++) = n;
    } else if (n < (1 << 16)) {
        *(dest++) = 0xfc;
        ::memcpy(dest, &n, 2);
        dest += 2;
    } else if (n < (1 << 24)) {
        *(dest++) = 0xfd;
        ::memcpy(dest, &n, 3);
        dest += 3;
    } else {
        *(dest++) = 0xfe;
        ::memcpy(dest, &n, 8);
        dest += 8;
    }
    return dest;
}

template <typename Iterator>
constexpr std::size_t lenenc_str_bound(const Iterator& begin,
                                       const Iterator& end) noexcept(true) {
    const std::size_t dis = std::distance(begin, end);
    return lenenc_int_bound(dis) + dis;
}
template <typename Iterator>
constexpr unsigned char* lenenc_str(const Iterator& begin, const Iterator& end,
                                    unsigned char* dest) noexcept(true) {
    return std::copy(begin, end,
                     lenenc_integer(std::distance(begin, end), dest));
}
}  // namespace chx::sql::mysql::detail