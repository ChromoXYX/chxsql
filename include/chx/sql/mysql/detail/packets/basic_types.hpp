#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <iterator>
#include "../result.hpp"

namespace chx::sql::mysql::detail::packets2 {
template <std::size_t N> struct fixed_length_integer {
    static_assert(N <= 8);

    constexpr PayloadResult
    operator()(const unsigned char*& begin,
               const unsigned char* end) noexcept(true) {
        const std::size_t consumed =
            std::min(static_cast<std::size_t>(std::distance(begin, end)),
                     N - __M_digits);
        for (std::size_t i = 0; i < consumed; ++i) {
            __M_result += *(begin++) << (8 * __M_digits++);
        }
        if (__M_digits == N) {
            return PayloadSuccess;
        } else {
            return PayloadNeedMore;
        }
    }

    constexpr std::size_t value() const noexcept(true) { return __M_result; }

  private:
    std::size_t __M_result = 0;
    std::uint8_t __M_digits = 0;
};

struct length_encoded_integer {
    constexpr PayloadResult
    operator()(const unsigned char*& begin,
               const unsigned char* end) noexcept(true) {
        if (__M_target == 0) {
            if (begin == end) {
                return PayloadNeedMore;
            }
            const std::uint8_t first_byte = *(begin++);
            if (first_byte < 251) {
                __M_target = 0;
                __M_result = first_byte;
                return PayloadSuccess;
            } else if (first_byte == 0xfc) {
                __M_target = 2;
                return operator()(begin, end);
            } else if (first_byte == 0xfd) {
                __M_target = 3;
                return operator()(begin, end);
            } else if (first_byte == 0xfe) {
                __M_target = 8;
                return operator()(begin, end);
            } else {
                return PayloadMalformed;
            }
        } else {
            const std::size_t len = std::distance(begin, end);
            const std::size_t consumed = std::min(
                static_cast<std::size_t>(__M_target - __M_digits), len);
            for (std::size_t i = 0; i < consumed; ++i) {
                __M_result += *(begin++) << (8 * __M_digits++);
            }
            if (__M_digits == __M_target) {
                return PayloadSuccess;
            } else {
                return PayloadNeedMore;
            }
        }
    }

    constexpr std::size_t value() const noexcept(true) { return __M_result; }

  private:
    std::uint8_t __M_digits = 0;
    std::uint8_t __M_target = 0;
    std::size_t __M_result = 0;
};

template <std::size_t N, typename Container = std::string>
struct fixed_length_string {
    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end) {
        const std::size_t len = std::distance(begin, end);
        const std::size_t consumed = std::min(len, N - __M_result.size());
        __M_result.insert(__M_result.end(), begin, begin + consumed);
        begin += consumed;
        if (__M_result.size() == N) {
            return PayloadSuccess;
        } else {
            return PayloadNeedMore;
        }
    }

    Container value() noexcept(true) { return std::move(__M_result); }

  private:
    Container __M_result;
};

template <typename Container = std::string> struct null_terminated_string {
    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end) {
        const std::size_t len = std::distance(begin, end);
        const std::size_t consumed = ::strnlen((const char*)begin, len);
        __M_result.insert(__M_result.end(), begin, begin + consumed);
        begin += consumed;
        if (consumed != len) {
            ++begin;
            return PayloadSuccess;
        } else {
            return PayloadNeedMore;
        }
    }

    Container value() noexcept(true) { return std::move(__M_result); }

  private:
    Container __M_result;
};

template <typename Container = std::string> struct variable_length_string {
    constexpr variable_length_string(std::size_t target_length)
        : __M_target_length(target_length) {}

    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end) {
        const std::size_t len = std::distance(begin, end);
        const std::size_t consumed =
            std::min(len, __M_target_length - __M_result.size());
        __M_result.insert(__M_result.end(), begin, begin + consumed);
        begin += consumed;
        if (std::size(__M_result) == __M_target_length) {
            return PayloadSuccess;
        } else {
            return PayloadNeedMore;
        }
    }

    Container value() noexcept(true) { return std::move(__M_result); }

  private:
    std::size_t __M_target_length = 0;
    Container __M_result;
};

template <typename Container = std::string> struct length_encoded_string {
    PayloadResult operator()(const unsigned char*& begin,
                             const unsigned char* end) {
        if (!__M_str_parser) {
            PayloadResult result = __M_len_parser(begin, end);
            if (result == PayloadSuccess) {
                __M_str_parser.emplace(__M_len_parser.value());
                return operator()(begin, end);
            } else {
                return result;
            }
        } else {
            return (*__M_str_parser)(begin, end);
        }
    }

    Container value() { return __M_str_parser->value(); }

  private:
    length_encoded_integer __M_len_parser = {};
    std::optional<variable_length_string<Container>> __M_str_parser = {};
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
}  // namespace chx::sql::mysql::detail::packets2