#pragma once

#include "../make_success.hpp"
#include "../basic_type.hpp"

namespace chx::sql::postgresql::detail::msg {
struct command_complete {
    std::string command_tag;

    static constexpr std::uint8_t message_type = 'C';
    constexpr ParseResult on_message_type(std::uint8_t type) noexcept(true) {
        return type == message_type ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t) noexcept(true) {
        return ParseSuccess;
    }

    ParseResult on_body(const unsigned char* begin,
                        const unsigned char* end) noexcept(true) {
        ParseResult r = __M_parser(begin, end);
        if (r == ParseSuccess) {
            command_tag = std::move(__M_parser.value());
            __M_parser = {};
            return make_success(begin, end);
        } else if (r == ParseNeedMore) {
            return ParseNeedMore;
        } else {
            return r;
        }
    }

  private:
    string<> __M_parser;
};
}  // namespace chx::sql::postgresql::detail::msg