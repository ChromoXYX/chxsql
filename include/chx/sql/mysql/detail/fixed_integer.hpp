#pragma once

#include <algorithm>
#include <cstring>
#include <cstdint>

namespace chx::sql::mysql::detail {
template <std::size_t N, typename Iterator>
inline std::uint64_t
fixed_length_integer_from_network(Iterator ptr) noexcept(true) {
    if constexpr (N > 1) {
        std::uint64_t r = {};
        std::copy_n(ptr, N, (std::uint8_t*)&r);
        return r;
    } else {
        return *ptr;
    }
}
template <std::size_t N, typename Iterator>
inline void fixed_length_integer_to_network(std::uint64_t n,
                                            Iterator ptr) noexcept(true) {
    if constexpr (N > 1) {
        std::copy_n((std::uint8_t*)&n, N, ptr);
    } else if constexpr (N == 1) {
        *ptr = n;
    }
}
}  // namespace chx::sql::mysql::detail
