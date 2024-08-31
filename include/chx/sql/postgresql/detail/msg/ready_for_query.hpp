#pragma once

#include "../make_success.hpp"
#include <cstdint>

namespace chx::sql::postgresql::detail::msg {
struct ready_for_query {
    std::uint8_t status = 0;

    constexpr ParseResult on_message_type(std::uint8_t type) const
        noexcept(true) {
        return type == 'Z' ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t len) const
        noexcept(true) {
        return len == 5 ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_body(const unsigned char* begin,
                                  const unsigned char* end) noexcept(true) {
        if (begin != end) {
            status = *(begin++);
            if (status == 'T' || status == 'E' || status == 'I') {
                return make_success(begin, end);
            } else {
                return ParseMalformed;
            }
        } else {
            return ParseNeedMore;
        }
    }
};
}  // namespace chx::sql::postgresql::detail::msg