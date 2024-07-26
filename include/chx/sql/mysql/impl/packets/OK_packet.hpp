#pragma once

#include <chx/ser2/rule.hpp>
#include <chx/ser2/struct_getter.hpp>
#include <chx/ser2/if.hpp>
#include <cstdint>

#include "../basic_rules.hpp"
#include "../../capabilities_flags.hpp"

namespace chx::sql::mysql::detail::packets {
struct OK_Packet {
    std::uint8_t type;
    std::size_t affected_rows;
    std::size_t last_insert_id;
    std::uint16_t server_status;
    std::uint16_t warning_count;

    std::string info;
    std::string session_state_info;

    template <std::uint8_t Header>
    constexpr auto rule(std::uint32_t client_cap) noexcept(true) {
        return ser2::rule{
            ser2::bind(rules::fixed_length_integer<1>{},
                       ser2::struct_getter<&OK_Packet::type>{},
                       [](auto& self, const auto& target, auto&&...) {
                           return target == Header
                                      ? ser2::ParseResult::Ok
                                      : ser2::ParseResult::Malformed;
                       }),
            ser2::bind(rules::length_encoded_integer{},
                       ser2::struct_getter<&OK_Packet::affected_rows>{}),
            ser2::bind(rules::length_encoded_integer{},
                       ser2::struct_getter<&OK_Packet::last_insert_id>{}),
            ser2::bind(rules::fixed_length_integer<2>{},
                       ser2::struct_getter<&OK_Packet::server_status>{}),
            ser2::bind(rules::fixed_length_integer<2>{},
                       ser2::struct_getter<&OK_Packet::warning_count>{}),

            ser2::bind(ser2::if_(
                [](const auto&, const auto&, const auto& begin,
                   const auto& end) { return begin != end; },
                ser2::rule(
                    ser2::bind(rules::length_encoded_string{},
                               ser2::struct_getter<&OK_Packet::info>{}),
                    ser2::if_(
                        [client_cap](const auto& target, auto&&...) {
                            return target.server_status & (1 << 14) &&
                                   client_cap &
                                       capabilities_flags::CLIENT_SESSION_TRACK;
                        },
                        ser2::bind(rules::length_encoded_string{},
                                   ser2::struct_getter<
                                       &OK_Packet::session_state_info>{})))))};
    }
};
}  // namespace chx::sql::mysql::detail::packets