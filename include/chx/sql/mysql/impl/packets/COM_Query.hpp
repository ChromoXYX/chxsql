#pragma once

#include <array>
#include <cstdint>
#include "../../basic_types.hpp"

namespace chx::sql::mysql::detail::packets {
struct COM_Query {
    // 1. CLIENT_QUERY_ATTRIBUTES should not be set
    // 2. SQL query should be sent with async_write_sequence_exactly
    // 3. ie, should never rely on chx.ser2 to generate binary data

    template <typename SQLQuery>
    constexpr std::array<unsigned char, 5>
    construct_header(std::uint8_t& next_sequence_id, const SQLQuery& query) {
        std::array<unsigned char, 5> ret;
        fixed_length_integer_to_network<3>(std::size(query), ret.data());
        fixed_length_integer_to_network<1>(next_sequence_id++, ret.data() + 3);
        ret[4] = 0x03;
        return ret;
    }
};
}  // namespace chx::sql::mysql::detail::packets