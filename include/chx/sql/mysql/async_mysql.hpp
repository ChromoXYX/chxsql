#pragma once

#include <chx/net/ssl/stream.hpp>
#include "./basic_types.hpp"

namespace chx::sql::mysql {
namespace detail {
template <typename AuthOper> struct operation_cmd {};

template <typename Stream, typename CntlType = int>
struct operation_auth
    : std::conditional_t<net::ssl::is_ssl_stream<std::decay_t<Stream>>::value,
                         operation_cmd<operation_auth<Stream, CntlType>>,
                         operation_cmd<operation_auth<Stream, CntlType>>> {
  private:
    template <typename Task> struct __ev_recv_pack {};
    struct __ev_initial_handshake {};

  public:
    friend struct operation_cmd<operation_auth>;
    template <typename T> using rebind = operation_auth<Stream, T>;
    using cntl_type = CntlType;

    template <typename Strm>
    operation_auth(Strm&& strm)
        : __M_stream(std::forward<Strm>(strm)), __M_inbuf(4096) {}

    constexpr auto& stream() noexcept(true) { return __M_stream; }
    constexpr cntl_type& cntl() noexcept(true) {
        return static_cast<cntl_type*>(this);
    }

    void operator()(cntl_type& cntl) {
        stream().async_read_some(net::buffer(__M_inbuf),
                                 cntl.template next_with_tag<
                                     __ev_recv_pack<__ev_initial_handshake>>());
    }

    template <typename Task>
    void operator()(cntl_type& cntl, const std::error_code& e, std::size_t s,
                    __ev_recv_pack<Task>) {
        if (!e) {
            feed(__M_inbuf.data(), __M_inbuf.data() + s,
                 __ev_recv_pack<Task>{});
        } else {
        }
        cntl.complete(e);
    }

  private:
    std::uint8_t seq_id_next = 0;

    struct __packet_type {
        std::uint64_t payload_length = 0;
        std::uint8_t sequence_id = 0;

        std::array<unsigned char, 4> header_buf;
        std::uint8_t header_buf_len = 0;
        constexpr void extract_header() noexcept(true) {
            payload_length =
                fixed_length_integer_from_network<3>(header_buf.data());
            sequence_id = header_buf[3];
        }

        enum Status { Header, Payload } status = Header;

        std::vector<unsigned char> payload;

        void clear() noexcept(true) {
            payload_length = 0;
            sequence_id = 0;
            header_buf_len = 0;
            status = Header;
            payload.clear();
        }
    } __current_packet;
    std::vector<__packet_type> __packets;

    struct {
        constexpr void shutdown_recv() noexcept(true) { v &= ~1; }
        constexpr void shutdown_send() noexcept(true) { v &= ~2; }
        constexpr void shutdown_both() noexcept(true) {
            shutdown_recv();
            shutdown_send();
        }
        constexpr bool want_recv() const noexcept(true) { return v | 1; }
        constexpr bool want_send() const noexcept(true) { return v | 2; }

        constexpr void send_goaway() noexcept(true) { v |= 4; }
        constexpr bool goaway_sent() const noexcept(true) { return v | 4; }

      private:
        std::uint8_t v = 1 | 2;
    } io_cntl;

    template <typename Task>
    void feed(unsigned char* begin, unsigned char* end, __ev_recv_pack<Task>) {
        while (begin < end && io_cntl.want_recv()) {
            std::size_t len = end - begin;
            __packet_type& packet = __current_packet;
            switch (packet.status) {
            case packet.Header: {
                if (len >= 4 - packet.header_buf_len) {
                    begin = std::memcpy(packet.header_buf.data() +
                                            packet.header_buf_len,
                                        begin, 4 - packet.header_buf_len);
                    packet.extract_header();
                    packet.status = packet.Payload;
                    if (packet.sequence_id != seq_id_next ||
                        packet.payload_length == 0xffffff) [[unlikely]] {
                        return terminate_now();
                    }
                } else {
                    begin = std::memcpy(packet.header_buf.data() +
                                            packet.header_buf_len,
                                        begin, len);
                    packet.header_buf_len += len;
                }
                break;
            }
            case packet.Payload: {
                std::size_t old_sz = packet.payload.size();
                packet.payload.resize(
                    old_sz + std::min(len, packet.payload_length - old_sz));
                begin = std::memcpy(packet.payload.data() + old_sz, begin,
                                    packet.payload.size() - old_sz);
                if (packet.payload.size() == packet.payload_length) {
                    if (packet.payload_length != 0xffffff) {
                        process();
                    } else {
                        __packets.emplace_back(std::move(packet));
                    }
                    packet.clear();
                }
                break;
            }
            }
        }
    }

    Stream __M_stream;
    std::vector<unsigned char> __M_inbuf;

    void cancel_all() { cntl()(nullptr); }

    void terminate_now() {
        io_cntl.shutdown_both();
        cancel_all();
    }

    void process() {}
};
}  // namespace detail
}  // namespace chx::sql::mysql