#pragma once

#include <cstddef>
#include <vector>
#include <variant>
#include <chx/ser2/list.hpp>

#include "./ColumnDefinition41.hpp"
#include "./ERR_Packet.hpp"
#include "./OK_packet.hpp"
#include "./variant.hpp"
#include "./packet.hpp"

namespace chx::sql::mysql::detail::packets {
struct NULL_field {};

struct Resultset {
    // 1. CLIENT_OPTIONAL_RESULTSET_METADATA should not be set
    // 2. CLIENT_DEPRECATE_EOF should be set

    std::size_t column_count;
    std::vector<ColumnDefinition41> field_metadata;
    std::vector<std::vector<std::variant<NULL_field, std::string>>>
        text_resultset_row;
    std::variant<ERR_Packet, OK_Packet> terminator;

    constexpr auto rule(std::uint8_t& next_sequence_id) {
        return ser2::rule(
            ser2::bind(rules::length_encoded_integer{},
                       ser2::struct_getter<&Resultset::column_count>{}),
            rules::repeat(
                ser2::bind(packet(next_sequence_id, ColumnDefinition41::rule()),
                           ser2::struct_getter<&Resultset::field_metadata>{}),
                ser2::struct_getter<&Resultset::column_count>{}),
            ser2::list(ser2::bind(
                packet(next_sequence_id,
                       rules::repeat(
                           variant(
                               [](std::variant<NULL_field, std::string>&,
                                  const auto& ctx, const auto& begin,
                                  const auto& end) -> std::size_t {
                                   if (std::distance(begin, end) > 0) {
                                       return *begin == 0xfb ? 0 : 1;
                                   } else {
                                       return -1;
                                   }
                               },
                               rules::reserved<1>{},
                               ser2::bind(rules::length_encoded_string{})),
                           [](const auto&, const auto& ctx) {
                               return ctx.target.column_count;
                           })),
                ser2::struct_getter<&Resultset::text_resultset_row>{})));
    }
};
}  // namespace chx::sql::mysql::detail::packets