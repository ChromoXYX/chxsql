#pragma once

#include <cstdint>
#pragma once

#include <vector>
#include <string>
#include <optional>
#include <algorithm>

namespace chx::sql::postgresql {
namespace detail {
template <typename Tag> struct visitor;
}

class result_set {
    template <typename Tag> friend struct detail::visitor;

  public:
    struct row_description_type {
        std::string field_name;
        std::uint32_t table_id = 0;
        std::uint16_t column_attribute_number = 0;
        std::uint32_t field_data_type_id = 0;
        std::int16_t data_type_size = 0;
        std::uint32_t type_modifier = 0;
        std::uint16_t format_code = 0;
    };

  private:
    std::vector<row_description_type> __M_def;

    using result_container_type =
        std::vector<std::vector<std::optional<std::vector<unsigned char>>>>;
    result_container_type __M_result;

  public:
    using row_type = std::vector<std::optional<std::vector<unsigned char>>>;
    using iterator = typename result_container_type::iterator;
    using const_iterator = typename result_container_type::const_iterator;

    constexpr std::vector<row_description_type>&
    row_description() noexcept(true) {
        return __M_def;
    }
    constexpr const std::vector<row_description_type>& row_description() const
        noexcept(true) {
        return __M_def;
    }

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
                            [&](const row_description_type& a) {
                                return a.field_name == std::forward<T>(t);
                            }) -
               __M_def.begin();
    }
};
}  // namespace chx::sql::postgresql