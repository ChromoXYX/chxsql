#pragma once

#include "./basic_type.hpp"
#include "./msg/error_response.hpp"
#include "./result.hpp"
#include <variant>
#include <cassert>

namespace chx::sql::postgresql::detail {
template <typename Message> struct basic_parser {
  private:
    std::variant<std::monostate, integer<std::uint32_t>, std::monostate>
        __M_parser = {};
    std::uint32_t body_remain = 0;

    std::variant<Message, msg::error_response> __M_result;

  public:
    constexpr ParseResult operator()(const unsigned char*& begin,
                                     const unsigned char* end) {
        while (begin < end) {
            switch (__M_parser.index()) {
            case 0: {
                const std::uint8_t type = *(begin++);
                ParseResult r = ParseInternalError;
                if (type != 'E') {
                    r = std::get<0>(__M_result).on_message_type(type);
                } else {
                    __M_result.template emplace<1>();
                    r = std::get_if<1>(&__M_result)->on_message_type(type);
                }
                if (r == ParseSuccess) {
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
                    if (body_remain <= 4) {
                        return ParseMalformedIncomplete;
                    }
                    switch (__M_result.index()) {
                    case 0: {
                        r = std::get_if<0>(&__M_result)
                                ->on_message_length(body_remain);
                        break;
                    }
                    case 1: {
                        r = std::get_if<1>(&__M_result)
                                ->on_message_length(body_remain);
                        break;
                    }
                    default:
                        assert(false);
                    }
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
                ParseResult r = ParseInternalError;
                switch (__M_result.index()) {
                case 0: {
                    r = std::get_if<0>(&__M_result)
                            ->on_body(begin - consumed, begin);
                    break;
                }
                case 1: {
                    r = std::get_if<1>(&__M_result)
                            ->on_body(begin - consumed, begin);
                    break;
                }
                default:
                    assert(false);
                }
                if (r == ParseSuccess) {
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
        return ParseNeedMore;
    }

    constexpr std::variant<Message, msg::error_response>
    value() noexcept(true) {
        return std::move(__M_result);
    }
};

template <> struct basic_parser<msg::error_response> {
  private:
    std::variant<std::monostate, integer<std::uint32_t>, std::monostate>
        __M_parser = {};
    std::uint32_t body_remain = 0;

    msg::error_response __M_result;

  public:
    constexpr ParseResult operator()(const unsigned char*& begin,
                                     const unsigned char* end) {
        while (begin < end) {
            switch (__M_parser.index()) {
            case 0: {
                const std::uint8_t type = *(begin++);
                ParseResult r = __M_result.on_message_type(type);
                if (r == ParseSuccess) {
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
                    if (body_remain <= 4) {
                        return ParseMalformedIncomplete;
                    }
                    r = __M_result.on_message_length(body_remain);
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
                ParseResult r = __M_result.on_body(begin - consumed, begin);
                if (r == ParseSuccess) {
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
        return ParseNeedMore;
    }

    constexpr msg::error_response value() noexcept(true) {
        return std::move(__M_result);
    }
};

}  // namespace chx::sql::postgresql::detail