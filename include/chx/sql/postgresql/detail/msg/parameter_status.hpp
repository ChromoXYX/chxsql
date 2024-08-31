#pragma once

#include "../make_success.hpp"
#include "../basic_type.hpp"

namespace chx::sql::postgresql::detail::msg {
struct parameter_status {
    constexpr ParseResult on_message_type(std::uint8_t type) const
        noexcept(true) {
        return type == 'S' ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t) const
        noexcept(true) {
        return ParseSuccess;
    }

    ParseResult on_body(const unsigned char* begin,
                        const unsigned char* end) noexcept(true) {
        ParseResult r = parser(begin, end);
        if (r == ParseSuccess) {
            if (!stage) {
                parser = {};
                stage = true;
                return on_body(begin, end);
            } else {
                return make_success(begin, end);
            }
        } else {
            return r;
        }
    }

  private:
    bool stage = false;
    string<nop_container> parser;
};
}  // namespace chx::sql::postgresql::detail::msg