#pragma once

#include "../connection.hpp"
#include "./basic_packet.hpp"
#include "./hash_ctx.hpp"
#include "./fixed_integer.hpp"
#include "../error_code.hpp"
#include "./packets/HandshakeV10_Packet.hpp"
#include "./packets/ERR_Packet.hpp"
#include "./packets/OK_Packet.hpp"

#include <chx/net/async_write_some_exactly.hpp>

namespace chx::sql::mysql::detail {
namespace tags {
struct connection2 {};
}  // namespace tags

template <> struct visitor<tags::connection2> {
    template <typename Stream, typename CntlType = int>
    struct operation : private visitor<tags::basic_packet>::operation<
                           operation<Stream, CntlType>> {
        template <typename T> using rebind = operation<Stream, T>;
        using cntl_type = CntlType;

        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }

      private:
        struct ev_read_stage_1 {};
        struct ev_read_stage_2 {};
        struct ev_send_handshake_response {};

        using PayloadResult = packets2::PayloadResult;
        using PacketResult = packets2::PacketResult;
        using packet_parser_type =
            visitor<tags::basic_packet>::operation<operation>;
        friend packet_parser_type;

      public:
        constexpr operation(connection<Stream>& c, connect_parameters&& param)
            : __M_conn(c), __M_param(std::move(param)) {}

        void operator()(cntl_type& cntl) {
            __M_buf.resize(256);
            __M_conn.stream().lowest_layer().async_read_some(
                net::buffer(__M_buf),
                cntl.template next_with_tag<ev_read_stage_1>());
        }
        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read_stage_1) {
            if (!e) {
                const unsigned char *begin = __M_buf.data(), *end = begin + s;
                PacketResult r = packet_parser_type::feed(begin, end);
                switch (r) {
                case packets2::PacketPaused: {
                    switch (stage) {
                    case S2_Guess: {
                        if (begin == end) {
                            return net::async_write_some_exactly(
                                __M_conn.stream().lowest_layer(),
                                std::move(__M_raw),
                                cntl.template next_with_tag<
                                    ev_send_handshake_response>());
                        } else {
                            return cntl.complete(
                                make_ec(errc::CR_MALFORMED_PACKET));
                        }
                    }
                    case S1_ERR: {
                        return cntl.complete(__M_ec);
                    }
                    default: {
                        assert(false);
                    }
                    }
                }
                case packets2::PacketNeedMore: {
                    return __M_conn.stream().lowest_layer().async_read_some(
                        net::buffer(__M_buf),
                        cntl.template next_with_tag<ev_read_stage_1>());
                }
                case packets2::PacketIncomplete:
                case packets2::PacketTrailingData:
                case packets2::PacketMalformed:
                    return cntl.complete(make_ec(errc::CR_MALFORMED_PACKET));
                case packets2::PacketInternalError:
                    return cntl.complete(
                        __M_ec ? __M_ec : make_ec(errc::CR_UNKNOWN_ERROR));
                default:
                    assert(false);
                }
            } else {
                cntl.complete(e);
            }
        }
        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read_stage_2) {
            if (!e) {
                const unsigned char *begin = __M_buf.data(), *end = begin + s;
                PacketResult r = packet_parser_type::feed(begin, end);
                switch (r) {
                case packets2::PacketPaused: {
                    if (begin == end) {
                        return cntl.complete(__M_ec);
                    } else {
                        return cntl.complete(
                            make_ec(errc::CR_MALFORMED_PACKET));
                    }
                }
                case packets2::PacketNeedMore: {
                    return __M_conn.stream().lowest_layer().async_read_some(
                        net::buffer(__M_buf),
                        cntl.template next_with_tag<ev_read_stage_2>());
                }
                case packets2::PacketIncomplete:
                case packets2::PacketTrailingData:
                case packets2::PacketMalformed:
                    return cntl.complete(make_ec(errc::CR_MALFORMED_PACKET));
                case packets2::PacketInternalError:
                    return cntl.complete(
                        __M_ec ? __M_ec : make_ec(errc::CR_UNKNOWN_ERROR));
                default:
                    assert(false);
                }
            } else {
                cntl.complete(e);
            }
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_send_handshake_response) {
            if (!e) {
                __M_conn.stream().lowest_layer().async_read_some(
                    net::buffer(__M_buf),
                    cntl.template next_with_tag<ev_read_stage_2>());
            } else {
                cntl.complete(e);
            }
        }

      private:
        enum Stage {
            InitialGuess,
            S1_ERR,
            S1_Handshake,
            S2_Guess,
            S2_OK,
            S2_ERR,
            Finished
        } stage = InitialGuess;

        constexpr PayloadResult
        on_packet_length_complete(std::uint32_t) noexcept(true) {
            return PayloadResult::PayloadSuccess;
        }
        constexpr PayloadResult
        on_sequence_id_complete(std::uint8_t sequence_id) noexcept(true) {
            if (next_sequence_id++ == sequence_id) {
                return PayloadResult::PayloadSuccess;
            } else {
                return PayloadResult::PayloadMalformed;
            }
        }
        constexpr PayloadResult on_payload_data(const unsigned char* begin,
                                                const unsigned char* end) {
            using namespace packets2;
            switch (stage) {
            case InitialGuess: {
                if (begin == end) {
                    return PayloadNeedMore;
                }
                const std::uint8_t first_byte = *begin;
                if (first_byte != 0xff) {
                    stage = S1_Handshake;
                } else {
                    __M_payload_parser.template emplace<1>();
                    stage = S1_ERR;
                    return on_payload_data(begin, end);
                }
            }
            case S1_Handshake: {
                HandshakeV10_Packet::parser& parser =
                    std::get<0>(__M_payload_parser);
                if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                    if ((r = do_handshake_response(parser.value())) ==
                        PayloadSuccess) {
                        parser = {};
                        stage = S2_Guess;
                        return make_pause(begin, end);
                    } else {
                        return r;
                    }
                } else {
                    return r;
                }
            }
            case S2_Guess: {
                if (begin == end) {
                    return PayloadNeedMore;
                }
                const std::uint8_t first_byte = *begin;
                if (first_byte == 0x00) {
                    stage = S2_OK;
                    __M_payload_parser.template emplace<2>();
                } else if (first_byte == 0xff) {
                    stage = S2_ERR;
                    __M_payload_parser.template emplace<1>();
                    return on_payload_data(begin, end);
                } else {
                    return PayloadInternalError;
                }
            }
            case S2_OK: {
                OK_Packet::parser<>& parser = std::get<2>(__M_payload_parser);
                if (PayloadResult r =
                        parser(__M_conn.client_cap(), begin, end,
                               packet_parser_type::remain_bytes() == 0);
                    r == PayloadSuccess) {
                    __M_ec = {};
                    parser = {};
                    stage = Finished;
                    return make_pause(begin, end);
                } else {
                    return r;
                }
            }
            case S1_ERR:
            case S2_ERR: {
                ERR_Packet::parser& parser = std::get<1>(__M_payload_parser);
                if (PayloadResult r = parser(
                        begin, end, packet_parser_type::remain_bytes() == 0);
                    r == PayloadSuccess) {
                    __M_ec = make_ec(parser.value().error_code);
                    parser = {};
                    stage = Finished;
                    return make_pause(begin, end);
                } else {
                    return r;
                }
            }
            default:
                assert(false);
            }
        }

        PayloadResult
        do_handshake_response(const packets2::HandshakeV10_Packet& packet) {
            if (packet.auth_plugin_name != "mysql_native_password") {
                __M_ec = make_ec(errc::CR_AUTH_PLUGIN_ERR);
                return PayloadResult::PayloadInternalError;
            }
            using namespace packets2;
            assert(packet.auth_plugin_data_len > 9);

            // cap
            std::uint32_t client_flag = 0x012ea207;
            if (!__M_param.database.empty()) {
                client_flag |= capabilities_flags::CLIENT_CONNECT_WITH_DB;
            }
            __M_conn.client_cap(client_flag);

            //
            std::size_t payload_length = 4 + 4 + 1 + 23 +
                                         (__M_param.username.size() + 1) + 21 +
                                         sizeof("mysql_native_password");
            if (!__M_param.database.empty()) {
                payload_length += __M_param.database.size() + 1;
            }
            std::vector<unsigned char> raw(4 + payload_length);
            fixed_length_integer_to_network<3>(payload_length, raw.data());
            raw[3] = next_sequence_id++;

            unsigned char* ptr = raw.data() + 4;
            fixed_length_integer_to_network<4>(client_flag, ptr);
            ptr += 4;
            fixed_length_integer_to_network<4>(0xffffff, ptr);
            ptr += 4;
            *(ptr++) = 0x21;
            ptr += 23;  // filler

            ptr = std::copy(__M_param.username.begin(),
                            __M_param.username.end(), ptr);
            *(ptr++) = 0;  // username nul

            *(ptr++) = 20;
            // generate password
            try {
                generate_pw(ptr, packet.auth_plugin_data_part_1.data(), 8,
                            packet.auth_plugin_data_part_2.data(),
                            packet.auth_plugin_data_len - 9,
                            __M_param.password.c_str(),
                            __M_param.password.size());
                ptr += 20;
            } catch (const bad_hash&) {
                return PayloadInternalError;
            }

            if (!__M_param.database.empty()) {
                ptr = std::copy(__M_param.database.begin(),
                                __M_param.database.end(), ptr);
                *(ptr++) = 0;
            }

            constexpr std::string_view plugin_name = "mysql_native_password";
            ptr = std::copy(plugin_name.begin(), plugin_name.end(), ptr);
            *(ptr++) = 0;

            assert(ptr == raw.data() + raw.size());

            __M_raw = std::move(raw);
            return PayloadSuccess;
        }

        std::vector<unsigned char> __M_buf;
        std::vector<unsigned char> __M_raw;
        connection<Stream>& __M_conn;
        connect_parameters __M_param;
        std::uint8_t next_sequence_id = 0;
        std::variant<packets2::HandshakeV10_Packet::parser,
                     packets2::ERR_Packet::parser,
                     packets2::OK_Packet::parser<>>
            __M_payload_parser;
        std::error_code __M_ec = {};

        inline static void generate_pw(unsigned char* buf, const void* nonce1,
                                       unsigned int nonce1_n,
                                       const void* nonce2,
                                       unsigned int nonce2_n,
                                       const void* password,
                                       unsigned int password_n) {
            hash_ctx ctx("SHA1");

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

template <typename Stream>
template <typename CompletionToken>
decltype(auto) chx::sql::mysql::connection<Stream>::async_connect(
    connect_parameters&& para, CompletionToken&& completion_token) {
    return net::async_combine<const std::error_code&>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<
            detail::visitor<detail::tags::connection2>::operation<Stream>>{},
        *this, std::move(para));
}