#pragma once

/*
CHXSQL.MYSQL NEVER CARES ABOUT TYPES OF COLUMNS IN SQL QUERY RESULT.

data_mapper should only be used with each ROW, and with a type trait or
something else that hints the container of each row.

every field should be optional<>

there should be 2 interfaces for data_mapper
#1 data_mapper(Getters...), while the Rule of each Getter is determined by
return type of Getter
#2 data_mapper(RuleConstructor)
*/

#include <utility>
#include <chx/ser2/bind.hpp>
#include <chx/ser2/list.hpp>
#include <chx/ser2/if.hpp>
#include <vector>
#include <map>
#include <optional>

#include "./error_code.hpp"
#include "./exception.hpp"
#include "./data_setter.hpp"
#include "./impl/packets/ColumnDefinition41.hpp"
#include "./impl/basic_rules.hpp"

namespace chx::sql::mysql {
class data_mapper_exception : public exception {
  public:
    using exception::exception;
};

namespace detail {
struct map_data_mapper {
    using result_set_type =
        std::vector<std::map<std::string, std::optional<std::string>>>;
    using value_type = std::pair<std::string, std::optional<std::string>>;

  private:
    auto make_rule(const std::vector<packets::ColumnDefinition41>& def,
                   std::size_t& cnt) {
        return ser2::rule(ser2::list(ser2::bind(
            ser2::if_(
                [](value_type& target, auto& ctx, const auto& begin,
                   const auto& end) {
                    if (std::distance(begin, end) > 0 && *begin != 0xfb) {
                        return true;
                    } else {
                        return false;
                    }
                },
                ser2::bind(
                    rules::length_encoded_string{}, ser2::default_getter{},
                    ser2::default_require{},
                    [&def, &cnt](auto& self, auto& ctx, value_type& target,
                                 const auto& begin, const auto& end) mutable {
                        if (cnt < def.size() && std::distance(begin, end) <=
                                                    def[cnt].column_length) {
                            target.first = def[cnt].name;
                            target.second.emplace(begin, end);
                        } else {
                            throw data_mapper_exception{};
                        }
                    }),
                ser2::bind(
                    rules::exactly([]() -> auto& {
                        static std::array<std::uint8_t, 1> a = {0xfb};
                        return a;
                    }),
                    ser2::default_getter{},
                    [&def, &cnt](auto& self, value_type& target, auto&&...) {
                        if (cnt < def.size()) {
                            target.first = def[cnt].name;
                            target.second = std::nullopt;
                            return ser2::ParseResult::Ok;
                        } else {
                            throw data_mapper_exception{};
                        }
                    })),
            ser2::default_getter{}, [&cnt](auto&&...) {
                ++cnt;
                return ser2::ParseResult::Ok;
            })));
    }

  public:
    template <typename Iterator>
    std::error_code
    operator()(const std::vector<packets::ColumnDefinition41>& def,
               result_set_type& container, Iterator begin, Iterator end) {
        try {
            std::map<std::string, std::optional<std::string>> row;
            std::size_t cnt = 0;
            auto rule = make_rule(def, cnt);
            auto b = begin;
            ser2::ParseResult r = rule.parse(row, begin, end);
            if (r == ser2::ParseResult::Ok) {
                container.emplace_back(std::move(row));
                return make_ec(errc::NO_ERROR);
            } else {
                return make_ec(errc::CR_MALFORMED_PACKET);
            }
        } catch (const data_mapper_exception&) {
            return make_ec(errc::CR_UNKNOWN_ERROR);
        }
    }
};
}  // namespace detail
constexpr detail::map_data_mapper map_data_mapper() noexcept(true) {
    return {};
}
}  // namespace chx::sql::mysql