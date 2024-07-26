#pragma once

#include <utility>

namespace chx::sql::mysql {
template <typename Target> struct data_setter {
    template <typename T, typename Iterator>
    constexpr void operator()(T&& t, Iterator begin, Iterator end) {
        std::forward<T>(t).assign(begin, end);
    }
};
}  // namespace chx::sql::mysql