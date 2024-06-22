#pragma once

#include "../connection.hpp"
#include "../basic_types.hpp"
#include "../error_code.hpp"
#include "./packets/handshake_v10.hpp"

#include <chx/ser2/rule.hpp>
#include <chx/ser2/bind.hpp>
#include <chx/ser2/struct_getter.hpp>
#include <chx/ser2/if.hpp>

namespace chx::sql::mysql::detail {
namespace tags {
struct connection_phase_operation {};
}  // namespace tags

template <> struct visitor<tags::connection_phase_operation> {
    template <typename Stream, typename CntlType = int> struct operation {
        template <typename T> using rebind = operation<Stream, T>;
        using cntl_type = CntlType;

        struct ev_connect {};
        template <typename Event> struct ev_recv_full_packet {};

        struct ev_handshakeV10 {
            using packet = packets::handshake_v10;
        };

        connection<Stream>& c;
        net::ip::tcp::endpoint ep;

        operation(connection<Stream>& conn,
                  const net::ip::tcp::endpoint& e) noexcept(true)
            : c(conn), ep(e) {}

        std::vector<unsigned char> __M_buf;
        std::size_t __M_buf_sz;
        std::uint8_t next_sequence_id;

        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }

        void operator()(cntl_type& cntl) {
            c.__M_stream.async_connect(
                ep, cntl.template next_with_tag<ev_connect>());
        }

        void operator()(cntl_type& cntl, const std::error_code& e, ev_connect) {
            if (!e) {
                recv_packet(ev_recv_full_packet<ev_handshakeV10>{});
            } else {
                cntl.complete(e);
            }
        }

        template <typename Event>
        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_recv_full_packet<Event>) {
            if (!e) {
                recv_packet_handle(e, s, ev_recv_full_packet<Event>{});
            } else {
                cntl.complete(e);
            }
        }

        template <typename Event> void recv_packet(ev_recv_full_packet<Event>) {
            __M_buf_sz = 0;
            __M_buf.clear();
            recv_packet_next(ev_recv_full_packet<Event>{});
        }

        template <typename Event>
        void recv_packet_next(ev_recv_full_packet<Event>) {
            // there should be at least 128 bytes available in buffer after
            // buf_sz
            if (__M_buf_sz + 128 > __M_buf.size()) {
                __M_buf.resize(__M_buf_sz + 128);
            }
            c.__M_stream.async_read_some(
                net::mutable_buffer(__M_buf.data() + __M_buf_sz,
                                    __M_buf.size() - __M_buf_sz),
                cntl().template next_with_tag<ev_recv_full_packet<Event>>());
        }

        template <typename Event>
        void recv_packet_handle(const std::error_code& e, std::size_t s,
                                ev_recv_full_packet<Event>) {
            __M_buf_sz += s;
            if (__M_buf_sz >= 3) {
                std::uint32_t packet_length =
                    fixed_length_integer_from_network<3>(__M_buf.data());
                if (__M_buf_sz == packet_length + 4) {
                    std::uint8_t sequence_id =
                        fixed_length_integer_from_network<1>(__M_buf.data() +
                                                             3);
                    if (sequence_id == next_sequence_id) {
                        next_sequence_id++;
                        typename Event::packet packet;
                        const char* begin = (const char*)__M_buf.data() + 4;
                        ser2::ParseResult pr = packet.parse(
                            begin, (const char*)__M_buf.data() + __M_buf_sz);
                        printf("%s\n", packet.to_string().c_str());
                        if ((begin - (const char*)__M_buf.data() ==
                             packet_length + 4) &&
                            pr == ser2::ParseResult::Ok) {
                            return process_packet(packet);
                        } else {
                            // failed to parse packet
                            return cntl().complete(
                                make_ec(errc::CR_MALFORMED_PACKET));
                        }
                    } else {
                        // packet with wrong sequence id
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET));
                    }
                } else if (__M_buf_sz < packet_length + 4) {
                    return recv_packet_next(ev_recv_full_packet<Event>{});
                } else {
                    // packet with trailing data
                    return cntl().complete(make_ec(errc::CR_MALFORMED_PACKET));
                }
            } else {
                return recv_packet_next(ev_recv_full_packet<Event>{});
            }
        }

        void process_packet(packets::handshake_v10& packet) {
            cntl().complete(std::error_code{});
        }
    };
};
}  // namespace chx::sql::mysql::detail

template <typename Stream>
template <typename CompletionToken>
decltype(auto) chx::sql::mysql::connection<Stream>::async_connect(
    const net::ip::tcp::endpoint& ep, CompletionToken&& completion_token) {
    using operation_type =
        decltype(detail::visitor<
                 detail::tags::connection_phase_operation>::operation(*this,
                                                                      ep));
    return net::async_combine<const std::error_code&>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<operation_type>{}, *this, ep);
}
