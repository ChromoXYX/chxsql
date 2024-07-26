#pragma once

#include "../connection.hpp"
#include "../error_code.hpp"
#include "./packets/handshake_v10.hpp"
#include "./fixed_integer.hpp"

#include "./auth_plugin.hpp"
#include "./auth/native_password.hpp"

#include <chx/net/async_write_sequence_exactly.hpp>
#include <chx/ser2/rule.hpp>
#include <chx/ser2/bind.hpp>
#include <chx/ser2/struct_getter.hpp>
#include <chx/ser2/if.hpp>
#include <chx/log.hpp>

namespace chx::sql::mysql::detail {
namespace tags {
struct connection_phase_operation {};
}  // namespace tags

template <> struct visitor<tags::connection_phase_operation> {
    template <typename Stream, typename CntlType = int>
    struct operation : protected auth_plugin<
                           operation<Stream, CntlType>,
                           auth::native_password<operation<Stream, CntlType>>> {
        template <typename T> using rebind = operation<Stream, T>;

        friend visitor<tags::native_pw>;

        using cntl_type = CntlType;
        using auth_plugin_type =
            auth_plugin<operation<Stream, CntlType>,
                        auth::native_password<operation<Stream, CntlType>>>;
        using auth_plugin_type::operator();
        using auth_plugin_type::process_packet;

        struct ev_connect {};
        template <typename Event> struct ev_recv {};
        struct ev_handshakeV10 {};

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
            c.stream().async_connect(ep,
                                     cntl.template next_with_tag<ev_connect>());
        }

        void operator()(cntl_type& cntl, const std::error_code& e, ev_connect) {
            if (!e) {
                recv_packet(ev_handshakeV10{});
            } else {
                cntl.complete(e);
            }
        }

        template <typename Event>
        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_recv<Event>) {
            if (!e) {
                recv_packet_handle(e, s, ev_recv<Event>{});
            } else {
                cntl.complete(e);
            }
        }

        template <typename Event> void recv_packet(Event) {
            __M_buf_sz = 0;
            __M_buf.clear();
            recv_packet_next(ev_recv<Event>{});
        }

        template <typename Event> void recv_packet_next(ev_recv<Event>) {
            // there should be at least 128 bytes available in buffer after
            // buf_sz
            if (__M_buf_sz + 128 > __M_buf.size()) {
                __M_buf.resize(__M_buf_sz + 128);
            }
            c.stream().async_read_some(
                net::mutable_buffer(__M_buf.data() + __M_buf_sz,
                                    __M_buf.size() - __M_buf_sz),
                cntl().template next_with_tag<ev_recv<Event>>());
        }

        template <typename Event>
        void recv_packet_handle(const std::error_code& e, std::size_t s,
                                ev_recv<Event>) {
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
                        return process_packet(Event{}, __M_buf.data(),
                                              __M_buf.data() + packet_length +
                                                  4);
                    } else {
                        // packet with wrong sequence id
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET));
                    }
                } else if (__M_buf_sz < packet_length + 4) {
                    return recv_packet_next(ev_recv<Event>{});
                } else {
                    // packet with trailing data
                    return cntl().complete(make_ec(errc::CR_MALFORMED_PACKET));
                }
            } else {
                return recv_packet_next(ev_recv<Event>{});
            }
        }

        void process_packet(ev_handshakeV10, const unsigned char* begin,
                            const unsigned char* end) {
            std::uint32_t packet_length =
                fixed_length_integer_from_network<3>(begin);
            assert(std::distance(begin, end) == packet_length + 4);
            begin += 4;

            packets::handshake_v10 handshake;
            if (ser2::ParseResult r = handshake.parse(begin, end);
                r != ser2::ParseResult::Ok || begin != end) {
                return cntl().complete(make_ec(errc::CR_MALFORMED_PACKET));
            }

            if (!auth_plugin_type::perform_auth(handshake)) {
                return cntl().complete(make_ec(errc::CR_AUTH_PLUGIN_ERR));
            }
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
