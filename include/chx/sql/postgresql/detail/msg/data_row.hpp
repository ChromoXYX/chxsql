#pragma once

#include "../make_success.hpp"
#include "../basic_type.hpp"

#include <cassert>
#include <optional>
#include <variant>

namespace chx::sql::postgresql::detail::msg {
struct data_row {
    data_row(std::uint16_t col_n) : data(col_n) {}

    std::vector<std::optional<std::vector<unsigned char>>> data;

    static constexpr std::uint8_t message_type = 'D';
    constexpr ParseResult on_message_type(std::uint8_t type) noexcept(true) {
        return type == message_type ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t) noexcept(true) {
        return ParseSuccess;
    }

    ParseResult on_body(const unsigned char* begin, const unsigned char* end) {
        while (begin <= end) {
            switch (__M_state.index()) {
            case 0: {
                integer<std::uint16_t>& parser = *std::get_if<0>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    if (parser.value() == data.size()) {
                        if (!data.empty()) {
                            __M_state.template emplace<1>();
                        } else {
                            return make_success(begin, end);
                        }
                    } else {
                        return ParseMalformed;
                    }
                } else {
                    return r;
                }
            }
            case 1: {
                integer<std::uint32_t>& parser = *std::get_if<1>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    const std::uint32_t len = parser.value();
                    if (len != -1) {
                        __M_state.template emplace<2>(len);
                    } else {
                        data[__M_idx] = std::nullopt;
                        if (++__M_idx < data.size()) {
                            __M_state.template emplace<1>();
                            break;
                        } else {
                            return make_success(begin, end);
                        }
                    }
                } else {
                    return r;
                }
            }
            case 2: {
                byte<>& parser = *std::get_if<2>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    data[__M_idx] = std::move(parser.value());
                    if (++__M_idx >= data.size()) {
                        return make_success(begin, end);
                    } else {
                        __M_state.template emplace<1>();
                    }
                } else {
                    return r;
                }
            }
            }
        }
        assert(false);
    }

  private:
    std::uint16_t __M_idx = 0;
    std::variant<integer<std::uint16_t>, integer<std::uint32_t>, byte<>>
        __M_state;
};
}  // namespace chx::sql::postgresql::detail::msg