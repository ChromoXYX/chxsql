#pragma once

#include <string>

namespace chx::sql::pq {
namespace detail {
template <typename T> struct is_complete {
  private:
    template <typename R>
    static auto f(R*) -> std::integral_constant<bool, sizeof(R) == sizeof(R)>;
    static auto f(...) -> std::false_type;

  public:
    using type = decltype(f((T*)0));
    static constexpr bool value = type::value;
};
}  // namespace detail

template <typename T> struct cast;

template <> struct cast<std::string> {
    inline std::string to(std::string&& str) noexcept(true) {
        return std::move(str);
    }
    inline std::string to(const std::string& str) noexcept(true) { return str; }

    inline std::string from(const char* begin, const char* end) noexcept(true) {
        return {begin, end};
    }
};

template <typename T>
struct cast_selector : std::conditional_t<detail::is_complete<cast<T>>::value,
                                          cast<T>, cast<std::decay_t<T>>> {};
}  // namespace chx::sql::pq
