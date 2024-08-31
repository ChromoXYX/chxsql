#pragma once

#include "./result.hpp"

namespace chx::sql::postgresql::detail {
template <typename Iterator>
constexpr ParseResult make_success(const Iterator& begin,
                                   const Iterator& end) noexcept(true) {
    if (begin == end) {
        return ParseSuccess;
    } else {
        return ParseMalformedTrailingData;
    }
}
}  // namespace chx::sql::postgresql::detail