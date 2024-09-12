#pragma once

#include "./basic_type.hpp"
#include "./result.hpp"
#include <variant>
#include <cassert>

namespace chx::sql::postgresql::detail {
template <typename Message> struct basic_parser {
  private:
    std::variant<std::monostate, integer<std::uint32_t>, std::monostate>
        __M_parser = {};
    std::uint32_t body_remain = 0;

  public:
    constexpr basic_parser() = default;
    constexpr basic_parser(const basic_parser&) = default;
    constexpr basic_parser(basic_parser&&) = default;
    constexpr basic_parser(Message&& r) : result(std::move(r)) {}

    Message result;

    constexpr ParseResult operator()(const unsigned char*& begin,
                                     const unsigned char* end) {
        while (begin <= end) {
            switch (__M_parser.index()) {
            case 0: {
                if (begin == end) {
                    return ParseNeedMore;
                }
                const std::uint8_t type = *(begin++);
                if (ParseResult r = result.on_message_type(type);
                    r == ParseSuccess) {
                    __M_parser.template emplace<1>();
                    break;
                } else if (r == ParseMalformed || r == ParseInternalError) {
                    return r;
                } else {
                    assert(false);
                }
            }
            case 1: {
                integer<std::uint32_t>& parser = std::get<1>(__M_parser);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    body_remain = parser.value();
                    if (body_remain < 4) {
                        return ParseMalformedIncomplete;
                    }
                    ParseResult r = result.on_message_length(body_remain);
                    body_remain -= 4;
                    if (r == ParseSuccess) {
                        __M_parser.template emplace<2>();
                        break;
                    } else if (r == ParseMalformed || r == ParseInternalError) {
                        return r;
                    } else {
                        assert(false);
                    }
                } else if (r == ParseNeedMore) {
                    return r;
                } else {
                    assert(false);
                }
            }
            case 2: {
                const std::size_t consumed = std::min(
                    static_cast<std::size_t>(body_remain),
                    static_cast<std::size_t>(std::distance(begin, end)));
                body_remain -= consumed;
                begin += consumed;
                if (ParseResult r = result.on_body(begin - consumed, begin);
                    r == ParseSuccess) {
                    if (!body_remain) {
                        return ParseSuccess;
                    } else {
                        return ParseMalformedTrailingData;
                    }
                } else if (r == ParseNeedMore) {
                    if (body_remain) {
                        return ParseNeedMore;
                    } else {
                        return ParseMalformedIncomplete;
                    }
                } else {
                    return r;
                }
            }
            }
        }
        assert(false);
    }
};
}  // namespace chx::sql::postgresql::detail