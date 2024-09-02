#pragma once

#include "../connection.hpp"
#include "./basic_type.hpp"
#include "./basic_parser.hpp"
#include "./msg/authentication.hpp"
#include "./msg/backend_key_data.hpp"
#include "./msg/ready_for_query.hpp"
#include "./message_parser.hpp"

#include <chx/net/async_write_some_exactly.hpp>
#include <chx/net/async_combine.hpp>

namespace chx::sql::postgresql::detail {
namespace tags {
struct connect {};
}  // namespace tags
template <> struct visitor<tags::connect> {
    template <typename Stream, typename CntlType = int>
    struct connect_operation {
      private:
        struct ev_send_startup_v2 {};
        struct ev_read_s1_v2 {};
        struct ev_send_oneshot_response {};
        struct ev_read_wait_success {};
        struct ev_read_till_ready {};

      public:
        template <typename T> using rebind = connect_operation<Stream, T>;
        using cntl_type = CntlType;

        connect_operation(connection<Stream>& conn, connect_parameters&& para)
            : __M_conn(conn), __M_para(std::move(para)) {}

        void operator()(cntl_type& cntl) {
            std::uint32_t raw_sz = 4 + 4 + 5 + __M_para.username.size() + 1 +
                                   9 + __M_para.database.size() + 1 + 1;
            std::vector<unsigned char> startup_raw(raw_sz);
            unsigned char* ptr = integer_to_network(raw_sz, startup_raw.data());
            ptr = integer_to_network(std::uint32_t{196608}, ptr);
            constexpr std::string_view user_key = "user";
            ptr = string_to_network(user_key.begin(), user_key.end(), ptr);
            ptr = string_to_network(__M_para.username.begin(),
                                    __M_para.username.end(), ptr);
            constexpr std::string_view database_key = "database";
            ptr = string_to_network(database_key.begin(), database_key.end(),
                                    ptr);
            ptr = string_to_network(__M_para.database.begin(),
                                    __M_para.database.end(), ptr);
            assert(ptr - startup_raw.data() + 1 == raw_sz);
            __M_state.template emplace<1>();
            net::async_write_some_exactly(
                __M_conn.stream(), std::move(startup_raw),
                cntl.template next_with_tag<ev_send_startup_v2>());
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_send_startup_v2) {
            if (!e) {
                __M_parsers.template emplace<1>();
                perform_read<ev_read_s1_v2>();
            } else {
                cntl.complete(e);
            }
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read_s1_v2) {
            if (!e) {
                do_read_s1_v2(buf_begin() = buf().data(),
                              buf_end() = buf().data() + s);
            } else {
                cntl.complete(e);
            }
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_send_oneshot_response) {
            if (!e) {
                do_wait_for_auth_success(buf_begin(), buf_end());
            } else {
                cntl.complete(e);
            }
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read_wait_success) {
            if (!e) {
                do_wait_for_auth_success(buf_begin() = buf().data(),
                                         buf_end() = buf().data() + s);
            } else {
                cntl.complete(e);
            }
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read_till_ready) {
            if (!e) {
                do_wait_till_ready(buf_begin() = buf().data(),
                                   buf_end() = buf().data() + s);
            } else {
                cntl.complete(e);
            }
        }

      private:
        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }

        connection<Stream>& __M_conn;
        connect_parameters __M_para;

        using wait_stage_parser =
            std::variant<std::monostate, basic_parser<msg::backend_key_data>,
                         basic_parser<msg::ready_for_query>>;
        std::variant<std::monostate, basic_parser<msg::authentication>,
                     wait_stage_parser>
            __M_parsers;
        constexpr std::vector<unsigned char>& buf() noexcept(true) {
            return __M_conn.__M_buffer;
        }
        constexpr const unsigned char*& buf_begin() noexcept(true) {
            return __M_conn.__M_buf_begin;
        }
        constexpr const unsigned char*& buf_end() noexcept(true) {
            return __M_conn.__M_buf_end;
        }
        template <typename Event> void perform_read() {
            buf_begin() = nullptr;
            buf_end() = nullptr;
            return __M_conn.stream().async_read_some(
                net::buffer(buf()), cntl().template next_with_tag<Event>());
        }

        std::variant<
            std::monostate, message_parser<msg::authentication>,
            message_parser<msg::authentication>,
            message_parser<msg::ready_for_query, msg::backend_key_data>>
            __M_state;

        void do_read_s1_v2(const unsigned char*& begin,
                           const unsigned char*& end) {
            message_parser<msg::authentication>& parser =
                std::get<1>(__M_state);
            ParseResult r = parser(begin, end);
            if (r == ParseSuccess) {
                auto val = parser.value();
                __M_state.template emplace<0>();
                const std::size_t idx = val.index();
                if (idx == 0) {
                    msg::authentication& auth = *std::get_if<0>(&val);
                    switch (auth.authentication_method()) {
                    case TrustOrFinished:
                        return cntl().complete(std::error_code{});

                    case CleartextPassword: {
                        const std::uint32_t raw_sz =
                            5 + __M_para.password.size() + 1;
                        std::vector<unsigned char> raw(raw_sz);
                        unsigned char* ptr = raw.data();
                        *(ptr++) = 'p';
                        ptr = integer_to_network(raw_sz - 1, ptr);
                        ptr = string_to_network(__M_para.password.begin(),
                                                __M_para.password.end(), ptr);
                        assert(raw.data() + raw_sz == ptr);
                        __M_state.template emplace<2>();
                        // begin may not eq end. but treat as response of
                        // Cleartext still.
                        return net::async_write_some_exactly(
                            __M_conn.stream().lowest_layer(), std::move(raw),
                            cntl()
                                .template next_with_tag<
                                    ev_send_oneshot_response>());
                    }
                    case MD5Password:
                    // do md5
                    case KerberosV5:
                    case GSS:
                    case SSPI:
                    case SASL:
                    default:
                        return cntl().complete(make_ec(
                            errc::sqlclient_unable_to_establish_sqlconnection));
                    }
                } else if (idx == 1) {
                    msg::error_response& err = *std::get_if<1>(&val);
                    return cntl().complete(make_ec(err.ec));
                } else {
                    // just dismiss, read next message
                    __M_state.template emplace<1>();
                    return do_read_s1_v2(begin, end);
                }
            } else if (r == ParseNeedMore) {
                return perform_read<ev_read_s1_v2>();
            } else {
                return cntl().complete(make_ec(errc::malformed_message));
            }
        }

        void do_wait_for_auth_success(const unsigned char*& begin,
                                      const unsigned char*& end) {
            message_parser<msg::authentication>& parser =
                std::get<2>(__M_state);
            ParseResult r = parser(begin, end);
            if (r == ParseSuccess) {
                auto val = parser.value();
                __M_state.template emplace<0>();
                const std::size_t idx = val.index();
                if (idx == 0) {
                    msg::authentication& auth = *std::get_if<0>(&val);
                    switch (auth.authentication_method()) {
                    case TrustOrFinished:
                        __M_state.template emplace<3>();
                        return do_wait_till_ready(begin, end);
                    default:
                        return cntl().complete(
                            make_ec(errc::malformed_message));
                    }
                } else if (idx == 1) {
                    msg::error_response& err = *std::get_if<1>(&val);
                    return cntl().complete(make_ec(err.ec));
                } else {
                    // just dismiss, read next message
                    __M_state.template emplace<1>();
                    return do_read_s1_v2(begin, end);
                }
            } else if (r == ParseNeedMore) {
                return perform_read<ev_read_wait_success>();
            } else {
                return cntl().complete(make_ec(errc::malformed_message));
            }
        }

        void do_wait_till_ready(const unsigned char*& begin,
                                const unsigned char*& end) {
            message_parser<msg::ready_for_query, msg::backend_key_data>&
                parser = std::get<3>(__M_state);
            ParseResult r = parser(begin, end);
            if (r == ParseSuccess) {
                auto val = parser.value();
                __M_state.template emplace<0>();
                switch (val.index()) {
                case 0: {
                    // ready for query
                    return cntl().complete(std::error_code{});
                }
                case 1:  // backend key data
                case 3:  // parameter status
                case 4:  // notice response
                {
                    __M_state.template emplace<3>();
                    return do_wait_till_ready(begin, end);
                }
                case 2: {
                    return cntl().complete(make_ec(std::get_if<2>(&val)->ec));
                }
                }
            } else if (r == ParseNeedMore) {
                return perform_read<ev_read_till_ready>();
            } else {
                return cntl().complete(make_ec(errc::malformed_message));
            }
        }
    };
};
}  // namespace chx::sql::postgresql::detail

template <typename Stream>
template <typename CompletionToken>
decltype(auto) chx::sql::postgresql::connection<Stream>::async_connect(
    connect_parameters&& para, CompletionToken&& completion_token) {
    return net::async_combine<const std::error_code&>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<detail::visitor<
            detail::tags::connect>::connect_operation<Stream>>{},
        *this, std::move(para));
}
