#pragma once

#include <cassert>
#include <openssl/evp.h>
#include <chx/log.hpp>
#include <chx/net/async_write_sequence_exactly.hpp>
#include <unistd.h>

#include "../visitor.hpp"
#include "../../error_code.hpp"
#include "../fixed_integer.hpp"
#include "./hash_ctx.hpp"
#include "../packets/handshake_v10.hpp"
#include "../packets/handshake_response.hpp"
#include "../packets/OK_packet.hpp"
#include "../packets/ERR_Packet.hpp"

namespace chx::sql::mysql::detail {
namespace tags {
struct native_pw {};
}  // namespace tags

template <> struct visitor<tags::native_pw> {
    template <typename Oper> struct native_pw {
      protected:
        constexpr static std::string_view auth_plugin_name =
            "mysql_native_password";

        struct ev_native_pw_send_handshake_response {};
        struct ev_native_pw_packet_after_handshake_response {};

        void perform_auth(const packets::handshake_v10& packet) {
            if (packet.auth_plugin_data_len < 9) {
                return oper().cntl().complete(
                    make_ec(errc::CR_MALFORMED_PACKET));
            }
            packets::handshake_response<std::array<unsigned char, 20>> resp =
                {};
            resp.client_flag =
                0x013ea20f | capabilities_flags::CLIENT_CONNECT_WITH_DB;
            oper().c.client_cap(resp.client_flag);
            resp.max_packet_size = 16777216;
            resp.character_set = 255;
            resp.username = "root";
            resp.database = "test";

            try {
                generate_pw(resp.auth_response.data(),
                            packet.auth_plugin_data_part_1.data(), 8,
                            packet.auth_plugin_data_part_2.data(),
                            packet.auth_plugin_data_len - 9, "123456", 6);
            } catch (const auth::bad_hash&) {
                return oper().cntl().complete(
                    make_ec(errc::CR_AUTH_PLUGIN_ERR));
            }
            resp.client_plugin_name = auth_plugin_name;
            auto& attrs = resp.connection_attrs;
            attrs.emplace_back("_pid",
                               log::format(CHXLOG_STR("%d"), ::getpid()));
            attrs.emplace_back("_platform", "x86_64");
            attrs.emplace_back("_client_name", "chxsql");
            attrs.emplace_back("os_user", "chromo");
            attrs.emplace_back("_client_version", "0.0.1");
            attrs.emplace_back("program_name", "chxsql");

            std::vector<unsigned char> resp_data;
            const auto& resp_rule = resp.rule();
            std::size_t payload_n = resp_rule.calculate_size(resp);
            resp_data.resize(payload_n + 4);
            fixed_length_integer_to_network<3>(payload_n, resp_data.data());
            fixed_length_integer_to_network<1>(oper().next_sequence_id++,
                                               resp_data.data() + 3);

            try {
                if (std::size_t sz =
                        std::distance(resp_data.data(),
                                      resp_rule.generate(
                                          resp, resp_data.data() + 4,
                                          resp_data.data() + resp_data.size()));
                    sz != payload_n + 4) {
                    return oper().cntl().complete(
                        make_ec(errc::CR_AUTH_PLUGIN_ERR));
                }
            } catch (const std::out_of_range&) {
                return oper().cntl().complete(
                    make_ec(errc::CR_AUTH_PLUGIN_ERR));
            }

            net::async_write_sequence_exactly(
                oper().c.stream(), std::move(resp_data),
                oper()
                    .cntl()
                    .template next_with_tag<
                        ev_native_pw_send_handshake_response>());
        }

        template <typename CntlType>
        void operator()(CntlType& cntl, const std::error_code& e, std::size_t s,
                        ev_native_pw_send_handshake_response) {
            if (!e) {
                oper().recv_packet(
                    ev_native_pw_packet_after_handshake_response{});
            } else {
                cntl.complete(e);
            }
        }

        void process_packet(ev_native_pw_packet_after_handshake_response,
                            const unsigned char* begin,
                            const unsigned char* end) {
            std::uint32_t packet_length =
                fixed_length_integer_from_network<3>(begin);
            if (packet_length <= 1) {
                return oper().cntl().complete(
                    make_ec(errc::CR_MALFORMED_PACKET));
            }
            assert(std::distance(begin, end) == packet_length + 4);
            begin += 4;
            std::uint8_t packet_type = *(begin + 4);
            std::uint8_t next_sequence_id = oper().next_sequence_id;
            if (packet_type == 0x00) {
                packets::OK_Packet ok_packet;
                auto&& rule =
                    ok_packet.template rule<0x00>(oper().c.client_cap());
                if (ser2::ParseResult r = rule.parse(ok_packet, begin, end);
                    r != ser2::ParseResult::Ok || begin != end) {
                    return oper().cntl().complete(
                        make_ec(errc::CR_MALFORMED_PACKET));
                }
                oper().cntl().complete(std::error_code{});
            } else if (packet_type == 0xff) {
                packets::ERR_Packet err_packet;
                auto&& rule = err_packet.rule(oper().c.client_cap());
                if (ser2::ParseResult r = rule.parse(err_packet, begin, end);
                    r != ser2::ParseResult::Ok || begin != end) {
                    return oper().cntl().complete(
                        make_ec(errc::CR_MALFORMED_PACKET));
                }
                oper().cntl().complete(make_ec(err_packet.error_code));
            } else {
                oper().cntl().complete(make_ec(errc::CR_AUTH_PLUGIN_ERR));
            }
        }

      private:
        constexpr Oper& oper() noexcept(true) {
            return static_cast<Oper&>(*this);
        }

        inline static void generate_pw(unsigned char* buf, const void* nonce1,
                                       unsigned int nonce1_n,
                                       const void* nonce2,
                                       unsigned int nonce2_n,
                                       const void* password,
                                       unsigned int password_n) {
            auth::hash_ctx ctx("SHA1");

            ctx.init();
            ctx.update(password, password_n);
            unsigned char digest_a[EVP_MAX_MD_SIZE] = {};
            assert(ctx.finalize(digest_a) == 20);

            ctx.init();
            ctx.update(digest_a, 20);
            unsigned char digest_b[EVP_MAX_MD_SIZE] = {};
            assert(ctx.finalize(digest_b) == 20);

            ctx.init();
            ctx.update(nonce1, nonce1_n);
            ctx.update(nonce2, nonce2_n);
            ctx.update(digest_b, 20);
            unsigned char digest_c[EVP_MAX_MD_SIZE] = {};
            assert(ctx.finalize(digest_c) == 20);

            for (int i = 0; i < 20; ++i) {
                buf[i] = digest_a[i] ^ digest_c[i];
            }
        }
    };
};
}  // namespace chx::sql::mysql::detail

namespace chx::sql::mysql::detail::auth {
template <typename Oper>
using native_password = visitor<tags::native_pw>::native_pw<Oper>;
}  // namespace chx::sql::mysql::detail::auth