#pragma once

#include <libpq-fe.h>
#include <memory>
#include "./cast.hpp"

namespace chx::net::detail {
template <typename> struct async_operation;
}

namespace chx::sql::pq {
struct field;
struct row;

class result_set {
    template <typename T> friend struct chx::net::detail::async_operation;

  protected:
    std::shared_ptr<PGresult> __M_ptr;
    PGresult* r() const noexcept(true) { return __M_ptr.get(); }

    result_set(PGresult* p) : __M_ptr(p, [](PGresult* r) { PQclear(r); }) {}

  public:
    using size_type = int;
    using difference_type = std::ptrdiff_t;
    using value_type = row;

    class iterator;
    using const_iterator = iterator;

    const_iterator cbegin() const;
    const_iterator cend() const;
    iterator begin();
    iterator end();

    int column_n() const noexcept(true) { return PQnfields(r()); }
    int tuple_n() const noexcept(true) { return PQntuples(r()); }
    int size() const noexcept(true) { return tuple_n(); }

    const char* column_name(int idx) const noexcept(true) {
        return PQfname(r(), idx);
    }
    int column_number(std::string_view name) const noexcept(true) {
        return PQfnumber(r(), name.data());
    }
    Oid column_oid(int idx) const noexcept(true) { return PQftable(r(), idx); }
    int column_number_within_table(int idx) const noexcept(true) {
        return PQftablecol(r(), idx);
    }
    int column_format(int idx) const noexcept(true) {
        return PQfformat(r(), idx);
    }
    Oid column_type(int idx) const noexcept(true) { return PQftype(r(), idx); }
    int column_modifier(int idx) const noexcept(true) {
        return PQfmod(r(), idx);
    }
    int column_size(int idx) const noexcept(true) { return PQfsize(r(), idx); }
    int binary_tuples() const noexcept(true) { return PQbinaryTuples(r()); }
    const char* get_value(int row, int col) const noexcept(true) {
        return PQgetvalue(r(), row, col);
    }
    bool get_is_null(int row, int col) const noexcept(true) {
        return PQgetisnull(r(), row, col) != 0;
    }
    int get_length(int row, int col) const noexcept(true) {
        return PQgetlength(r(), row, col);
    }
    int num_params() const noexcept(true) { return PQnparams(r()); }
    Oid param_type(int idx) const noexcept(true) {
        return PQparamtype(r(), idx);
    }

    ExecStatusType result_status() const noexcept(true) {
        return PQresultStatus(r());
    }
    std::string error_message() const { return PQresultErrorMessage(r()); }

    void print(FILE* out, const PQprintOpt& opt) const noexcept(true) {
        PQprint(out, r(), &opt);
    }
};

class row : protected result_set {
    friend class result_set;

  protected:
    int __M_irow = 0;

    row(const result_set& parent, int ir)
        : pq::result_set(parent), __M_irow(ir) {}

  public:
    using size_type = int;
    using difference_type = std::ptrdiff_t;
    using value_type = field;

    class iterator;
    using const_iterator = iterator;
    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;

    constexpr pq::result_set& result_set() noexcept(true) { return *this; }
    constexpr const pq::result_set& result_set() const noexcept(true) {
        return *this;
    }

    constexpr int tuple_i() const { return __M_irow; }
    constexpr int row_i() const { return tuple_i(); }

    const char* get_value(int col) const noexcept(true) {
        return result_set::get_value(tuple_i(), col);
    }
    bool get_is_null(int col) const noexcept(true) {
        return result_set::get_is_null(tuple_i(), col) != 0;
    }
    int get_length(int col) const noexcept(true) {
        return result_set::get_length(tuple_i(), col);
    }
};

class field : protected row {
    friend class row;

  protected:
    int __M_icol = 0;

    field(const row& r, int col) : pq::row(r), __M_icol(col) {}

  public:
    constexpr pq::row& row() noexcept(true) { return *this; }
    constexpr const pq::row& row() const noexcept(true) { return *this; }

    constexpr int column_i() const noexcept(true) { return __M_icol; }
    constexpr int field_i() const noexcept(true) { return column_i(); }

