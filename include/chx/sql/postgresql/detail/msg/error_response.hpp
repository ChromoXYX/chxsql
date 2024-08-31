#pragma once

#include "../basic_type.hpp"
#include "../make_success.hpp"
#include "../../error_code.hpp"
#include <cassert>
#include <variant>

namespace chx::sql::postgresql::detail::msg {
struct error_response {
    errc ec;

    constexpr ParseResult on_message_type(std::uint8_t v) noexcept(true) {
        return v == 'E' ? ParseSuccess : ParseMalformed;
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
            if (type == 'C') {
                __M_parsers.template emplace<1>(6);
                return on_body(begin, end);
            } else if (type) {
                __M_parsers.template emplace<2>();
                return on_body(begin, end);
            } else {
                return make_success(begin, end);
            }
        }
        case 1: {
            integer_array<char>& parser = *std::get_if<1>(&__M_parsers);
            ParseResult r = parser(begin, end);
            if (r == ParseSuccess) {
                std::vector<char> v = parser.value();
                ec = sqlstate_to_errc({v.begin(), v.end() - 1});
                __M_parsers.template emplace<0>();
                return on_body(begin, end);
            } else {
                return r;
            }
        }
        case 2: {
            string<nop_container>& parser = *std::get_if<2>(&__M_parsers);
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
    std::variant<std::monostate, integer_array<char>, string<nop_container>>
        __M_parsers;
};
}  // namespace chx::sql::postgresql::detail::msg