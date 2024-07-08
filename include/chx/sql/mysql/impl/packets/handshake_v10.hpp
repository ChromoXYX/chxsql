#pragma once

#include <cstdint>
#include <vector>
#include <chx/ser2/parse_result.hpp>
#include <chx/ser2/rule.hpp>
#include <chx/ser2/struct_getter.hpp>
#include <chx/ser2/bind.hpp>
#include <chx/ser2/skip.hpp>
#include <chx/ser2/if.hpp>

#include "../basic_rules.hpp"
#include "../../capabilities_flags.hpp"

namespace chx::sql::mysql::detail::packets {
struct handshake_v10 {
    constexpr static auto rule() noexcept(true) {
        return ser2::rule(
            ser2::bind(rules::fixed_length_integer<1>{},
                       ser2::struct_getter<&handshake_v10::protocol_version>{},
                       [](const auto& target, auto&&...) -> ser2::ParseResult {
                           return target == 10 ? ser2::ParseResult::Ok
                                               : ser2::ParseResult::Malformed;
                       }),
            ser2::bind(rules::null_terminated_string{},
                       ser2::struct_getter<&handshake_v10::server_version>{}),
            ser2::bind(rules::fixed_length_integer<4>{},
                       ser2::struct_getter<&handshake_v10::thread_id>{}),
            ser2::bind(
                rules::fixed_length_string<8>{},
                ser2::struct_getter<&handshake_v10::auth_plugin_data_part_1>{}),
            ser2::bind(rules::fixed_length_integer<1>{},
                       ser2::struct_getter<&handshake_v10::filler>{}),
            ser2::bind(
                rules::fixed_length_integer<2>{},
                ser2::struct_getter<&handshake_v10::capability_flags_1>{}),
            ser2::bind(rules::fixed_length_integer<1>{},
                       ser2::struct_getter<&handshake_v10::character_set>{}),
            ser2::bind(rules::fixed_length_integer<2>{},
                       ser2::struct_getter<&handshake_v10::status_flags>{}),
            ser2::bind(
                rules::fixed_length_integer<2>{},
                ser2::struct_getter<&handshake_v10::capability_flags_2>{}),
            ser2::if_(
                [](const auto& target, auto&&...) -> bool {
                    std::uint32_t __cap = target.capability_flags_1 +
                                          (target.capability_flags_2 << 2 * 8);
                    return __cap & capabilities_flags::CLIENT_PLUGIN_AUTH;
                },
                ser2::bind(
                    rules::fixed_length_integer<1>{},
                    ser2::struct_getter<&handshake_v10::auth_plugin_data_len>{},
                    [](const std::uint8_t& target,
                       auto&&...) -> ser2::ParseResult {
                        return target > 8 ? ser2::ParseResult::Ok
                                          : ser2::ParseResult::Malformed;
                    }),
                rules::reserved<1>{}),
            rules::reserved<10>{},
            ser2::bind(rules::variable_length_string(
                           ser2::struct_getter<
                               &handshake_v10::auth_plugin_data_part_2>{},
                           [](const handshake_v10& p, auto& ctx) {
                               return std::max(ssize_t{13},
                                               static_cast<ssize_t>(
                                                   p.auth_plugin_data_len - 8));
                           }),
                       ser2::default_getter{}),
            ser2::if_(
                [](const auto& target, auto&&...) -> bool {
                    std::uint32_t __cap = target.capability_flags_1 +
                                          (target.capability_flags_2 << 2 * 8);
                    return __cap & capabilities_flags::CLIENT_PLUGIN_AUTH;
                },
                ser2::bind(
                    rules::null_terminated_string{},
                    ser2::struct_getter<&handshake_v10::auth_plugin_name>{})));
    }

    std::uint8_t protocol_version;
    std::vector<unsigned char> server_version;
    std::uint32_t thread_id;
    std::vector<unsigned char> auth_plugin_data_part_1;
    std::uint8_t filler;
    std::uint16_t capability_flags_1;
    std::uint8_t character_set;
    std::uint16_t status_flags;
    std::uint16_t capability_flags_2;
    std::uint8_t auth_plugin_data_len;
    std::vector<unsigned char> auth_plugin_data_part_2;
    std::vector<unsigned char> auth_plugin_name;

    template <typename CharT>
    ser2::ParseResult parse(const CharT*& begin, const CharT* end) {
        return rule().parse(*this, begin, end);
    }

    std::string to_string() { return rule().to_string(*this); }
};
}  // namespace chx::sql::mysql::detail::packets
