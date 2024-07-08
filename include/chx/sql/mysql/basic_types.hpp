#pragma once

#include <cstring>
#include <cstdint>

namespace chx::sql::mysql {
template <std::size_t N, typename CharT>
inline std::uint64_t
fixed_length_integer_from_network(const CharT* ptr) noexcept(true) {
    std::uint64_t r = {};
    std::memcpy(&r, ptr, N);
    return r;
}
template <std::size_t N, typename CharT>
inline CharT* fixed_length_integer_to_network(std::uint64_t n,
                                              CharT* ptr) noexcept(true) {
    return (CharT*)std::memcpy(ptr, &n, N);
}
}  // namespace chx::sql::mysql
