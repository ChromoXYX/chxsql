#pragma once

#include "./basic_types.hpp"
#include <cassert>

namespace chx::sql::mysql::detail::packets2 {
struct EOF_Packet {
    std::uint16_t warning_count = 0;
    std::uint16_t server_status = 0;

    struct parser;
};

struct EOF_Packet::parser {
    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end) {
        switch (stage) {
        case Header: {
            if (begin != end) {
                if (*(begin++) == 0xfe) {
                    stage = WarningCount;
                } else {
                    return PayloadMalformedHeader;
                }
            } else {
                return PayloadNeedMore;
            }
        }
        case WarningCount: {
            if (PayloadResult r = __M_parser(begin, end); r == PayloadSuccess) {
                __M_result.warning_count = __M_parser.value();
                __M_parser = {};
                stage = ServerStatus;
            } else {
                return r;
            }
        }
        case ServerStatus: {
            if (PayloadResult r = __M_parser(begin, end); r == PayloadSuccess) {
                __M_result.server_status = __M_parser.value();
                __M_parser = {};
                return PayloadSuccess;
            } else {
                return r;
            }
        }
        default: {
            assert(false);
        }
        }
    }

    EOF_Packet value() noexcept(true) { return std::move(__M_result); }

  private:
    enum Stage { Header, WarningCount, ServerStatus } stage = Header;

    fixed_length_integer<2> __M_parser = {};
    EOF_Packet __M_result = {};
};
}  // namespace chx::sql::mysql::detail::packets2