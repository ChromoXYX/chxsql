#pragma once

#include "./basic_types.hpp"

#include <vector>
#include <optional>

namespace chx::sql::mysql::detail::packets2 {
struct Resultset_Packet {
    std::vector<std::optional<std::string>> column_data;

    struct parser;
};

struct Resultset_Packet::parser {
    PayloadResult operator()(std::size_t column_n, const unsigned char*& begin,
                             const unsigned char* end) {
        if (__M_result.column_data.size() != column_n) {
            switch (stage) {
            case Guess: {
                if (begin == end) {
                    return PayloadNeedMore;
                }
                const std::uint8_t first_byte = *begin;
                if (first_byte == 0xfb) {
                    __M_result.column_data.emplace_back(std::nullopt);
                    ++begin;
                    return operator()(column_n, begin, end);
                } else {
                    stage = LncStr;
                }
            }
            case LncStr: {
                if (PayloadResult r = __M_parser(begin, end);
                    r == PayloadSuccess) {
                    __M_result.column_data.emplace_back(__M_parser.value());
                    __M_parser = {};
                    stage = Guess;
                    return operator()(column_n, begin, end);
                } else {
                    return r;
                }
            }
            default: {
                return PayloadInternalError;
            }
            }
        } else {
            return PayloadSuccess;
        }
    }

    Resultset_Packet value() noexcept(true) { return std::move(__M_result); }

  private:
    enum Stage { Guess, LncStr } stage = Guess;

    length_encoded_string<> __M_parser = {};
    Resultset_Packet __M_result = {};
};
}  // namespace chx::sql::mysql::detail::packets2