#pragma once

#include <algorithm>
#include <cassert>
#include <charconv>
#include <chx/ser2/rule.hpp>
#include <chx/ser2/default_getter.hpp>
#include <chx/ser2/default_require.hpp>
#include <chx/ser2/ignore.hpp>
#include <chx/ser2/bind.hpp>
#include <cstdint>
#include <string>
#include <stdexcept>

#include <sstream>
#include <iomanip>

namespace chx::sql::mysql::detail::rules {
template <typename T> std::string replaceNonVcharWithEscape(const T& input) {
    if constexpr (std::is_same_v<std::decay_t<T>, ser2::ignore_t>) {
        return "Ignore";
    } else {
        std::ostringstream oss;
        for (unsigned char c : input) {
            if (!isprint(c) || c >= 128) {
                oss << '%' << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(c);
            } else {
                oss << c;
            }
        }
        return oss.str();
    }
}
template <typename T> inline std::string octToHex(T t) {
    char buf[20] = {};
    std::to_chars(buf, buf + 20, t, 16);
    return buf;
}

template <std::size_t N> struct fixed_length_integer {
    template <typename Self, typename UnsignedInteger, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, UnsignedInteger&& v, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) noexcept(true) {
        if (std::distance(begin, end) < N) {
            return ser2::ParseResult::Incomplete;
        }
        std::uint64_t nv = 0;
        std::copy(begin, begin + N, (unsigned char*)&nv);
        self.setter()(self, ctx, std::forward<UnsignedInteger>(v), nv);
        begin += N;
        return ser2::ParseResult::Ok;
    }

