#pragma once

#include "./basic_types.hpp"
#include <cassert>
#include <variant>

namespace chx::sql::mysql::detail::packets2 {
struct ColumnDefinition41_Packet {
    // std::string schema;
    // std::string table;
    // std::string org_table;
    std::string name;
    // std::string org_name;

    std::uint16_t character_set;
    std::uint32_t column_length;
    std::uint8_t type;
    std::uint16_t flags;
    std::uint8_t decimals;

    struct parser;
};

struct ColumnDefinition41_Packet::parser {
    constexpr parser() noexcept(true) : __M_parsers(std::in_place_index<1>) {}

    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end) {
        switch (stage) {
        case Catalog: {
            length_encoded_string<>& parser = std::get<1>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                std::string val = parser.value();
                parser = {};
                if (val == "def") {
                    __M_parsers.template emplace<2>();
                    stage = Schema;
                } else {
                    return PayloadMalformed;
                }
            } else {
                return r;
            }
        }
        case Schema: {
            length_encoded_string<nop_container>& parser =
                std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                parser = {};
                stage = Table;
            } else {
                return r;
            }
        }
        case Table: {
            length_encoded_string<nop_container>& parser =
                std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                parser = {};
                stage = OrgTable;
            } else {
                return r;
            }
        }
        case OrgTable: {
            length_encoded_string<nop_container>& parser =
                std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                parser = {};
                __M_parsers.template emplace<1>();
                stage = Name;
            } else {
                return r;
            }
        }
        case Name: {
            length_encoded_string<>& parser = std::get<1>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.name = parser.value();
                parser = {};
                __M_parsers.template emplace<2>();
                stage = OrgName;
            } else {
                return r;
            }
        }
        case OrgName: {
            length_encoded_string<nop_container>& parser =
                std::get<2>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                parser = {};
                __M_parsers.template emplace<0>();
                stage = LengthOfFixedLengthFields;
            } else {
                return r;
            }
        }
        case LengthOfFixedLengthFields: {
            fixed_length_integer<1>& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                if (parser.value() == 0x0c) {
                    parser = {};
                    __M_parsers.template emplace<4>();
                    stage = CharacterSet;
                } else {
                    return PayloadMalformed;
                }
            } else {
                return r;
            }
        }
        case CharacterSet: {
            fixed_length_integer<2>& parser = std::get<4>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.character_set = parser.value();
                parser = {};
                __M_parsers.template emplace<5>();
                stage = ColumnLength;
            } else {
                return r;
            }
        }
        case ColumnLength: {
            fixed_length_integer<4>& parser = std::get<5>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.column_length = parser.value();
                parser = {};
                __M_parsers.template emplace<0>();
                stage = Type;
            } else {
                return r;
            }
        }
        case Type: {
            fixed_length_integer<1>& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.type = parser.value();
                parser = {};
                __M_parsers.template emplace<4>();
                stage = Flags;
            } else {
                return r;
            }
        }
        case Flags: {
            fixed_length_integer<2>& parser = std::get<4>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.flags = parser.value();
                parser = {};
                __M_parsers.template emplace<0>();
                stage = Decimals;
            } else {
                return r;
            }
        }
        case Decimals: {
            fixed_length_integer<1>& parser = std::get<0>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                __M_result.decimals = parser.value();
                parser = {};
                __M_parsers.template emplace<4>();
                stage = UnusedNotInDoc;
            } else {
                return r;
            }
        }
        case UnusedNotInDoc: {
            fixed_length_integer<2>& parser = std::get<4>(__M_parsers);
            if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                parser = {};
                return PayloadSuccess;
            } else {
                return r;
            }
        }
        default: {
            assert(false);
        }
        }
    }

    ColumnDefinition41_Packet value() noexcept(true) {
        return std::move(__M_result);
    }

  private:
    enum Stage : std::uint8_t {
        Catalog,
        Schema,
        Table,
        OrgTable,
        Name,
        OrgName,
        LengthOfFixedLengthFields,
        CharacterSet,
        ColumnLength,
        Type,
        Flags,
        Decimals,
        UnusedNotInDoc
    } stage = Catalog;

    std::variant<fixed_length_integer<1>, length_encoded_string<>,
                 length_encoded_string<nop_container>, length_encoded_integer,
                 fixed_length_integer<2>, fixed_length_integer<4>>
        __M_parsers = {};

    ColumnDefinition41_Packet __M_result = {};
};
}  // namespace chx::sql::mysql::detail::packets2