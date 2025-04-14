#pragma once

#include "./result.hpp"
#include <chx/net/async_token.hpp>
#include <chx/net/detail/type_identity.hpp>

namespace chx::sql::postgresql {
namespace detail {
template <typename GeneratedToken> struct last_base_callable : GeneratedToken {
    template <typename G>
    last_base_callable(G&& g) : GeneratedToken(std::forward<G>(g)) {}

    template <typename... Ts> constexpr decltype(auto) operator()(Ts&&... ts) {
        std::tuple<Ts&...> args(ts...);
        const std::error_code& ec = std::get<0>(args);
        result_sets& r = std::get<result_sets&>(args);
        return GeneratedToken::operator()(nullptr)(
            ec, !r.empty() ? std::move(r.back()) : std::nullopt);
    }
};
template <typename G>
last_base_callable(G&&) -> last_base_callable<std::remove_reference_t<G>>;

template <typename FinalFunctor, typename CallableObj>
struct last_3 : CallableObj {
    FinalFunctor final_functor;
    template <typename F, typename C>
    last_3(F&& f, C&& c)
        : final_functor(std::forward<F>(f)), CallableObj(std::forward<C>(c)) {}

    decltype(auto) operator()(net::task_decl* self) {
        return final_functor(static_cast<CallableObj&>(*this), self);
    }
};
template <typename F, typename C>
last_3(F&&, C&&)
    -> last_3<std::remove_reference_t<F>, std::remove_reference_t<C>>;

template <typename BindCompletionToken> struct last_2 {
    using attribute_type = net::attribute<net::async_token>;

    BindCompletionToken ct;

    template <typename T>
    constexpr last_2(T&& t) noexcept(true) : ct(std::forward<T>(t)) {}

    template <typename FinalFunctor>
    constexpr decltype(auto) generate_token(net::task_decl* task,
                                            FinalFunctor&& final_functor) {
        return last_3(std::forward<FinalFunctor>(final_functor),
                      last_base_callable(net::detail::async_token_generate(
                          task, net::detail::fake_final_functor(), ct)));
    }

    template <typename TypeIdentity> decltype(auto) get_init(TypeIdentity ti) {
        return net::detail::async_token_init(ti, ct);
    }
};
template <typename C> last_2(C&& c) -> last_2<std::remove_reference_t<C>>;
}  // namespace detail

template <typename CompletionToken> struct last {
    using attribute_type = net::attribute<net::async_token>;
    CompletionToken completion_token;

    template <typename T>
    constexpr last(T&& t) noexcept(true)
        : completion_token(std::forward<T>(t)) {}

    template <typename... Sig> constexpr decltype(auto) bind() {
        return detail::last_2(
            net::detail::async_token_bind<const std::error_code&,
                                          std::optional<result_set>>(
                completion_token));
    }
};
template <typename CompletionToken>
last(CompletionToken&&) -> last<std::remove_reference_t<CompletionToken>>;
}  // namespace chx::sql::postgresql
