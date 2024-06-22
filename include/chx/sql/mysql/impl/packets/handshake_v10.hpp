#pragma once

#include <cstdint>
#include <string>
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
    static auto& rule() noexcept(true) {
        static ser2::rule r(
            ser2::bind<rules::fixed_length_integer<1>,
                       ser2::struct_getter<&handshake_v10::protocol_version>>{},
            ser2::bind<rules::null_terminated_string,
                       ser2::struct_getter<&handshake_v10::server_version>>{},
            ser2::bind<rules::fixed_length_integer<4>,
                       ser2::struct_getter<&handshake_v10::thread_id>>{},
            ser2::bind<
                rules::fixed_length_string<8>,
                ser2::struct_getter<&handshake_v10::auth_plugin_data_part_1>>{},
            ser2::bind<rules::fixed_length_integer<1>,
                       ser2::struct_getter<&handshake_v10::filler>>{},
            ser2::bind<
                rules::fixed_length_integer<2>,
                ser2::struct_getter<&handshake_v10::capability_flags_1>>{},
            ser2::bind<rules::fixed_length_integer<1>,
                       ser2::struct_getter<&handshake_v10::character_set>>{},
            ser2::bind<rules::fixed_length_integer<2>,
                       ser2::struct_getter<&handshake_v10::status_flags>>{},
            ser2::bind<
                rules::fixed_length_integer<2>,
                ser2::struct_getter<&handshake_v10::capability_flags_2>>{},
            ser2::bind<
                rules::fixed_length_integer<1>,
                ser2::struct_getter<&handshake_v10::auth_plugin_data_len>>{},
            rules::reserved<10>{},
            ser2::bind(rules::variable_length_string(
                           ser2::struct_getter<
                               &handshake_v10::auth_plugin_data_part_2>{},
                           [](handshake_v10& p, auto& ctx) {
                               return std::max(ssize_t{13},
                                               static_cast<ssize_t>(
                                                   p.auth_plugin_data_len - 8));
                           }),
                       ser2::default_getter{}),
            ser2::bind(
                ser2::if_(
                    [](auto& target, auto& ctx) -> bool {
                        std::uint32_t __cap =
                            ctx.target.capability_flags_1 +
                            (ctx.target.capability_flags_2 << 2 * 8);
                        return __cap & capabilities_flags::CLIENT_PLUGIN_AUTH;
                    },
                    rules::null_terminated_string{}, ser2::skip{}),
                ser2::struct_getter<&handshake_v10::auth_plugin_name>{}));
        return r;
    }

    std::uint8_t protocol_version;
    std::string server_version;
    std::uint32_t thread_id;
    std::string auth_plugin_data_part_1;
    std::uint8_t filler;
    std::uint16_t capability_flags_1;
    std::uint8_t character_set;
    std::uint16_t status_flags;
    std::uint16_t capability_flags_2;
    std::uint8_t auth_plugin_data_len;
    std::string auth_plugin_data_part_2;
    std::string auth_plugin_name;

    ser2::ParseResult parse(const char*& begin, const char* end) {
        return rule().parse(*this, begin, end);
    }

    std::string to_string() { return rule().to_string(*this); }
};
}  // namespace chx::sql::mysql::detail::packets
