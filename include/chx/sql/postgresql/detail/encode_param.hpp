#pragma once

#include <netinet/in.h>
#include <string>
#include <type_traits>
#include <chx/net/buffer.hpp>
#include <chx/net/binary_container.hpp>
#include <chx/net/detail/remove_rvalue_reference.hpp>

namespace chx::sql::postgresql::detail {
template <typename T> inline auto encode_param(T&& t) {
    if constexpr (std::is_arithmetic_v<T>) {
        return encode_param(std::to_string(t));
    } else {
        net::const_buffer b = net::buffer(t);
        return std::tuple<
            net::binary_container<std::uint32_t>,
            typename net::detail::remove_rvalue_reference<T&&>::type>(
            htonl(b.size()), std::forward<T>(t));
    }
}
}  // namespace chx::sql::postgresql::detail
