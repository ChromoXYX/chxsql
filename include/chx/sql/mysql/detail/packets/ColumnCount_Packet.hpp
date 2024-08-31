#pragma once

#include "./basic_types.hpp"

namespace chx::sql::mysql::detail::packets2 {
struct ColumnCount_Packet {
    std::size_t column_count;

    struct parser;
};

struct ColumnCount_Packet::parser {
    PayloadResult operator()(const unsigned char*& begin, const unsigned char* end) {
        return __M_parser(begin, end);
    }

    constexpr ColumnCount_Packet value() noexcept(true) {
        return {__M_parser.value()};
    }

  private:
    length_encoded_integer __M_parser = {};
};
}  // namespace chx::sql::mysql::detail::packets
