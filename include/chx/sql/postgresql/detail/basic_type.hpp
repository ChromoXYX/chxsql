#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <vector>
#include <netinet/in.h>

#include "./result.hpp"

namespace chx::sql::postgresql::detail {
template <typename T> struct integer {
    ParseResult operator()(const unsigned char*& begin,
                           const unsigned char* end) noexcept(true) {
        const std::size_t consumed =
            std::min(static_cast<unsigned long>(std::distance(begin, end)),
                     sizeof(T) - __M_digits_consumed);
        unsigned char* dest =
            reinterpret_cast<unsigned char*>(&__M_result) + __M_digits_consumed;
        std::memcpy(dest, begin, consumed);
        __M_digits_consumed += consumed;
        begin += consumed;
        if (__M_digits_consumed == sizeof(T)) {
            return ParseSuccess;
        } else {
            return ParseNeedMore;
        }
    }

    T value() noexcept(true) {
        if constexpr (sizeof(T) == 1) {
            return __M_result;
        } else if constexpr (sizeof(T) == 2) {
            return ntohs(__M_result);
        } else if constexpr (sizeof(T) == 4) {
            return ntohl(__M_result);
        } else {
            static_assert(sizeof(T) == 8);
            return __builtin_bswap64(__M_result);
        }
    }

  private:
    T __M_result = 0;
    std::uint8_t __M_digits_consumed = 0;
};
template <typename T> struct integer_array {
    integer_array(std::size_t n) : __M_target(n) {
        __M_result.reserve(__M_target);
    }

    ParseResult operator()(const unsigned char*& begin,
                           const unsigned char* end) {
        for (; __M_result.size() < __M_target;) {
            ParseResult r = __M_parser(begin, end);
            if (r == ParseSuccess) {
                __M_result.push_back(__M_parser.value());
                __M_parser = {};
            } else {
                return r;
            }
        }
        return ParseSuccess;
    }

    std::vector<T> value() noexcept(true) { return std::move(__M_result); }

  private:
    std::vector<T> __M_result;
    const std::size_t __M_target;
    integer<T> __M_parser = {};
};
template <typename Container = std::string> struct string {
    ParseResult operator()(const unsigned char*& begin,
                           const unsigned char* end) {
        const std::size_t len = std::distance(begin, end);
        const std::size_t consumed = ::strnlen((const char*)begin, len);
        __M_result.insert(__M_result.end(), begin, begin + consumed);
        begin += consumed;
        if (consumed != len) {
            ++begin;
            return ParseSuccess;
        } else {
            return ParseNeedMore;
        }
    }

    Container value() noexcept(true) { return std::move(__M_result); }

  private:
    Container __M_result;
};
template <typename Container = std::vector<unsigned char>> struct byte {
    constexpr byte(std::size_t target_length)
        : __M_target_length(target_length) {}

    ParseResult operator()(const unsigned char*& begin,
                           const unsigned char* end) {
        const std::size_t len = std::distance(begin, end);
        const std::size_t consumed =
            std::min(len, __M_target_length - __M_result.size());
        __M_result.insert(__M_result.end(), begin, begin + consumed);
        begin += consumed;
        if (std::size(__M_result) == __M_target_length) {
            return ParseSuccess;
        } else {
            return ParseNeedMore;
        }
    }

    Container value() noexcept(true) { return std::move(__M_result); }

  private:
    std::size_t __M_target_length = 0;
    Container __M_result;
};

struct nop_container {
    std::size_t sz = 0;

    constexpr int end() noexcept(true) { return 0; }
    template <typename Iterator>
    constexpr void insert(int, const Iterator& begin,
                          const Iterator& end) noexcept(true) {
        sz += std::distance(begin, end);
    }

    constexpr std::size_t size() const noexcept(true) { return sz; }
};

template <typename T>
[[nodiscard]] constexpr unsigned char*
integer_to_network(T v, unsigned char* dest) noexcept(true) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 2);
    if constexpr (sizeof(T) == 4) {
        v = htonl(v);
        ::memcpy(dest, &v, 4);
        return dest + 4;
    } else {
        v = htons(v);
        ::memcpy(dest, &v, 2);
        return dest + 2;
    }
}
template <typename Iterator>
[[nodiscard]] constexpr unsigned char*
string_to_network(const Iterator& begin, const Iterator& end,
                  unsigned char* dest) noexcept(true) {
    dest = std::copy(begin, end, dest);
    *(dest++) = 0;
    return dest;
}
}  // namespace chx::sql::postgresql::detail