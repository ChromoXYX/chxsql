#pragma once

#include <cstdint>
#include "../make_success.hpp"

namespace chx::sql::postgresql::detail::msg {
struct parse_complete {
    std::uint8_t status = 0;
    static constexpr std::uint8_t message_type = '1';
    constexpr ParseResult on_message_type(std::uint8_t type) const
        noexcept(true) {
        return type == message_type ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t len) const
        noexcept(true) {
        return len == 4 ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_body(const unsigned char* begin,
                                  const unsigned char* end) noexcept(true) {
        return make_success(begin, end);
    }
};
}  // namespace chx::sql::postgresql::detail::msg
