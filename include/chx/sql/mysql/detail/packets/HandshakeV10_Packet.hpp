#pragma once

#include "./basic_types.hpp"
#include "../result.hpp"
#include "../../capabilities_flags.hpp"

#include <cassert>
#include <variant>
#include <vector>

namespace chx::sql::mysql::detail::packets2 {
struct HandshakeV10_Packet {
    // std::uint8_t protocol_version;
    std::string server_version;
    std::uint32_t thread_id;
    std::vector<unsigned char> auth_plugin_data_part_1;
    std::uint8_t filler;
    std::uint16_t capability_flags_1;
    std::uint8_t character_set;
    std::uint16_t status_flags;
    std::uint16_t capability_flags_2;
    std::uint8_t auth_plugin_data_len;
    std::vector<unsigned char> auth_plugin_data_part_2;
    std::string auth_plugin_name;

    struct parser;
};

struct HandshakeV10_Packet::parser {
    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end) {
        switch (stage) {
        case ProtocolVersion: {
            fixed_length_integer<1>& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                if (parser.value() == 10) {
                    parser = {};
                    __M_parsers.template emplace<1>();
                    stage = ServerVersion;
                } else {
                    return PayloadMalformed;
                }
            } else {
                return r;
            }
        }
        case ServerVersion: {
            null_terminated_string<>& parser = std::get<1>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.server_version = std::move(parser.value());
                parser = {};
                __M_parsers.template emplace<2>();
                stage = ThreadId;
            } else {
                return r;
            }
        }
        case ThreadId: {
            fixed_length_integer<4>& parser = std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.thread_id = parser.value();
                parser = {};
                __M_parsers.template emplace<3>();
                stage = AuthPluginDataPart1;
            } else {
                return r;
            }
        }
        case AuthPluginDataPart1: {
            fixed_length_string<8, std::vector<unsigned char>>& parser =
                std::get<3>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.auth_plugin_data_part_1 = parser.value();
                parser = {};
                __M_parsers.template emplace<0>();
                stage = Filler;
            } else {
                return r;
            }
        }
        case Filler: {
            fixed_length_integer<1>& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                parser = {};
                __M_parsers.template emplace<4>();
                stage = CapFlags1;
            } else {
                return r;
            }
        }
        case CapFlags1: {
            fixed_length_integer<2>& parser = std::get<4>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.capability_flags_1 = parser.value();
                parser = {};
                __M_parsers.template emplace<0>();
                stage = CharacterSet;
            } else {
                return r;
            }
        }
        case CharacterSet: {
            fixed_length_integer<1>& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.character_set = parser.value();
                parser = {};
                __M_parsers.template emplace<4>();
                stage = StatusFlags;
            } else {
                return r;
            }
        }
        case StatusFlags: {
            fixed_length_integer<2>& parser = std::get<4>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.status_flags = parser.value();
                parser = {};
                __M_parsers.template emplace<4>();
                stage = CapFlags2;
            } else {
                return r;
            }
        }
        case CapFlags2: {
            fixed_length_integer<2>& parser = std::get<4>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.capability_flags_2 = parser.value();
                parser = {};
                __M_parsers.template emplace<0>();
                stage = AuthPluginDataLen;
            } else {
                return r;
            }
        }
        case AuthPluginDataLen: {
            fixed_length_integer<1>& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                const std::uint8_t data_len = parser.value();
                if (data_len <= 9) {
                    return PayloadMalformed;
                }
                __M_result.auth_plugin_data_len = parser.value();
                parser = {};
                __M_parsers.template emplace<5>();
                stage = Reserved;
            } else {
                return r;
            }
        }
        case Reserved: {
            fixed_length_string<10, nop_container>& parser =
                std::get<5>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                parser = {};
                __M_parsers.template emplace<6>(std::max(
                    13, static_cast<int>(__M_result.auth_plugin_data_len) - 8));
                stage = AuthPluginDataPart2;
            } else {
                return r;
            }
        }
        case AuthPluginDataPart2: {
            variable_length_string<std::vector<unsigned char>>& parser =
                std::get<6>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.auth_plugin_data_part_2 = std::move(parser.value());
                // parser = {};
                const std::uint32_t cap = __M_result.capability_flags_1 +
                                          (__M_result.capability_flags_1 << 16);
                if (cap & capabilities_flags::CLIENT_PLUGIN_AUTH) {
                    __M_parsers.template emplace<1>();
                    stage = AuthPluginName;
                } else {
                    return PayloadSuccess;
                }
            } else {
                return r;
            }
        }
        case AuthPluginName: {
            null_terminated_string<>& parser = std::get<1>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.auth_plugin_name = std::move(parser.value());
                parser = {};
                return PayloadSuccess;
            } else {
                return r;
            }
        }
        default:
            assert(false);
        }
    }

    constexpr HandshakeV10_Packet value() noexcept(true) {
        return std::move(__M_result);
    }

  private:
    enum Stage {
        ProtocolVersion,
        ServerVersion,
        ThreadId,
        AuthPluginDataPart1,
        Filler,
        CapFlags1,
        CharacterSet,
        StatusFlags,
        CapFlags2,
        AuthPluginDataLen,
        Reserved,
        AuthPluginDataPart2,
        AuthPluginName
    } stage = ProtocolVersion;

    std::variant<fixed_length_integer<1>, null_terminated_string<>,
                 fixed_length_integer<4>,
                 fixed_length_string<8, std::vector<unsigned char>>,
                 fixed_length_integer<2>,
                 fixed_length_string<10, nop_container>,
                 variable_length_string<std::vector<unsigned char>>>
        __M_parsers = {};
    HandshakeV10_Packet __M_result = {};
};
}  // namespace chx::sql::mysql::detail::packets2