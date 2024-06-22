#pragma once

#include <charconv>
#include <chx/ser2/rule.hpp>
#include <chx/ser2/default_getter.hpp>
#include <cstdint>
#include <cstring>
#include <string>

#include <sstream>
#include <iomanip>

namespace chx::sql::mysql::detail::rules {
inline std::string replaceNonVcharWithEscape(const std::string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        // 如果字符是不可见字符或者不是ASCII字符
        if (!isprint(c) || c >= 128) {
            oss << '%' << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(c);
        } else {
            oss << c;
        }
    }
    return oss.str();
}
template <typename T> inline std::string octToHex(T t) {
    char buf[20] = {};
    std::to_chars(buf, buf + 20, t, 16);
    return buf;
}

template <std::size_t N> struct fixed_length_integer {
    template <typename Self, typename UnsignedInteger, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self&, UnsignedInteger& v, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) noexcept(true) {
        static_assert(sizeof(UnsignedInteger) >= N);
        if (std::distance(begin, end) < N) {
            return ser2::ParseResult::Incomplete;
        }
        v = 0;
        std::memcpy(&v, &*begin, N);
        begin += N;
        return ser2::ParseResult::Ok;
    }

    template <typename Self, typename UnsignedInteger, typename Context>
    std::string to_string(Self&, UnsignedInteger& v, Context& ctx) {
        return "[FixedInt " + octToHex(v) + "]";
    }
};
struct length_encoded_integer {
    template <typename Self, typename Context, typename RandomAccessIterator>
    ser2::ParseResult parse(Self&, std::uint64_t& v, Context&,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) noexcept(true) {
        std::size_t l = std::distance(begin, end);
        if (l < 1) {
            return ser2::ParseResult::Incomplete;
        }
        v = 0;
        std::uint8_t head = *(begin++);
        if (head < 251) {
            v = head;
            return ser2::ParseResult::Ok;
        } else if (head == 0xfc) {
            if (l >= 3) {
                std::memcpy(&v, &*begin, 2);
                begin += 2;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else if (head == 0xfd) {
            if (l >= 4) {
                std::memcpy(&v, &*begin, 3);
                begin += 3;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else if (head == 0xfe) {
            if (l >= 9) {
                std::memcpy(&v, &*begin, 8);
                begin += 8;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else {
            return ser2::ParseResult::Malformed;
        }
        return ser2::ParseResult::Ok;
    }

    template <typename Self, typename Context>
    std::string to_string(Self&, std::uint64_t& v, Context& ctx) {
        return "[lec " + octToHex(v) + "]";
    }
};

template <std::size_t N> struct fixed_length_string {
    template <typename Self, typename Context, typename RandomAccessIterator>
    ser2::ParseResult parse(Self&, std::string& target, Context&,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::size_t l = std::distance(begin, end);
        if (l >= N) {
            target.assign(begin, begin + N);
            begin += N;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Context>
    std::string to_string(Self&, std::string& target, Context& ctx) {
        return "[Fixed " + replaceNonVcharWithEscape(target) + "]";
    }
};
struct null_terminated_string {
    template <typename Self, typename Context, typename RandomAccessIterator>
    ser2::ParseResult parse(Self&, std::string& target, Context&,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::size_t l = std::distance(begin, end);
        std::size_t str_len = ::strnlen((const char*)&*(begin), l);
        if (str_len < l) {
            target.assign(begin, begin + str_len);
            begin += str_len + 1;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Context>
    std::string to_string(Self&, std::string& target, Context& ctx) {
        return "[Null " + replaceNonVcharWithEscape(target) + "]";
    }
};
template <typename TargetGetter, typename LengthGetter>
struct variable_length_string : protected TargetGetter, protected LengthGetter {
    variable_length_string() = default;
    variable_length_string(const variable_length_string&) = default;
    variable_length_string(variable_length_string&&) = default;
    variable_length_string(TargetGetter&& target_getter,
                           LengthGetter&& length_getter)
        : TargetGetter(std::move(target_getter)),
          LengthGetter(std::move(length_getter)) {}

    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, Target& target, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::size_t l = std::distance(begin, end);
        std::string& t = TargetGetter::operator()(target, ctx);
        ssize_t target_len = LengthGetter::operator()(target, ctx);
        if (target_len < 0) {
            return ser2::ParseResult::Malformed;
        }
        if (l >= target_len) {
            t.assign(begin, begin + target_len);
            begin += target_len;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(Self&, Target& target, Context& ctx) {
        return "[VarStr " +
               replaceNonVcharWithEscape(
                   TargetGetter::operator()(target, ctx)) +
               "]";
    }
};
struct length_encoded_string {
    template <typename Self, typename Context, typename RandomAccessIterator>
    ser2::ParseResult parse(Self&, std::string& target, Context&,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::size_t length = 0;
        ser2::rule<std::size_t, length_encoded_integer> subrule;
        ser2::ParseResult r = subrule.parse(length, begin, end);
        if (r == ser2::ParseResult::Ok) {
            std::size_t l = std::distance(begin, end);
            if (l >= length) {
                target.assign(begin, begin + length);
                begin += length;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else {
            return r;
        }
    }

    template <typename Self, typename Context>
    std::string to_string(Self&, std::string& target, Context& ctx) {
        return "[lec " + replaceNonVcharWithEscape(target) + "]";
    }
};
struct rest_of_packet_string {
    template <typename Self, typename Context, typename RandomAccessIterator>
    ser2::ParseResult parse(Self&, std::string& target, Context&,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        target.assign(begin, end);
        begin = end;
        return ser2::ParseResult::Ok;
    }

    template <typename Self, typename Context>
    std::string to_string(Self&, std::string& target, Context& ctx) {
        return "[RestOfPacket " + replaceNonVcharWithEscape(target) + "]";
    }
};

template <std::size_t N> struct reserved {
    constexpr auto getter() const noexcept(true) {
        return [](auto&&...) { return 0; };
    }

    template <typename Self, typename Context, typename RandomAccessIterator>
    constexpr ser2::ParseResult
    parse(Self&, int, Context&, RandomAccessIterator& begin,
          const RandomAccessIterator& end) noexcept(true) {
        if (std::distance(begin, end) >= N) {
            begin += N;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Context>
    std::string to_string(Self&, int, Context& ctx) {
        return "[Reserved]";
    }
};
}  // namespace chx::sql::mysql::detail::rules