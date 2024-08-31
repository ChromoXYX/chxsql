#pragma once

#include <vector>
#include <string>
#include <optional>
#include <algorithm>

namespace chx::sql::mysql {
namespace detail {
template <typename Tag> struct visitor;
}

class result_set {
    template <typename Tag> friend struct detail::visitor;

    std::vector<std::string> __M_def;

    using result_container_type =
        std::vector<std::vector<std::optional<std::string>>>;
    result_container_type __M_result;

  public:
    using row_type = std::vector<std::optional<std::string>>;
    using iterator = typename result_container_type::iterator;
    using const_iterator = typename result_container_type::const_iterator;

    constexpr iterator begin() noexcept(true) { return __M_result.begin(); }
    constexpr iterator end() noexcept(true) { return __M_result.end(); }
    constexpr const_iterator begin() const noexcept(true) {
        return __M_result.begin();
    }
    constexpr const_iterator end() const noexcept(true) {
        return __M_result.end();
    }

    constexpr const row_type& operator[](std::size_t idx) const noexcept(true) {
        return __M_result[idx];
    }
    constexpr row_type& operator[](std::size_t idx) noexcept(true) {
        return __M_result[idx];
    }
    constexpr const row_type& at(std::size_t idx) const {
        return __M_result.at(idx);
    }
    constexpr row_type& at(std::size_t idx) { return __M_result.at(idx); }

    constexpr std::size_t size() const noexcept(true) {
        return __M_result.size();
    }
    constexpr bool empty() const noexcept(true) { return __M_result.empty(); }

    template <typename T>
    constexpr std::size_t index(T&& t) const noexcept(true) {
        return std::find_if(__M_def.begin(), __M_def.end(),
                            [&](const std::string& a) {
                                return a == std::forward<T>(t);
                            }) -
               __M_def.begin();
    }
};
}  // namespace chx::sql::mysql