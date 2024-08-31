#pragma once

#include "../connection.hpp"
#include "./basic_type.hpp"
#include "./basic_parser.hpp"
#include "./msg/authentication.hpp"
#include "./msg/backend_key_data.hpp"
#include "./msg/notice_response.hpp"
#include "./msg/parameter_status.hpp"
#include "./msg/ready_for_query.hpp"

#include <chx/net/async_write_some_exactly.hpp>
#include <chx/net/async_combine.hpp>

namespace chx::sql::postgresql::detail {
template <typename Stream, typename CntlType = int> struct connect_operation {
  private:
    struct ev_send_startup {};
    struct ev_read_s1 {};
    struct ev_send_cleartext {};
    struct ev_read_ok_or_err {};
    struct ev_read_till_ready {};

  public:
    template <typename T> using rebind = connect_operation<Stream, T>;
    using cntl_type = CntlType;

    connect_operation(connection<Stream>& conn, connect_parameters&& para)
        : __M_conn(conn), __M_para(std::move(para)) {}

    void operator()(cntl_type& cntl) {
        std::uint32_t raw_sz = 4 + 4 + 5 + __M_para.username.size() + 1 + 9 +
                               __M_para.database.size() + 1 + 1;
        std::vector<unsigned char> startup_raw(raw_sz);
        unsigned char* ptr = integer_to_network(raw_sz, startup_raw.data());
        ptr = integer_to_network(std::uint32_t{196608}, ptr);
        constexpr std::string_view user_key = "user";
        ptr = string_to_network(user_key.begin(), user_key.end(), ptr);
        ptr = string_to_network(__M_para.username.begin(),
                                __M_para.username.end(), ptr);
        constexpr std::string_view database_key = "database";
        ptr = string_to_network(database_key.begin(), database_key.end(), ptr);
        ptr = string_to_network(__M_para.database.begin(),
                                __M_para.database.end(), ptr);
        assert(ptr - startup_raw.data() + 1 == raw_sz);

        net::async_write_some_exactly(
            __M_conn.stream(), std::move(startup_raw),
            cntl.template next_with_tag<ev_send_startup>());
    }

    void operator()(cntl_type& cntl, const std::error_code& e, std::size_t s,
                    ev_send_startup) {
        if (!e) {
            __M_parsers.template emplace<1>();
            __M_buf.resize(256);
            __M_conn.stream().async_read_some(
                net::buffer(__M_buf),
                cntl.template next_with_tag<ev_read_s1>());
        } else {
            cntl.complete(e);
        }
    }

    void operator()(cntl_type& cntl, const std::error_code& e, std::size_t s,
                    ev_read_s1) {
        if (!e) {
            // currently we only implement md5, clear text or trust
            const unsigned char *begin = __M_buf.data(), *end = begin + s;
            basic_parser<msg::authentication>& parser =
                std::get<1>(__M_parsers);
            if (ParseResult r = parser(begin, end); r == ParseSuccess) {
                if (begin != end) {
                    return cntl.complete(make_ec(errc::malformed_message));
                }
                std::variant<msg::authentication, msg::error_response> m =
                    parser.value();
                __M_parsers.template emplace<0>();
                if (m.index() == 0) {
                    switch (std::get_if<0>(&m)->authentication_method()) {
                    case TrustOrFinished:
                        return cntl.complete(std::error_code{});
                    case CleartextPassword:
                        return do_cleartext();
                    case MD5Password:
                        return do_md5();
                    case KerberosV5:
                    case GSS:
                    case SSPI:
                    case SASL:
                        return cntl.complete(make_ec(
                            errc::sqlclient_unable_to_establish_sqlconnection));
                    }
                } else {
                    cntl.complete(make_ec(std::get_if<1>(&m)->ec));
                }
            } else if (r == ParseNeedMore) {
                __M_conn.stream().async_read_some(
                    net::buffer(__M_buf),
                    cntl.template next_with_tag<ev_read_s1>());
            } else {
                cntl.complete(make_ec(errc::malformed_message));
            }
        } else {
            cntl.complete(e);
        }
    }

    void operator()(cntl_type& cntl, const std::error_code& e, std::size_t s,
                    ev_send_cleartext) {
        if (!e) {
            __M_conn.stream().async_read_some(
                net::buffer(__M_buf),
                cntl.template next_with_tag<ev_read_ok_or_err>());
        } else {
            cntl.complete(e);
        }
    }

    void operator()(cntl_type& cntl, const std::error_code& e, std::size_t s,
                    ev_read_ok_or_err) {
        if (!e) {
            const unsigned char *begin = __M_buf.data(), *end = begin + s;
            basic_parser<msg::authentication>& parser =
                std::get<1>(__M_parsers);
            if (ParseResult r = parser(begin, end); r == ParseSuccess) {
                std::variant<msg::authentication, msg::error_response> m =
                    parser.value();
                __M_parsers.template emplace<0>();
                if (m.index() == 0) {
                    if (std::get_if<0>(&m)->authentication_method() ==
                        TrustOrFinished) {
                        __M_parsers.template emplace<2>();
                        return do_read_till_ready(begin, end);
                    } else {
                        return cntl.complete(make_ec(
                            errc::sqlclient_unable_to_establish_sqlconnection));
                    }
                } else {
                    cntl.complete(make_ec(std::get_if<1>(&m)->ec));
                }
            } else if (r == ParseNeedMore) {
                __M_conn.stream().async_read_some(
                    net::buffer(__M_buf),
                    cntl.template next_with_tag<ev_read_ok_or_err>());
            } else {
                cntl.complete(make_ec(errc::malformed_message));
            }
        } else {
            cntl.complete(e);
        }
    }

    void operator()(cntl_type& cntl, const std::error_code& e, std::size_t s,
                    ev_read_till_ready) {
        if (!e) {
            do_read_till_ready(__M_buf.data(), __M_buf.data() + s);
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

    using wait_stage_parser = std::variant<
        std::monostate, basic_parser<msg::backend_key_data>,
        basic_parser<msg::parameter_status>, basic_parser<msg::notice_response>,
        basic_parser<msg::ready_for_query>, basic_parser<msg::error_response>>;
    std::variant<std::monostate, basic_parser<msg::authentication>,
                 wait_stage_parser>
        __M_parsers;
    std::vector<unsigned char> __M_buf;

    void do_cleartext() {
        const std::uint32_t raw_sz = 5 + __M_para.password.size() + 1;
        std::vector<unsigned char> raw(raw_sz);
        unsigned char* ptr = raw.data();
        *(ptr++) = 'p';
        ptr = integer_to_network(raw_sz - 1, ptr);
        ptr = string_to_network(__M_para.password.begin(),
                                __M_para.password.end(), ptr);
        assert(raw.data() + raw_sz == ptr);
        __M_parsers.template emplace<1>();
        net::async_write_some_exactly(
            __M_conn.stream().lowest_layer(), std::move(raw),
            cntl().template next_with_tag<ev_send_cleartext>());
    }
    void do_md5() {
        return cntl().complete(
            make_ec(errc::sqlclient_unable_to_establish_sqlconnection));
    }
    void do_read_till_ready(const unsigned char* begin,
                            const unsigned char* end) {
        wait_stage_parser& parsers = std::get<2>(__M_parsers);
        while (begin < end) {
            switch (parsers.index()) {
            case 0: {
                // guess stage
                const std::uint8_t type = *begin;  // just peek
                if (type == 'K') {
                    parsers.template emplace<1>();
                } else if (type == 'S') {
                    parsers.template emplace<2>();
                } else if (type == 'N') {
                    parsers.template emplace<3>();
                } else if (type == 'Z') {
                    parsers.template emplace<4>();
                } else if (type == 'E') {
                    parsers.template emplace<5>();
                } else {
                    return cntl().complete(make_ec(errc::malformed_message));
                }
                break;
            }
            case 1: {
                basic_parser<msg::backend_key_data>& p =
                    *std::get_if<1>(&parsers);
                if (ParseResult r = p(begin, end); r == ParseSuccess) {
                    parsers.template emplace<0>();
                    break;
                } else if (r == ParseNeedMore) {
                    break;
                } else {
                    return cntl().complete(make_ec(errc::malformed_message));
                }
            }
            case 2: {
                basic_parser<msg::parameter_status>& p =
                    *std::get_if<2>(&parsers);
                if (ParseResult r = p(begin, end); r == ParseSuccess) {
                    parsers.template emplace<0>();
                    break;
                } else if (r == ParseNeedMore) {
                    break;
                } else {
                    return cntl().complete(make_ec(errc::malformed_message));
                }
            }
            case 3: {
                basic_parser<msg::notice_response>& p =
                    *std::get_if<3>(&parsers);
                if (ParseResult r = p(begin, end); r == ParseSuccess) {
                    parsers.template emplace<0>();
                    break;
                } else if (r == ParseNeedMore) {
                    break;
                } else {
                    return cntl().complete(make_ec(errc::malformed_message));
                }
            }
            case 4: {
                basic_parser<msg::ready_for_query>& p =
                    *std::get_if<4>(&parsers);
                if (ParseResult r = p(begin, end); r == ParseSuccess) {
                    if (begin == end) {
                        return cntl().complete(std::error_code{});
                    } else {
                        return cntl().complete(
                            make_ec(errc::malformed_message));
                    }
                } else if (r == ParseNeedMore) {
                    break;
                } else {
                    return cntl().complete(make_ec(errc::malformed_message));
                }
            }
            case 5: {
                basic_parser<msg::error_response>& p =
                    *std::get_if<5>(&parsers);
                if (ParseResult r = p(begin, end); r == ParseSuccess) {
                    return cntl().complete(make_ec(p.value().ec));
                } else if (r == ParseNeedMore) {
                    break;
                } else {
                    return cntl().complete(make_ec(errc::malformed_message));
                }
            }
            }
        }
        return __M_conn.stream().lowest_layer().async_read_some(
            net::buffer(__M_buf),
            cntl().template next_with_tag<ev_read_till_ready>());
    }
};
}  // namespace chx::sql::postgresql::detail

template <typename Stream>
template <typename CompletionToken>
decltype(auto) chx::sql::postgresql::connection<Stream>::async_connect(
    connect_parameters&& para, CompletionToken&& completion_token) {
    return net::async_combine<const std::error_code&>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<detail::connect_operation<Stream>>{}, *this,
        std::move(para));
}
