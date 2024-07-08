#pragma once

#include "../basic_rules.hpp"
#include "../../capabilities_flags.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <chx/ser2/rule.hpp>
#include <chx/ser2/bind.hpp>
#include <chx/ser2/struct_getter.hpp>
#include <chx/ser2/if.hpp>
#include <chx/ser2/list.hpp>

namespace chx::sql::mysql::detail::packets {
template <typename AuthResponse> struct handshake_response {
    // why size of attrs in bytes?
    constexpr static auto rule_attr() noexcept(true) {
        return ser2::rule{ser2::bind(
            ser2::list{ser2::rule{ser2::bind(rules::length_encoded_string{},
                                             [](const auto& target, auto&) {
                                                 return target.first;
                                             }),
                                  ser2::bind(rules::length_encoded_string{},
                                             [](const auto& target, auto&) {
                                                 return target.second;
                                             })}},
            ser2::default_getter{})};
    }
    constexpr static auto rule() noexcept(true) {
        return ser2::rule{
            ser2::bind(rules::fixed_length_integer<4>{},
                       ser2::struct_getter<&handshake_response::client_flag>{}),
            ser2::bind(
                rules::fixed_length_integer<4>{},
                ser2::struct_getter<&handshake_response::max_packet_size>{}),
            ser2::bind(
                rules::fixed_length_integer<1>{},
                ser2::struct_getter<&handshake_response::character_set>{}),
            rules::reserved<23>{},
            ser2::bind(rules::null_terminated_string{},
                       ser2::struct_getter<&handshake_response::username>{}),
            ser2::bind(
                rules::length_encoded_string{},
                ser2::struct_getter<&handshake_response::auth_response>{}),
            ser2::if_(
                [](const auto& target, auto&&...) -> bool {
                    return target.client_flag &
                           capabilities_flags::CLIENT_CONNECT_WITH_DB;
                },
                ser2::bind(
                    rules::null_terminated_string{},
                    ser2::struct_getter<&handshake_response::database>{})),
            ser2::if_(
                [](const auto& target, auto&&...) -> bool {
                    return target.client_flag &
                           capabilities_flags::CLIENT_PLUGIN_AUTH;
                },
                ser2::bind(rules::null_terminated_string{},
                           ser2::struct_getter<
                               &handshake_response::client_plugin_name>{})),
            ser2::if_(
                [](const handshake_response& target, auto&&...) -> bool {
                    return target.client_flag &
                           capabilities_flags::CLIENT_CONNECT_ATTRS;
                },
                ser2::rule(
                    ser2::bind(rules::length_encoded_integer{},
                               [](const handshake_response& target,
                                  auto&) -> std::size_t {
                                   return rule_attr().calculate_size(
                                       target.connection_attrs);
                               }),
                    ser2::bind(rule_attr(),
                               ser2::struct_getter<
                                   &handshake_response::connection_attrs>{})))};
    }

    std::uint32_t client_flag;
    std::uint32_t max_packet_size;
    std::uint8_t character_set;

    std::string username;
    AuthResponse auth_response;
    std::string database;
    std::string_view client_plugin_name;

    std::vector<std::pair<std::string, std::string>> connection_attrs;
};
}  // namespace chx::sql::mysql::detail::packets