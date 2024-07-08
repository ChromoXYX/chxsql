#pragma once

#include <algorithm>
#include <chx/ser2/parse_result.hpp>
#include <chx/ser2/struct_getter.hpp>
#include <chx/ser2/default_getter.hpp>
#include <cstdint>
#include <iterator>

namespace chx::sql::mysql::detail::packets {
template <typename Rule> struct packet : protected Rule {
    constexpr packet(std::uint8_t& next_sequence_id, Rule&& rule)
        : Rule(std::move(rule)), __M_next_sequence_id(next_sequence_id) {}

    template <typename Self, typename Target, typename Context,
              typename RandomAccessIterator>
    constexpr ser2::ParseResult parse(Self& self, Target&& target, Context& ctx,
                                      RandomAccessIterator& begin,
                                      const RandomAccessIterator& end) {
        const std::size_t len = std::distance(begin, end);
        std::uint32_t payload_length = 0;
        std::uint8_t sequence_id = 0;
        if (len < 4) {
            return ser2::ParseResult::Incomplete;
        }
        std::copy_n(begin, 3, (unsigned char*)&payload_length);
        begin += 3;
        std::copy_n(begin++, 1, &sequence_id);
        if (payload_length > std::distance(begin, end)) {
            return ser2::ParseResult::Incomplete;
        }
        if (__M_next_sequence_id++ != sequence_id) {
            return ser2::ParseResult::Malformed;
        }
        return Rule::parse(static_cast<Rule&>(*this),
                           Rule::getter()(std::forward<Target>(target), ctx),
                           ctx, begin, end);
    }

    constexpr ser2::default_getter getter() const noexcept(true) { return {}; }

  private:
    std::uint8_t& __M_next_sequence_id;
};
}  // namespace chx::sql::mysql::detail::packets