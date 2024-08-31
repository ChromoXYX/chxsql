#pragma once

#include "./basic_types.hpp"
#include "../result.hpp"
#include "../../capabilities_flags.hpp"
#include <cassert>
#include <variant>

namespace chx::sql::mysql::detail::packets2 {
struct OK_Packet {
    std::size_t affected_rows = 0;
    std::size_t last_insert_id = 0;
    std::uint16_t status_flags = 0;
    std::uint16_t warnings = 0;

    std::string info;
    std::string session_state_info;

    template <std::uint8_t Header = 0x00> struct parser;
};

template <std::uint8_t HeaderCh> struct OK_Packet::parser {
    PayloadResult operator()(std::uint32_t client_cap,
                             const unsigned char*& begin,
                             const unsigned char* end, bool is_final) {
        switch (stage) {
        case Header: {
            if (begin != end) {
                if (*(begin++) == HeaderCh) {
                    stage = AffectedRows;
                } else {
                    return PayloadMalformedHeader;
                }
            } else {
                return PayloadNeedMore;
            }
        }
        case AffectedRows: {
            auto& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.affected_rows = parser.value();
                parser = {};

                stage = LastInsertId;
            } else {
                return r;
            }
        }
        case LastInsertId: {
            auto& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.last_insert_id = parser.value();
                parser = {};

                __M_parsers.template emplace<1>();
                stage = StatusFlags;
            } else {
                return r;
            }
        }
        case StatusFlags: {
            auto& parser = std::get<1>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.status_flags = parser.value();
                parser = {};

                stage = Warnings;
            } else {
                return r;
            }
        }
        case Warnings: {
            auto& parser = std::get<1>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.warnings = parser.value();
                parser = {};

                __M_parsers.template emplace<2>();
                if (client_cap & capabilities_flags::CLIENT_SESSION_TRACK) {
                    stage = SessionTrackInfo;
                } else {
                    stage = Info;
                }
                return operator()(client_cap, begin, end, is_final);
            } else {
                return r;
            }
        }
        case SessionTrackInfo: {
            auto& parser = std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r != PayloadSuccess) {
                __M_result.info = parser.value();
                parser = {};

                if (__M_result.status_flags & (1 << 14)) {
                    stage = SessionTrackStateInfo;
                } else {
                    return PayloadSuccess;
                }
            } else {
                return r;
            }
        }
        case SessionTrackStateInfo: {
            auto& parser = std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r != PayloadSuccess) {
                __M_result.session_state_info = parser.value();
                parser = {};

                return PayloadSuccess;
            } else {
                return r;
            }
        }
        case Info: {
            __M_result.info.append(begin, end);
            begin = end;
            return is_final ? PayloadSuccess : PayloadNeedMore;
        }
        default: {
            assert(false);
        }
        }
    }

    OK_Packet value() noexcept(true) { return std::move(__M_result); }

  private:
    enum Stage {
        Header,
        AffectedRows,
        LastInsertId,
        StatusFlags,
        Warnings,

        SessionTrackInfo,
        SessionTrackStateInfo,
        Info
    } stage = Header;

    std::variant<length_encoded_integer, fixed_length_integer<2>,
                 length_encoded_string<>>
        __M_parsers;

    OK_Packet __M_result = {};
};
}  // namespace chx::sql::mysql::detail::packets2