#pragma once

#include "../make_success.hpp"
#include "../basic_type.hpp"
#include <cassert>
#include <variant>

namespace chx::sql::postgresql::detail::msg {
struct notice_response {
    static constexpr std::uint8_t message_type = 'N';
    constexpr ParseResult on_message_type(std::uint8_t v) noexcept(true) {
        return v == 'N' ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t v) noexcept(true) {
        return ParseSuccess;
    }

    constexpr ParseResult on_body(const unsigned char* begin,
                                  const unsigned char* end) {
        switch (__M_parsers.index()) {
        case 0: {
            if (begin == end) {
                return ParseNeedMore;
            }
            const std::uint8_t type = *(begin++);
            if (type) {
                __M_parsers.template emplace<1>();
            } else {
                return make_success(begin, end);
            }
        }
        case 1: {
            string<nop_container>& parser = *std::get_if<1>(&__M_parsers);
            if (ParseResult r = parser(begin, end); r == ParseSuccess) {
                __M_parsers.template emplace<0>();
                return on_body(begin, end);
            } else {
                return r;
            }
        }
        default: {
            assert(false);
        }
        }
    }

  private:
    std::variant<std::monostate, string<nop_container>> __M_parsers;
};
}  // namespace chx::sql::postgresql::detail::msg