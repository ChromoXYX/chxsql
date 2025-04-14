#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>
#include <chx/net/detail/accumulate_size.hpp>
#include <chx/net/binary_container.hpp>
#include <array>
#include "./encode_param.hpp"
#include "./basic_type.hpp"

namespace chx::sql::postgresql::detail {
template <typename... Ts>
inline auto create_bind_msg(std::string_view prepared_statement_name,
                            std::tuple<Ts...> tp) {
    auto body = std::apply(
        [](auto&&... ts) {
            return std::tuple(encode_param(std::forward<decltype(ts)>(ts))...);
        },
        std::move(tp));
    const std::uint32_t body_sz = net::detail::accumulate_size(body);

    const std::uint32_t header_len =
        5 + 1 + prepared_statement_name.size() + 1 + 2 + 2;
    std::vector<unsigned char> header(header_len);
    unsigned char* ptr = header.data();
    *(ptr++) = 'B';
    ptr = integer_to_network(std::uint32_t{header_len - 1 + body_sz + 2}, ptr);
    ++ptr;
    ptr = string_to_network(prepared_statement_name.begin(),
                            prepared_statement_name.end(), ptr);
    ptr += 2;
    ptr = integer_to_network((unsigned short){sizeof...(Ts)}, ptr);
    assert(ptr == header.data() + header.size());

    net::binary_container<unsigned short> tail = {};
    return std::tuple(std::move(header), std::move(body), std::move(tail));
}
inline auto create_DES_msg() noexcept(true) {
    std::array<unsigned char, 7 + 10 + 5> raw = {};
    unsigned char* ptr = raw.data();
    *(ptr++) = 'D';
    ptr = integer_to_network(std::uint32_t{6}, ptr);
    *(ptr++) = 'P';
    ++ptr;

    *(ptr++) = 'E';
    ptr = integer_to_network(std::uint32_t{9}, ptr);
    ptr += 5;

    *(ptr++) = 'S';
    ptr = integer_to_network(std::uint32_t{4}, ptr);
    return raw;
}
inline auto create_parse_msg(std::string_view pattern) noexcept(true) {
    const std::uint32_t p_len = 5 + 1 + pattern.size() + 1 + 2;
    std::vector<unsigned char> raw(p_len);
    unsigned char* ptr = raw.data();
    *(ptr++) = 'P';
    ptr = integer_to_network(p_len - 1, ptr);
    ++ptr;
    ptr = string_to_network(pattern.begin(), pattern.end(), ptr);
    ptr += 2;
    return std::move(raw);
}
}  // namespace chx::sql::postgresql::detail
