#pragma once

#include <cstdint>

namespace chx::sql::mysql::detail::packets2 {
enum [[nodiscard]] PayloadResult : std::uint8_t {
    PayloadSuccess,
    PayloadPause,

    PayloadNeedMore,

    PayloadMalformed,
    PayloadMalformedHeader,
    PayloadTrailingData,
    PayloadInternalError
};
template <typename Iterator>
constexpr PayloadResult make_success(const Iterator& begin,
                                     const Iterator& end) noexcept(true) {
    return (begin == end) ? PayloadSuccess : PayloadTrailingData;
}
template <typename Iterator>
constexpr PayloadResult make_pause(const Iterator& begin,
                                   const Iterator& end) noexcept(true) {
    return (begin == end) ? PayloadPause : PayloadTrailingData;
}
enum [[nodiscard]] PacketResult : std::uint8_t {
    PacketPaused,
    PacketNeedMore,

    PacketIncomplete,
    PacketTrailingData,

    PacketMalformed,
    PacketInternalError
};
}  // namespace chx::sql::mysql::detail::packets2