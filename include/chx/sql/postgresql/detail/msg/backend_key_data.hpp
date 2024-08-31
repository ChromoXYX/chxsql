#pragma once

#include "../make_success.hpp"
#include "../basic_type.hpp"

namespace chx::sql::postgresql::detail::msg {
struct backend_key_data {
    std::uint32_t process_id = 0, secret_key = 0;

    constexpr ParseResult on_message_type(std::uint8_t type) const
        noexcept(true) {
        return type == 'K' ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t len) const
        noexcept(true) {
        return len == 12 ? ParseSuccess : ParseMalformed;
    }

    constexpr ParseResult on_body(const unsigned char* begin,
                                  const unsigned char* end) noexcept(true) {
        ParseResult r = parser(begin, end);
        if (r == ParseSuccess) {
            if (stage == S1) {
                process_id = parser.value();
                parser = {};
                stage = S2;
                return on_body(begin, end);
            } else {
                secret_key = parser.value();
                return make_success(begin, end);
            }
        } else {
            return r;
        }
    }

  private:
    enum Stage : std::uint8_t { S1, S2 } stage = S1;
    integer<std::uint32_t> parser = {};
};
}  // namespace chx::sql::postgresql::detail::msg