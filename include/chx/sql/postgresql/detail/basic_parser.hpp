#pragma once

#include "./basic_type.hpp"
#include "./msg/error_response.hpp"
#include "./msg/parameter_status.hpp"
#include "./msg/notice_response.hpp"
#include "./result.hpp"
#include <variant>
#include <cassert>

namespace chx::sql::postgresql::detail {
template <typename Message> struct basic_parser {
    static_assert(!std::is_same_v<Message, msg::error_response> &&
                  !std::is_same_v<Message, msg::notice_response> &&
                  !std::is_same_v<Message, msg::parameter_status>);

  private:
    std::variant<std::monostate, integer<std::uint32_t>, std::monostate>
        __M_parser = {};
    std::uint32_t body_remain = 0;

    std::variant<Message, msg::error_response, msg::notice_response,
                 msg::parameter_status>
        __M_result;

  public:
    constexpr ParseResult operator()(const unsigned char*& begin,
                                     const unsigned char* end) {
        while (begin < end) {
            switch (__M_parser.index()) {
            case 0: {
                const std::uint8_t type = *(begin++);
                ParseResult r = ParseInternalError;

                switch (type) {
                case 'E': {
                    __M_result.template emplace<1>();
                    r = std::get_if<1>(&__M_result)->on_message_type(type);
                    break;
                }
                case 'N': {
                    __M_result.template emplace<2>();
                    r = std::get_if<2>(&__M_result)->on_message_type(type);
                    break;
                }
                case 'S': {
                    __M_result.template emplace<3>();
                    r = std::get_if<3>(&__M_result)->on_message_type(type);
                    break;
                }
                default: {
                    r = std::get<0>(__M_result).on_message_type(type);
                }
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
                    case 2: {
                        r = std::get_if<2>(&__M_result)
                                ->on_message_length(body_remain);
                        break;
                    }
                    case 3: {
                        r = std::get_if<3>(&__M_result)
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
                case 2: {
                    r = std::get_if<2>(&__M_result)
                            ->on_body(begin - consumed, begin);
                    break;
                }
                case 3: {
                    r = std::get_if<3>(&__M_result)
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

    constexpr std::variant<Message, msg::error_response, msg::notice_response,
                           msg::parameter_status>
    value() noexcept(true) {
        return std::move(__M_result);
    }
};

template <> struct basic_parser<void> {
  private:
    std::variant<std::monostate, integer<std::uint32_t>, std::monostate>
        __M_parser = {};
    std::uint32_t body_remain = 0;
    std::variant<msg::error_response, msg::notice_response,
                 msg::parameter_status>
        __M_result;

  public:
    constexpr ParseResult operator()(const unsigned char*& begin,
                                     const unsigned char* end) {
        while (begin < end) {
            switch (__M_parser.index()) {
            case 0: {
                assert(__M_result.index() == 0);
                const std::uint8_t type = *(begin++);
                ParseResult r = ParseInternalError;

                switch (type) {
                case 'E': {
                    r = std::get_if<0>(&__M_result)->on_message_type(type);
                    break;
                }
                case 'N': {
                    __M_result.template emplace<1>();
                    r = std::get_if<1>(&__M_result)->on_message_type(type);
                    break;
                }
                case 'S': {
                    __M_result.template emplace<2>();
                    r = std::get_if<2>(&__M_result)->on_message_type(type);
                    break;
                }
                default: {
                    r = ParseMalformed;
                }
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
                    case 2: {
                        r = std::get_if<2>(&__M_result)
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
                case 2: {
                    r = std::get_if<2>(&__M_result)
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

    constexpr std::variant<msg::error_response, msg::notice_response,
                           msg::parameter_status>
    value() noexcept(true) {
        return std::move(__M_result);
    }
};
}  // namespace chx::sql::postgresql::detail