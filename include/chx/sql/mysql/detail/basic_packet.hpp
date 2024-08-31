#pragma once

#include "./result.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>

namespace chx::sql::mysql::detail {
template <typename T> struct visitor;

namespace tags {
struct basic_packet {};
}  // namespace tags

template <> struct visitor<tags::basic_packet> {
    template <typename Impl> struct operation {
        // just like llhttp
      private:
        enum Stage : std::uint8_t {
            PacketLength,
            SequenceId,
            Payload
        } stage = PacketLength;
        unsigned char header[4] = {};
        std::size_t stage_remain = 3;

        constexpr Impl& impl() noexcept(true) {
            return static_cast<Impl&>(*this);
        }

      public:
        constexpr void reset() noexcept(true) {
            stage = PacketLength;
            std::fill(header, header + 4, 0);
            stage_remain = 3;
        }

        constexpr std::uint32_t payload_length() const noexcept(true) {
            return header[0] + (header[1] << 8) + (header[2] << 16);
        }
        constexpr std::uint8_t sequence_id() const noexcept(true) {
            return header[3];
        }
        constexpr std::size_t remain_bytes() const noexcept(true) {
            return stage_remain;
        }

        // feed() should never Success
        constexpr packets2::PacketResult feed(const unsigned char*& begin,
                                              const unsigned char* end) {
            using namespace packets2;
            while (begin < end) {
                const std::size_t len = std::distance(begin, end);
                switch (stage) {
                case PacketLength: {
                    const std::size_t consumed = std::min(stage_remain, len);
                    std::copy(begin, begin + consumed,
                              header + 3 - stage_remain);
                    stage_remain -= consumed;
                    begin += consumed;
                    if (stage_remain == 0) {
                        stage = SequenceId;
                        stage_remain = 1;
                        if (PayloadResult r = impl().on_packet_length_complete(
                                payload_length());
                            r != PayloadSuccess) {
                            if (r == PayloadMalformed) {
                                return PacketMalformed;
                            } else {
                                return PacketInternalError;
                            }
                        }
                    }
                    break;
                }
                case SequenceId: {
                    header[3] = *(begin++);
                    stage_remain = payload_length();
                    stage = Payload;
                    if (PayloadResult r =
                            impl().on_sequence_id_complete(header[3]);
                        r != PayloadSuccess) {
                        if (r == PayloadMalformed) {
                            return PacketMalformed;
                        } else {
                            return PacketInternalError;
                        }
                    }
                    break;
                }
                case Payload: {
                    const std::size_t consumed = std::min(stage_remain, len);
                    stage_remain -= consumed;
                    begin += consumed;
                    bool is_final = !stage_remain;
                    PayloadResult r =
                        impl().on_payload_data(begin - consumed, begin);
                    if (is_final) {
                        reset();
                    }
                    // basic_packet will make sure that any PayloadSuccess/Pause
                    // would only return with *is_final*, it's payload parser's
                    // responsibility to make sure begin==end.
                    switch (r) {
                    case PayloadSuccess: {
                        if (is_final) {
                            continue;
                        } else {
                            return PacketTrailingData;
                        }
                    }
                    case PayloadPause: {
                        if (is_final) {
                            return PacketPaused;
                        } else {
                            return PacketTrailingData;
                        }
                    }
                    case PayloadNeedMore: {
                        if (!is_final) {
                            return PacketNeedMore;
                        } else {
                            return PacketIncomplete;
                        }
                    }
                    case PayloadTrailingData:
                        return PacketTrailingData;
                    case PayloadMalformed:
                    case PayloadMalformedHeader:
                        return PacketMalformed;
                    case PayloadInternalError:
                        return PacketInternalError;
                    default:
                        assert(false);
                    }
                }
                }
            }
            return PacketNeedMore;
        }
    };
};
}  // namespace chx::sql::mysql::detail