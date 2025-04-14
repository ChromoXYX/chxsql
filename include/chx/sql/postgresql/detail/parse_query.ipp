#pragma once

#include "../connection.hpp"
#include "./basic_type.hpp"
#include "./basic_parser.hpp"
#include "./msg/error_response.hpp"
#include "./msg/notice_response.hpp"
#include "./msg/ready_for_query.hpp"

#include "./msg/parse_complete.hpp"

#include <chx/net/async_write_some_exactly.hpp>
#include <chx/net/async_combine.hpp>

namespace chx::sql::postgresql::detail {
namespace tags {
struct parse_query {};
}  // namespace tags

template <> struct visitor<tags::parse_query> {
    template <typename Stream, typename CntlType = int> struct operation {
      private:
        struct ev_send {};
        struct ev_read {};

        connection<Stream>& __M_conn;
        const std::string __M_pattern_name;
        const std::string __M_pattern;

      public:
        template <typename T> using rebind = operation<Stream, T>;
        using cntl_type = CntlType;

        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }

        operation(connection<Stream>& c, std::string&& pattern_name,
                  std::string&& pattern)
            : __M_conn(c), __M_pattern_name(std::move(pattern_name)),
              __M_pattern(std::move(pattern)) {}

        void operator()(cntl_type& cntl) {
            const std::uint32_t p_len =
                5 + __M_pattern_name.size() + __M_pattern.size() + 2 + 2;
            const std::uint32_t raw_len = p_len + 5;
            std::vector<unsigned char> raw(raw_len);
            unsigned char* ptr = raw.data();
            *(ptr++) = 'P';
            ptr = integer_to_network(p_len - 1, ptr);
            ptr = string_to_network(__M_pattern_name.begin(),
                                    __M_pattern_name.end(), ptr);
            ptr =
                string_to_network(__M_pattern.begin(), __M_pattern.end(), ptr);
            ptr += 2;

            *(ptr++) = 'S';
            ptr = integer_to_network(4, ptr);
            assert(ptr == raw.data() + raw.size());
            // number of parameter data remains 0
            net::async_write_some_exactly(
                __M_conn.stream(), std::move(raw),
                cntl.template next_with_tag<ev_send>());
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read) {
            if (!e) {
                do_parse(__M_conn.__M_buf_begin = __M_conn.__M_buffer.data(),
                         __M_conn.__M_buf_end = __M_conn.__M_buffer.data() + s);
            } else {
                cntl.complete(e);
            }
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_send) {
            if (!e) {
                do_parse(__M_conn.__M_buf_begin, __M_conn.__M_buf_end);
            } else {
                cntl.complete(e);
            }
        }

        std::variant<std::monostate, basic_parser<msg::parse_complete>,
                     basic_parser<msg::error_response>,
                     basic_parser<msg::notice_response>,
                     basic_parser<msg::ready_for_query>>
            parsers;

        enum Stage {
            WaitForParseComplete,
            WaitForReadyForQuery
        } stage = WaitForParseComplete;

        void do_read() {
            __M_conn.__M_buf_begin = __M_conn.__M_buf_end = nullptr;
            return __M_conn.stream().async_read_some(
                net::buffer(__M_conn.__M_buffer),
                cntl().template next_with_tag<ev_read>());
        }

        void do_parse(const unsigned char*& begin, const unsigned char*& end) {
            while (begin <= end) {
                switch (parsers.index()) {
                case 0: {
                    if (begin == end) {
                        return do_read();
                    }
                    const std::uint32_t type = *begin;
                    if (type == '1') {
                        if (stage == WaitForParseComplete) {
                            parsers.template emplace<1>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message));
                        }
                    } else if (type == 'E') {
                        parsers.template emplace<2>();
                    } else if (type == 'N') {
                        parsers.template emplace<3>();
                    } else if (type == 'Z') {
                        if (stage == WaitForReadyForQuery) {
                            parsers.template emplace<4>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message));
                        }
                    } else {
                        return cntl().complete(
                            make_ec(errc::malformed_message));
                    }
                    break;
                }
                case 1: {
                    basic_parser<msg::parse_complete>& parser =
                        *std::get_if<1>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        parsers.template emplace<0>();
                        stage = WaitForReadyForQuery;
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(
                            make_ec(errc::malformed_message));
                    }
                }
                case 2: {
                    basic_parser<msg::error_response>& parser =
                        *std::get_if<2>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        return cntl().complete(make_ec(parser.result.ec));
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(
                            make_ec(errc::malformed_message));
                    }
                }
                case 3: {
                    basic_parser<msg::notice_response>& parser =
                        *std::get_if<3>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(
                            make_ec(errc::malformed_message));
                    }
                }
                case 4: {
                    basic_parser<msg::ready_for_query>& parser =
                        *std::get_if<4>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        return cntl().complete(std::error_code{});
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(
                            make_ec(errc::malformed_message));
                    }
                }
                default:
                    assert(false);
                }
            }
        }
    };
};
}  // namespace chx::sql::postgresql::detail

template <typename Stream>
template <typename CompletionToken>
decltype(auto) chx::sql::postgresql::connection<Stream>::async_parse(
    std::string prepared_statement_name, std::string prepared_statement,
    CompletionToken&& completion_token) {
    return net::async_combine<const std::error_code&>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<
            detail::visitor<detail::tags::parse_query>::operation<Stream>>{},
        *this, std::move(prepared_statement_name),
        std::move(prepared_statement));
}
