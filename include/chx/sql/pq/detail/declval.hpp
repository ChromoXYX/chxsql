#pragma once

namespace chx::sql::pq::detail {
template <typename T> struct declval_wrapper {
    const T t;
};
template <typename T>
const extern declval_wrapper<T>
    CHX_SQL_PQ_DETAIL_DECLVAL_YOU_SHOULD_NEVER_DEFINE_THIS;

template <typename T> constexpr const T& declval() {
    return CHX_SQL_PQ_DETAIL_DECLVAL_YOU_SHOULD_NEVER_DEFINE_THIS<T>.t;
}
}  // namespace chx::sql::pq::detail
