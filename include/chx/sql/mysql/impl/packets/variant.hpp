#pragma once

#include <chx/ser2/switch.hpp>
#include <chx/ser2/bind.hpp>
#include <chx/ser2/rule.hpp>

namespace chx::sql::mysql::detail::packets {
template <typename... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts&&... ts) -> overloaded<std::decay_t<Ts>...>;

template <std::size_t I, typename Rule>
constexpr auto variant_rule(Rule&& rule) {
    return ser2::bind(
        std::forward<Rule>(rule),
        overloaded{[](auto& target, const auto& ctx) -> decltype(auto) {
                       if (target.index() != I) {
                           target.template emplace<I>();
                       }
                       return std::get<I>(target);
                   },
                   [](const auto& target, const auto& ctx) -> decltype(auto) {
                       return std::get<I>(target);
                   }});
}
template <typename SwitchFn, std::size_t... Is, typename... PacketRules>
constexpr auto variant_impl(SwitchFn&& fn,
                            std::integer_sequence<std::size_t, Is...>,
                            PacketRules&&... rules) {
    return ser2::switch_(
        std::forward<SwitchFn>(fn),
        variant_rule<Is>(ser2::rule(std::forward<PacketRules>(rules)))...);
}
template <typename SwitchFn, typename... PacketRules>
constexpr auto variant(SwitchFn&& fn, PacketRules&&... rules) {
    return variant_impl(
        overloaded{
            std::forward<SwitchFn>(fn),
            [](const auto& target, const auto&) { return target.index(); }},
        std::make_integer_sequence<std::size_t, sizeof...(rules)>{},
        std::forward<PacketRules>(rules)...);
}
}  // namespace chx::sql::mysql::detail::packets