    template <typename Self, typename UnsignedInteger, typename Context>
    std::string to_string(const Self&, const UnsignedInteger& v,
                          const Context& ctx) const {
        return "[FixedInt " + octToHex(v) + "]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, const Target&,
                                         const Context&) const noexcept(true) {
        return N;
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    RandomAccessIterator generate(const Self&, const Target& target,
                                  const Context&, RandomAccessIterator iter,
                                  const RandomAccessIterator& end) const {
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        return std::copy_n((unsigned char*)&target, N, iter);
    }
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};
struct length_encoded_integer {
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, Target&& ov, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) noexcept(true) {
        static_assert(sizeof(std::decay_t<Target>) >= 8);
        std::ptrdiff_t l = std::distance(begin, end);
        if (l < 1) {
            return ser2::ParseResult::Incomplete;
        }
        std::uint64_t nv = 0;
        std::uint8_t head = *(begin++);
        if (head < 251) {
            nv = head;
        } else if (head == 0xfc) {
            if (l >= 3) {
                std::copy_n(begin, 2, (unsigned char*)&nv);
                begin += 2;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else if (head == 0xfd) {
            if (l >= 4) {
                std::copy_n(begin, 3, (unsigned char*)&nv);
                begin += 3;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else if (head == 0xfe) {
            if (l >= 9) {
                std::copy_n(begin, 8, (unsigned char*)&nv);
                begin += 8;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else {
            return ser2::ParseResult::Malformed;
        }
        self.setter()(self, ctx, std::forward<Target>(ov), nv);
        return ser2::ParseResult::Ok;
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, const Target& v,
                          const Context& ctx) const {
        static_assert(sizeof(std::decay_t<Target>) >= 8);
        return "[lec " + octToHex(v) + "]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, const Target& target,
                                         const Context&) const noexcept(true) {
        static_assert(sizeof(std::decay_t<Target>) >= 8);
        if (target < 251) {
            return 1;
        } else if (target < (2 << 16)) {
            return 3;
        } else if (target < (2 << 24)) {
            return 4;
        } else {
            return 9;
        }
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    RandomAccessIterator generate(const Self&, const Target& target,
                                  const Context&, RandomAccessIterator iter,
                                  const RandomAccessIterator& end) const {
        static_assert(sizeof(std::decay_t<Target>) >= 8);
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        if (target < 251) {
            std::copy_n((unsigned char*)&target, 1, iter);
            ++iter;
        } else if (target < (2 << 16)) {
            *(iter++) = 0xfc;
            std::copy_n((unsigned char*)&target, 2, iter);
            iter += 2;
        } else if (target < (2 << 24)) {
            *(iter++) = 0xfd;
            std::copy_n((unsigned char*)&target, 3, iter);
            iter += 3;
        } else {
            *(iter++) = 0xfe;
            std::copy_n((unsigned char*)&target, 8, iter);
            iter += 8;
        }
        return iter;
    }
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};

template <std::size_t N> struct fixed_length_string {
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, Target&& target, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::ptrdiff_t l = std::distance(begin, end);
        if (l >= N) {
            self.setter()(self, ctx, std::forward<Target>(target), begin,
                          begin + N);
            begin += N;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, const Target& target,
                          const Context& ctx) const {
        return "[Fixed " + replaceNonVcharWithEscape(target) + "]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, const Target&,
                                         const Context&) const noexcept(true) {
        return N;
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    RandomAccessIterator generate(const Self&, const Target& target,
                                  const Context&, RandomAccessIterator iter,
                                  const RandomAccessIterator& end) const {
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        std::copy_n(std::data(target), std::min(std::size(target), N), iter);
        return iter + N;
    }
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};
struct null_terminated_string {
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, Target&& target, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::ptrdiff_t l = std::distance(begin, end);
        std::size_t str_len = 0;
        while (str_len < l && *(begin + str_len) != '\0') {
            ++str_len;
        }
        if (str_len < l) {
            self.setter()(self, ctx, std::forward<Target>(target), begin,
                          begin + str_len);
            begin += str_len + 1;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, const Target& target,
                          const Context& ctx) const {
        return "[Null " + replaceNonVcharWithEscape(target) + "]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, const Target& target,
                                         const Context&) const noexcept(true) {
        return std::size(target) + 1;
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    RandomAccessIterator generate(const Self&, const Target& target,
                                  const Context&, RandomAccessIterator iter,
                                  const RandomAccessIterator& end) const {
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        std::copy_n(std::data(target), std::size(target), iter);
        iter += std::size(target);
        *(iter++) = 0;
        return iter;
    }
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};
template <typename TargetGetter, typename LengthGetter>
struct variable_length_string : protected TargetGetter, protected LengthGetter {
    constexpr variable_length_string() = default;
    constexpr variable_length_string(const variable_length_string&) = default;
    constexpr variable_length_string(variable_length_string&&) = default;
    constexpr variable_length_string(TargetGetter&& target_getter,
                                     LengthGetter&& length_getter)
        : TargetGetter(std::move(target_getter)),
          LengthGetter(std::move(length_getter)) {}

    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, Target&& target, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::ptrdiff_t l = std::distance(begin, end);
        ssize_t target_len =
            LengthGetter::operator()(std::forward<Target>(target), ctx);
        if (target_len < 0) {
            return ser2::ParseResult::Malformed;
        }
        if (l >= target_len) {
            self.setter()(
                self, ctx,
                TargetGetter::operator()(std::forward<Target>(target), ctx),
                begin, begin + target_len);
            begin += target_len;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, const Target& target,
                          const Context& ctx) const {
        return "[VarStr " +
               replaceNonVcharWithEscape(
                   TargetGetter::operator()(target, ctx)) +
               "]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, const Target& target,
                                         const Context& ctx) const
        noexcept(true) {
        ssize_t length = LengthGetter::operator()(target, ctx);
        assert(length >= 0);
        return length;
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    RandomAccessIterator generate(const Self&, const Target& target,
                                  const Context& ctx, RandomAccessIterator iter,
                                  const RandomAccessIterator& end) const {
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        auto& t = TargetGetter::operator()(target, ctx);
        ssize_t length = LengthGetter::operator()(target, ctx);
        assert(length >= 0);
        std::copy_n(std::begin(t),
                    std::min(static_cast<size_t>(length), std::size(t)), iter);
        return iter + length;
    }
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};
struct rest_of_packet_string {
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, Target&& target, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        self.setter()(self, ctx, std::forward<Target>(target), begin, end);
        begin = end;
        return ser2::ParseResult::Ok;
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, const Target& target,
                          const Context& ctx) const {
        return "[RestOfPacket " + replaceNonVcharWithEscape(target) + "]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, const Target& target,
                                         const Context& ctx) const {
        return std::size(target);
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    constexpr RandomAccessIterator
    generate(const Self&, const Target& target, const Context&,
             RandomAccessIterator iter, const RandomAccessIterator& end) const {
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        std::copy(std::begin(target), std::end(target), iter);
        return iter + std::size(target);
    }
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};
using plain_string = rest_of_packet_string;

struct length_encoded_string {
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    ser2::ParseResult parse(Self& self, Target&& target, Context& ctx,
                            RandomAccessIterator& begin,
                            const RandomAccessIterator& end) {
        std::size_t length = 0;
        auto subrule = ser2::rule(ser2::bind(length_encoded_integer{}));
        ser2::ParseResult r = subrule.parse(length, begin, end);
        if (r == ser2::ParseResult::Ok) {
            std::ptrdiff_t l = std::distance(begin, end);
            if (l >= length) {
                self.setter()(self, ctx, std::forward<Target>(target), begin,
                              begin + length);
                begin += length;
                return r;
            } else {
                return ser2::ParseResult::Incomplete;
            }
        } else {
            return r;
        }
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, const Target& target,
                          const Context& ctx) const {
        return "[lec " + replaceNonVcharWithEscape(target) + "]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, const Target& target,
                                         const Context& ctx) const
        noexcept(true) {
        std::size_t str_len = std::size(target);
        ser2::rule r(
            ser2::bind(length_encoded_integer{}, ser2::default_getter{}));
        return r.calculate_size(str_len) + str_len;
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    RandomAccessIterator generate(const Self&, const Target& target,
                                  const Context& ctx, RandomAccessIterator iter,
                                  const RandomAccessIterator& end) const {
        ser2::rule r(ser2::bind(length_encoded_integer{},
                                [](const auto& target, auto&) {
                                    return std::size(target);
                                }),
                     ser2::bind(plain_string{}, ser2::default_getter{}));
        return r.generate(target, iter, end);
    }
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};

template <std::size_t N> struct reserved {
    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }

    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    constexpr ser2::ParseResult
    parse(Self&, Target&&, Context&, RandomAccessIterator& begin,
          const RandomAccessIterator& end) noexcept(true) {
        if (std::distance(begin, end) >= N) {
            begin += N;
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Incomplete;
        }
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, Target&&, const Context&) const {
        return "[Reserved]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, Target&&,
                                         const Context&) const {
        return N;
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    constexpr RandomAccessIterator
    generate(const Self&, Target&&, const Context&, RandomAccessIterator iter,
             const RandomAccessIterator& end) const {
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        return iter + N;
    }
};

template <typename T> struct exactly : protected T {
    constexpr exactly() = default;
    constexpr exactly(const exactly&) = default;
    constexpr exactly(exactly&&) = default;
    constexpr exactly(T&& t) : T(std::move(t)) {}

    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    constexpr ser2::ParseResult
    parse(Self&, Target&&, Context&, RandomAccessIterator& begin,
          const RandomAccessIterator& end) noexcept(true) {
        const auto& t = T::operator()();
        if (std::size(t) <= std::distance(begin, end) &&
            std::equal(std::begin(t), std::end(t), begin,
                       begin + std::size(t))) {
            begin += std::size(t);
            return ser2::ParseResult::Ok;
        } else {
            return ser2::ParseResult::Malformed;
        }
    }

    template <typename Self, typename Target, typename Context>
    std::string to_string(const Self&, Target&&, const Context&) const {
        return "[Exactly]";
    }

    template <typename Self, typename Target, typename Context>
    constexpr std::size_t calculate_size(const Self&, Target&&,
                                         const Context&) const {
        return std::size(T::operator()());
    }
    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    constexpr RandomAccessIterator
    generate(const Self&, Target&&, const Context&, RandomAccessIterator iter,
             const RandomAccessIterator& end) const {
        if (iter >= end) {
            throw std::out_of_range{"iter >= end in rule.generate"};
        }
        const auto& t = T::operator()();
        return std::copy(std::begin(t), std::end(t), iter);
    }

    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};

template <typename SubRule, typename RepeatN>
struct repeat : protected SubRule, protected RepeatN {
    constexpr repeat() = default;
    constexpr repeat(const repeat&) = default;
    constexpr repeat(repeat&&) = default;
    constexpr repeat(SubRule&& subrule, RepeatN&& repeat_n)
        : SubRule(std::move(subrule)), RepeatN(std::move(repeat_n)) {}

    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    constexpr ser2::ParseResult parse(Self& self, Target&& target, Context& ctx,
                                      RandomAccessIterator& begin,
                                      const RandomAccessIterator& end) {
        decltype(auto) t = SubRule::getter()(std::forward<Target>(target), ctx);
        using value_type = typename std::decay_t<decltype(t)>::value_type;
        const std::size_t n = static_cast<RepeatN&>(*this)(
            std::forward<Target>(target), ctx, begin, end);
        for (std::size_t i = 0; i < n; ++i) {
            value_type v;
            ser2::ParseResult r = SubRule::parse(static_cast<SubRule&>(*this),
                                                 v, ctx, begin, end);
            if (r != ser2::ParseResult::Ok) {
                return r;
            }
            std::forward<decltype(t)>(t).push_back(std::move(v));
        }
        return ser2::ParseResult::Ok;
    }

    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }
};
}  // namespace chx::sql::mysql::detail::rules