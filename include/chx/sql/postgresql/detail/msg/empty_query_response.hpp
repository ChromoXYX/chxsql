#pragma once

#include "../make_success.hpp"
#include <cstdint>

namespace chx::sql::postgresql::detail::msg {
struct empty_query_response {
    static constexpr std::uint8_t message_type = 'I';
    constexpr ParseResult on_message_type(std::uint8_t type) noexcept(true) {
        return type == message_type ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t len) noexcept(true) {
        return len == 4 ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_body(const unsigned char* begin,
                                  const unsigned char* end) noexcept(true) {
        return make_success(begin, end);
    }
};
}  // namespace chx::sql::postgresql::detail::msg