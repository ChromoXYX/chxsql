#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <string>
#include <algorithm>

namespace chx::sql::postgresql {
namespace detail {
template <typename Tag> struct visitor;
}

struct row_description {
    std::string field_name;
    std::uint32_t table_id = 0;
    std::uint16_t column_attribute_number = 0;
    std::uint32_t field_data_type_id = 0;
    std::int16_t data_type_size = 0;
    std::uint32_t type_modifier = 0;
    std::uint16_t format_code = 0;
};

class result_set;
class row;

class const_row_iterator {
    const result_set* __M_result;
    std::size_t __M_idx;

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = const row;
    using difference_type = std::ptrdiff_t;
    using pointer = const row*;
    using reference = const row;

    constexpr const_row_iterator() noexcept : __M_result(nullptr), __M_idx(0) {}
    constexpr const_row_iterator(const result_set& result,
                                 std::size_t idx) noexcept
        : __M_result(&result), __M_idx(idx) {}
    constexpr const row operator*() const;
    constexpr const_row_iterator& operator++() noexcept {
        ++__M_idx;
        return *this;
    }
    constexpr const_row_iterator operator++(int) noexcept {
        const_row_iterator tmp = *this;
        ++__M_idx;
        return tmp;
    }
    constexpr const_row_iterator& operator--() noexcept {
        --__M_idx;
        return *this;
    }
    constexpr const_row_iterator operator--(int) noexcept {
        const_row_iterator tmp = *this;
        --__M_idx;
        return tmp;
    }

    constexpr const_row_iterator& operator+=(difference_type n) noexcept {
        __M_idx += n;
        return *this;
    }
    constexpr const_row_iterator operator+(difference_type n) const noexcept {
        return const_row_iterator(*__M_result, __M_idx + n);
    }
    constexpr const_row_iterator& operator-=(difference_type n) noexcept {
        __M_idx -= n;
        return *this;
    }
    constexpr const_row_iterator operator-(difference_type n) const noexcept {
        return const_row_iterator(*__M_result, __M_idx - n);
    }
    constexpr difference_type
    operator-(const const_row_iterator& other) const noexcept {
        return __M_idx - other.__M_idx;
    }

    constexpr bool operator==(const const_row_iterator& other) const noexcept {
        return __M_result == other.__M_result && __M_idx == other.__M_idx;
    }
    constexpr bool operator!=(const const_row_iterator& other) const noexcept {
        return !(*this == other);
    }
    constexpr bool operator<(const const_row_iterator& other) const noexcept {
        return __M_idx < other.__M_idx;
    }
    constexpr bool operator>(const const_row_iterator& other) const noexcept {
        return other < *this;
    }
    constexpr bool operator<=(const const_row_iterator& other) const noexcept {
        return !(other < *this);
    }
    constexpr bool operator>=(const const_row_iterator& other) const noexcept {
        return !(*this < other);
    }
    constexpr std::size_t index() const noexcept { return __M_idx; }
};

class result_set {
    template <typename> friend struct detail::visitor;

    std::vector<row_description> __M_desc;
    std::vector<std::vector<std::optional<std::vector<unsigned char>>>>
        __M_data;

  public:
    using const_iterator = const_row_iterator;
    using iterator = const_iterator;

    constexpr const_iterator begin() const noexcept {
        return const_iterator(*this, 0);
    }
    constexpr const_iterator end() const noexcept {
        return const_iterator(*this, this->size());
    }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }

    constexpr std::size_t column_size() const noexcept(true) {
        return __M_desc.size();
    }
    constexpr std::size_t tuple_size() const noexcept(true) {
        return __M_data.size();
    }
    constexpr std::size_t size() const noexcept(true) { return tuple_size(); }

    constexpr const row_description& column(std::size_t idx) const
        noexcept(true) {
        return __M_desc[idx];
    }
    constexpr std::string_view column_name(std::size_t idx) const
        noexcept(true) {
        return __M_desc[idx].field_name;
    }
    constexpr std::size_t column_number(std::string_view name) const
        noexcept(true) {
        return std::find_if(__M_desc.begin(), __M_desc.end(),
                            [&](const row_description& r) {
                                return r.field_name == name;
                            }) -
               __M_desc.begin();
    }

    constexpr const std::optional<std::vector<unsigned char>>&
    value(std::size_t row, std::size_t col) const {
        return __M_data.at(row).at(col);
    }
    constexpr const std::optional<std::vector<unsigned char>>&
    get_value(std::size_t row, std::size_t col) const noexcept(true) {
        return __M_data[row][col];
    }
};

class row {
    friend result_set;
    result_set& __M_par;
    const std::size_t __M_i;

    constexpr row(result_set& r, std::size_t i) noexcept(true)
        : __M_par(r), __M_i(i) {}

  public:
    row(const row&) = default;

    constexpr result_set& parent() noexcept(true) { return __M_par; }
    constexpr const result_set& parent() const noexcept(true) {
        return __M_par;
    }

    constexpr const std::optional<std::vector<unsigned char>>&
    value(std::size_t col) const {
        return parent().value(__M_i, col);
    }
    constexpr const std::optional<std::vector<unsigned char>>&
    value(std::string_view col_name) const {
        return parent().value(__M_i, parent().column_number(col_name));
    }

    constexpr const std::optional<std::vector<unsigned char>>&
    get_value(std::size_t col) const noexcept(true) {
        return parent().get_value(__M_i, col);
    }
    constexpr const std::optional<std::vector<unsigned char>>&
    get_value(std::string_view col_name) const noexcept(true) {
        return parent().get_value(__M_i, parent().column_number(col_name));
    }
};

using result_sets = std::vector<std::optional<result_set>>;
}  // namespace chx::sql::postgresql
