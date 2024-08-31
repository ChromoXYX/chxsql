#pragma once

#include "./basic_types.hpp"
#include "../result.hpp"
#include <cassert>
#include <variant>

namespace chx::sql::mysql::detail::packets2 {
struct ERR_Packet {
    std::uint16_t error_code;
    std::string sql_state;
    std::string error_message;

    struct parser;
};

struct ERR_Packet::parser {
    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end, bool is_final) {
        switch (stage) {
        case Header: {
            auto& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                if (parser.value() == 0xff) {
                    parser = {};
                    __M_parsers.template emplace<1>();
                    stage = ErrorCode;
                } else {
                    return PayloadMalformedHeader;
                }
            } else {
                return r;
            }
        }
        case ErrorCode: {
            auto& parser = std::get<1>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.error_code = parser.value();
                parser = {};

                stage = SqlStateMarker;
            } else {
                return r;
            }
        }
        case SqlStateMarker: {
            if (begin != end) {
                const std::uint8_t marker = *(begin++);
                if (marker == '#') {
                    __M_parsers.template emplace<2>();
                    stage = SqlState;
                } else {
                    return PayloadMalformed;
                }
            } else {
                return PayloadNeedMore;
            }
        }
        case SqlState: {
            auto& parser = std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.sql_state = parser.value();
                parser = {};

                stage = ErrorMessage;
            } else {
                return r;
            }
        }
        case ErrorMessage: {
            __M_result.error_message.append(begin, end);
            begin = end;
            return is_final ? PayloadSuccess : PayloadNeedMore;
        }
        default: {
            assert(false);
        }
        }
    }

    ERR_Packet value() noexcept(true) { return std::move(__M_result); }

  private:
    enum Stage : std::uint8_t {
        Header,
        ErrorCode,
        SqlStateMarker,
        SqlState,
        ErrorMessage
    } stage = Header;

    std::variant<fixed_length_integer<1>, fixed_length_integer<2>,
                 fixed_length_string<5>>
        __M_parsers;

    ERR_Packet __M_result;
};
}  // namespace chx::sql::mysql::detail::packets2