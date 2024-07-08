#pragma once

#include <chx/ser2/bind.hpp>
#include <chx/ser2/rule.hpp>
#include <chx/ser2/struct_getter.hpp>
#include "./basic_rules.hpp"

namespace chx::sql::mysql::detail {
template <typename Payload> struct packet {
    std::uint32_t payload_length;
    std::uint8_t sequence_id;

    Payload payload;

    template <typename PayloadRule>
    constexpr auto rule(std::uint8_t& next_sequence_id,
                        PayloadRule&& payload_rule) noexcept(true) {
        return ser2::rule(
            ser2::bind(rules::fixed_length_integer<3>{},
                       ser2::struct_getter<&packet::payload_length>{},
                       [](const std::uint32_t& target, const auto&,
                          const auto& begin, const auto& end) {
                           return (std::distance(begin, end) >= target + 1)
                                      ? ser2::ParseResult::Ok
                                      : ser2::ParseResult::Incomplete;
                       }),
            ser2::bind(rules::fixed_length_integer<1>{},
                       ser2::struct_getter<&packet::sequence_id>{},
                       [&next_sequence_id](const std::uint8_t& target) {
                           return (next_sequence_id++ == target)
                                      ? ser2::ParseResult::Ok
                                      : ser2::ParseResult::Malformed;
                       }),
            ser2::bind(std::forward<PayloadRule>(payload_rule),
                       ser2::struct_getter<&packet::payload>{}));
    }
};
}  // namespace chx::sql::mysql::detail