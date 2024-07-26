#pragma once

#include <array>

#include <chx/ser2/rule.hpp>
#include <chx/ser2/bind.hpp>
#include <chx/ser2/if.hpp>
#include <chx/ser2/struct_getter.hpp>

#include "../basic_rules.hpp"
#include "../../capabilities_flags.hpp"

namespace chx::sql::mysql::detail::packets {
struct ERR_Packet {
    std::uint8_t header;
    std::uint16_t error_code;

    std::array<unsigned char, 5> sql_state;
    std::string error_message;

    constexpr auto rule(std::uint32_t client_cap) noexcept(true) {
        return ser2::rule(
            ser2::bind(rules::fixed_length_integer<1>(),
                       ser2::struct_getter<&ERR_Packet::header>{},
                       [](auto& self, std::uint8_t target, auto&&...) {
                           return target == 0xff ? ser2::ParseResult::Ok
                                                 : ser2::ParseResult::Malformed;
                       }),
            ser2::bind(rules::fixed_length_integer<2>{},
                       ser2::struct_getter<&ERR_Packet::error_code>{}),
            ser2::if_(
                [client_cap](const ERR_Packet& packet, auto&&...) {
                    return client_cap & capabilities_flags::CLIENT_PROTOCOL_41;
                },
                ser2::rule(
                    rules::exactly([]() -> auto& {
                        static std::array<std::uint8_t, 1> a = {'#'};
                        return a;
                    }),
                    ser2::bind(rules::fixed_length_string<5>{},
                               ser2::struct_getter<&ERR_Packet::sql_state>{}))),
            ser2::bind(rules::rest_of_packet_string{},
                       ser2::struct_getter<&ERR_Packet::error_message>{}));
    }
};
}  // namespace chx::sql::mysql::detail::packets