#pragma once

#include "../make_success.hpp"
#include "../basic_type.hpp"
#include "../../result_set.hpp"

#include <cassert>
#include <variant>

namespace chx::sql::postgresql::detail::msg {
struct row_description {
    std::vector<result_set::row_description_type> desc;

    static constexpr std::uint8_t message_type = 'T';
    constexpr ParseResult on_message_type(std::uint8_t type) noexcept(true) {
        return type == message_type ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t) noexcept(true) {
        return ParseSuccess;
    }

    constexpr ParseResult on_body(const unsigned char* begin,
                                  const unsigned char* end) {
        while (begin <= end) {
            switch (__M_state.index()) {
            case 0: {
                integer<std::uint16_t>& parser = *std::get_if<0>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    const std::uint16_t sz = parser.value();
                    if (sz) {
                        desc.resize(parser.value());
                        __M_state.template emplace<1>();
                    } else {
                        return make_success(begin, end);
                    }
                } else {
                    return r;
                }
            }
            case 1: {
                string<>& parser = *std::get_if<1>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    desc[__M_idx].field_name = parser.value();
                    __M_state.template emplace<2>();
                } else {
                    return r;
                }
            }
            case 2: {
                integer<std::uint32_t>& parser = *std::get_if<2>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    desc[__M_idx].table_id = parser.value();
                    __M_state.template emplace<3>();
                } else {
                    return r;
                }
            }
            case 3: {
                integer<std::uint16_t>& parser = *std::get_if<3>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    desc[__M_idx].column_attribute_number = parser.value();
                    __M_state.template emplace<4>();
                } else {
                    return r;
                }
            }
            case 4: {
                integer<std::uint32_t>& parser = *std::get_if<4>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    desc[__M_idx].field_data_type_id = parser.value();
                    __M_state.template emplace<5>();
                } else {
                    return r;
                }
            }
            case 5: {
                integer<std::uint16_t>& parser = *std::get_if<5>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    desc[__M_idx].data_type_size = parser.value();
                    __M_state.template emplace<6>();
                } else {
                    return r;
                }
            }
            case 6: {
                integer<std::uint32_t>& parser = *std::get_if<6>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    desc[__M_idx].type_modifier = parser.value();
                    __M_state.template emplace<7>();
                } else {
                    return r;
                }
            }
            case 7: {
                integer<std::uint16_t>& parser = *std::get_if<7>(&__M_state);
                ParseResult r = parser(begin, end);
                if (r == ParseSuccess) {
                    desc[__M_idx].format_code = parser.value();
                    if (++__M_idx < desc.size()) {
                        __M_state.template emplace<1>();
                    } else {
                        return make_success(begin, end);
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
    std::variant<integer<std::uint16_t>, string<>, integer<std::uint32_t>,
                 integer<std::uint16_t>, integer<std::uint32_t>,
                 integer<std::uint16_t>, integer<std::uint32_t>,
                 integer<std::uint16_t>>
        __M_state;
};
}  // namespace chx::sql::postgresql::detail::msg