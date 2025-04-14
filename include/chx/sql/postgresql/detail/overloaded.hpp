#pragma once

namespace chx::sql::postgresql::detail {
template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}  // namespace chx::sql::postgresql::detail
