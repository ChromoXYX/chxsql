#pragma once

#include "../basic_rules.hpp"
#include <array>

#include <chx/ser2/struct_getter.hpp>
#include <chx/ser2/ignore.hpp>

namespace chx::sql::mysql::detail::packets {
struct ColumnDefinition41 {
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

    constexpr static auto rule() {
        return ser2::rule(
            rules::exactly([]() -> auto& {
                static std::array<unsigned char, 4> __c = {0x03, 0x64, 0x65,
                                                           0x66};
                return __c;
            }),
            ser2::bind(rules::length_encoded_string{}, ser2::ignore_getter{},
                       ser2::default_require{}, ser2::ignore_setter{}),
            ser2::bind(rules::length_encoded_string{}, ser2::ignore_getter{},
                       ser2::default_require{}, ser2::ignore_setter{}),
            ser2::bind(rules::length_encoded_string{}, ser2::ignore_getter{},
                       ser2::default_require{}, ser2::ignore_setter{}),
            ser2::bind(rules::length_encoded_string{},
                       ser2::struct_getter<&ColumnDefinition41::name>{}),
            ser2::bind(rules::length_encoded_string{}, ser2::ignore_getter{},
                       ser2::default_require{}, ser2::ignore_setter{}),

            rules::exactly([]() -> auto& {
                static std::array<unsigned char, 1> __c = {0x0c};
                return __c;
            }),

            ser2::bind(
                rules::fixed_length_integer<2>{},
                ser2::struct_getter<&ColumnDefinition41::character_set>{}),
            ser2::bind(
                rules::fixed_length_integer<4>{},
                ser2::struct_getter<&ColumnDefinition41::column_length>{}),
            ser2::bind(rules::fixed_length_integer<1>{},
                       ser2::struct_getter<&ColumnDefinition41::type>{}),
            ser2::bind(rules::fixed_length_integer<2>{},
                       ser2::struct_getter<&ColumnDefinition41::flags>{}),
            ser2::bind(rules::fixed_length_integer<1>{},
                       ser2::struct_getter<&ColumnDefinition41::decimals>{}),
            // why the unused 2 bytes not mentioned in docs?
            rules::reserved<2>{});
    }
};
}  // namespace chx::sql::mysql::detail::packets