    const char* get_value() const noexcept(true) {
        return row::get_value(__M_icol);
    }
    bool get_is_null() const noexcept(true) {
        return row::get_is_null(__M_icol);
    }
    int get_length() const noexcept(true) { return row::get_length(__M_icol); }

    template <typename T> T cast_to() const {
        const char* p = get_value();
        int l = get_length();
        return cast_selector<T>().from(p, p + l);
    }
};

class result_set::iterator {
  private:
    const result_set& __M_result_set;
    size_type __M_index;

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = row;
    using difference_type = std::ptrdiff_t;

    iterator(const result_set& rs, size_type index)
        : __M_result_set(rs), __M_index(index) {}

    iterator& operator++() {
        ++__M_index;
        return *this;
    }
    iterator operator++(int) {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    iterator& operator--() {
        --__M_index;
        return *this;
    }
    iterator operator--(int) {
        iterator tmp = *this;
        --(*this);
        return tmp;
    }
    iterator& operator+=(difference_type n) {
        __M_index += n;
        return *this;
    }
    iterator operator+(difference_type n) const {
        return iterator(__M_result_set, __M_index + n);
    }
    iterator& operator-=(difference_type n) {
        __M_index -= n;
        return *this;
    }
    iterator operator-(difference_type n) const {
        return iterator(__M_result_set, __M_index - n);
    }
    difference_type operator-(const iterator& other) const {
        return __M_index - other.__M_index;
    }

    bool operator==(const iterator& other) const {
        return __M_index == other.__M_index;
    }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    bool operator<(const iterator& other) const {
        return __M_index < other.__M_index;
    }
    bool operator>(const iterator& other) const {
        return __M_index > other.__M_index;
    }
    bool operator<=(const iterator& other) const { return !(*this > other); }
    bool operator>=(const iterator& other) const { return !(*this < other); }

    row operator*() const { return row(__M_result_set, __M_index); }
};

inline auto result_set::cbegin() const -> const_iterator {
    return const_iterator(*this, 0);
}
inline auto result_set::cend() const -> const_iterator {
    return const_iterator(*this, tuple_n());
}
inline auto result_set::begin() -> iterator { return cbegin(); }
inline auto result_set::end() -> iterator { return cend(); }

class row::iterator {
  private:
    const row& __M_row;
    size_type __M_index;

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = field;
    using difference_type = std::ptrdiff_t;

    iterator(const row& r, size_type index) : __M_row(r), __M_index(index) {}

    iterator& operator++() {
        ++__M_index;
        return *this;
    }
    iterator operator++(int) {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    iterator& operator--() {
        --__M_index;
        return *this;
    }
    iterator operator--(int) {
        iterator tmp = *this;
        --(*this);
        return tmp;
    }
    iterator& operator+=(difference_type n) {
        __M_index += n;
        return *this;
    }
    iterator operator+(difference_type n) const {
        return iterator(__M_row, __M_index + n);
    }
    iterator& operator-=(difference_type n) {
        __M_index -= n;
        return *this;
    }
    iterator operator-(difference_type n) const {
        return iterator(__M_row, __M_index - n);
    }
    difference_type operator-(const iterator& other) const {
        return __M_index - other.__M_index;
    }

    bool operator==(const iterator& other) const {
        return __M_index == other.__M_index;
    }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    bool operator<(const iterator& other) const {
        return __M_index < other.__M_index;
    }
    bool operator>(const iterator& other) const {
        return __M_index > other.__M_index;
    }
    bool operator<=(const iterator& other) const { return !(*this > other); }
    bool operator>=(const iterator& other) const { return !(*this < other); }

    field operator*() const { return field(__M_row, __M_index); }
};

inline auto row::cbegin() const -> const_iterator {
    return const_iterator(*this, 0);
}
inline auto row::cend() const -> const_iterator {
    return const_iterator(*this, column_n());
}
inline auto row::begin() -> iterator { return cbegin(); }
inline auto row::end() -> iterator { return cend(); }
}  // namespace chx::sql::pq
