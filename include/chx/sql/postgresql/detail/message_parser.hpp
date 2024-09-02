#pragma once

#include "./msg/error_response.hpp"
#include "./msg/parameter_status.hpp"
#include "./msg/notice_response.hpp"

#include <array>

namespace chx::sql::postgresql::detail {
template <typename... Message, std::size_t... Is>
constexpr void
msg_type_jump_table_impl(std::array<std::size_t, 256>& r,
                         std::integer_sequence<std::size_t, Is...>) {
    (..., (r[Message::message_type] = Is));
}
template <typename... Messages>
constexpr std::array<std::size_t, 256> msg_type_jump_table() {
    std::array<std::size_t, 256> r;
    std::fill(r.begin(), r.end(), -1);
    msg_type_jump_table_impl<Messages...>(
        r, std::make_integer_sequence<std::size_t, sizeof...(Messages)>{});
    return r;
}

template <typename... Messages> struct message_parser {
    constexpr ParseResult operator()(const unsigned char*& begin,
                                     const unsigned char* end) {
        while (begin < end) {
            switch (__M_state.index()) {
            case 0: {
                // guess stage
                const std::uint8_t type = *(begin++);
                __M_state.template emplace<1>();
                if (jump_table[type] != -1) {
                    ParseResult r =
                        result_emplace_and_invoke<0>(jump_table[type], type);
                    if (r == ParseSuccess) {
                        break;
                    } else if (r == ParseMalformed || r == ParseInternalError) {
                        return r;
                    } else {
                        assert(false);
                    }
                    break;
                } else {
                    return ParseMalformed;
                }
            }
            case 1: {
                integer<std::uint32_t>& parser = *std::get_if<1>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    body_remain = parser.value();
                    if (body_remain <= 4) {
                        return ParseMalformedIncomplete;
                    }
                    r = invoke_on_message_length<0>(__M_result.index(),
                                                    body_remain);
                    body_remain -= 4;
                    if (r == ParseSuccess) {
                        __M_state.template emplace<2>();
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
                ParseResult r = invoke_on_body<0>(__M_result.index(),
                                                  begin - consumed, begin);
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

    constexpr std::variant<Messages..., msg::error_response,
                           msg::parameter_status, msg::notice_response>
    value() noexcept(true) {
        return std::move(__M_result);
    }

  private:
    std::uint32_t body_remain = 0;
    std::variant<std::monostate, integer<std::uint32_t>, std::monostate>
        __M_state;
    std::variant<Messages..., msg::error_response, msg::parameter_status,
                 msg::notice_response>
        __M_result;

    static inline constexpr std::array<std::size_t, 256> jump_table =
        msg_type_jump_table<Messages..., msg::error_response,
                            msg::parameter_status, msg::notice_response>();
    template <std::size_t I>
    constexpr ParseResult result_emplace_and_invoke(std::size_t idx,
                                                    std::uint8_t type) {
        if constexpr (I < sizeof...(Messages) + 3) {
            if (I == idx) {
                return __M_result.template emplace<I>().on_message_type(type);
            } else {
                return result_emplace_and_invoke<I + 1>(idx, type);
            }
        } else {
            assert(false);
        }
    }
    template <std::size_t I>
    constexpr ParseResult invoke_on_message_length(std::size_t idx,
                                                   std::uint32_t len) {
        if constexpr (I < sizeof...(Messages) + 3) {
            if (I == idx) {
                return std::get_if<I>(&__M_result)->on_message_length(len);
            } else {
                return invoke_on_message_length<I + 1>(idx, len);
            }
        } else {
            assert(false);
        }
    }
    template <std::size_t I>
    constexpr ParseResult invoke_on_body(std::size_t idx,
                                         const unsigned char* begin,
                                         const unsigned char* end) {
        if constexpr (I < sizeof...(Messages) + 3) {
            if (I == idx) {
                return std::get_if<I>(&__M_result)->on_body(begin, end);
            } else {
                return invoke_on_body<I + 1>(idx, begin, end);
            }
        } else {
            assert(false);
        }
    }
};
}  // namespace chx::sql::postgresql::detail
