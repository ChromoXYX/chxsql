#pragma once

#include "../result.hpp"

#include "../connection.hpp"
#include "./basic_parser.hpp"

#include "./msg/error_response.hpp"
#include "./msg/notice_response.hpp"
#include "./msg/row_description.hpp"
#include "./msg/data_row.hpp"
#include "./msg/bind_complete.hpp"
#include "./msg/no_data.hpp"
#include "./msg/empty_query_response.hpp"
#include "./msg/ready_for_query.hpp"
#include "./msg/command_complete.hpp"
#include "./msg/parse_complete.hpp"

#include "./create_msg.hpp"

#include <chx/net/async_write_some_exactly.hpp>
#include <chx/net/async_combine.hpp>
#include <chx/net/iovec_buffer.hpp>
#include <chx/net/detail/flatten_sequence.hpp>
#include <chx/net/binary_container.hpp>
#include <chx/net/detail/remove_rvalue_reference.hpp>
#include <chx/net/detail/accumulate_size.hpp>
#include <chx/net/async_write_sequence_exactly.hpp>

namespace chx::sql::postgresql::detail {
namespace tags {
struct oneshot {};
}  // namespace tags

template <> struct visitor<tags::oneshot> {
    template <typename Stream, typename CntlType = int> struct operation {
      private:
        struct ev_send {};
        struct ev_read {};

        connection<Stream>& __M_c;

        std::error_code err;
        std::vector<std::optional<result_set>> result;
        std::optional<result_set> __M_curr;

      public:
        template <typename T> using rebind = operation<Stream, T>;
        using cntl_type = CntlType;

        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }

        operation(connection<Stream>* c) : __M_c(*c) {}

        template <typename InitBuffer>
        void operator()(cntl_type& cntl, InitBuffer buffer) {
            do_parse(__M_c.__M_buf_begin, __M_c.__M_buf_end);
            net::async_write_sequence_exactly(
                __M_c.stream(), std::move(buffer),
                cntl.template next_with_tag<ev_send>());
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_send) {
            cntl.complete(e, std::move(result));
        }
        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read) {
            if (!e) {
                do_parse(__M_c.__M_buf_begin = __M_c.__M_buffer.data(),
                         __M_c.__M_buf_end = __M_c.__M_buffer.data() + s);
            } else {
                cntl.complete(e, std::move(result));
            }
        }

        void do_read() {
            __M_c.__M_buf_begin = __M_c.__M_buf_end = nullptr;
            return __M_c.stream().async_read_some(
                net::buffer(__M_c.__M_buffer),
                cntl().template next_with_tag<ev_read>());
        }

        std::variant<std::monostate, basic_parser<msg::error_response>,
                     basic_parser<msg::notice_response>,
                     basic_parser<msg::bind_complete>,
                     basic_parser<msg::row_description>,
                     basic_parser<msg::data_row>, basic_parser<msg::no_data>,
                     basic_parser<msg::ready_for_query>,
                     basic_parser<msg::command_complete>,
                     basic_parser<msg::empty_query_response>,
                     basic_parser<msg::parse_complete>>
            parsers;
        enum {
            ParseComplete,
            BindComplete,
            RowDescOrNoData,
            DataRow,
            EmptyQueryResponse,
            ReadyForQuery
        } expect = ParseComplete;

        void do_parse(const unsigned char*& begin, const unsigned char*& end) {
            while (begin <= end) {
                switch (parsers.index()) {
                case 0: {
                    if (begin == end) {
                        return do_read();
                    }
                    const std::uint32_t type = *begin;
                    if (type == '1') {
                        if (expect == ParseComplete) {
                            parsers
                                .emplace<basic_parser<msg::parse_complete>>();
                            expect = BindComplete;
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    } else if (type == '2') {
                        if (expect == BindComplete) {
                            parsers.emplace<basic_parser<msg::bind_complete>>();
                            expect = RowDescOrNoData;
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    }
                    // begin -- normal query
                    else if (type == 'T') {
                        if (expect == RowDescOrNoData) {
                            parsers
                                .emplace<basic_parser<msg::row_description>>();
                            expect = DataRow;
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    } else if (type == 'D') {
                        if (expect == DataRow) {
                            parsers.emplace<basic_parser<msg::data_row>>(
                                static_cast<std::uint16_t>(
                                    __M_curr->__M_desc.size()));
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    } else if (type == 'C') {
                        if (expect == DataRow) {
                            expect = ReadyForQuery;
                            parsers
                                .emplace<basic_parser<msg::command_complete>>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    }
                    // begin -- NoData
                    else if (type == 'n') {
                        if (expect == RowDescOrNoData) {
                            expect = EmptyQueryResponse;
                            parsers.emplace<basic_parser<msg::no_data>>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    } else if (type == 'I') {
                        if (expect == EmptyQueryResponse) {
                            expect = ReadyForQuery;
                            parsers.emplace<
                                basic_parser<msg::empty_query_response>>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    }
                    // begin -- ready for query
                    else if (type == 'Z') {
                        if (expect == ReadyForQuery) {
                            parsers
                                .emplace<basic_parser<msg::ready_for_query>>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(result));
                        }
                    }
                    // begin -- error or notice
                    else if (type == 'E') {
                        expect = ReadyForQuery;
                        parsers.emplace<basic_parser<msg::error_response>>();
                    } else if (type == 'N') {
                        parsers.emplace<basic_parser<msg::notice_response>>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                    break;
                }
                case 1: {
                    basic_parser<msg::error_response>& parser =
                        *std::get_if<1>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        err = make_ec(parser.result.ec);
                        parsers.template emplace<std::monostate>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 2: {
                    basic_parser<msg::notice_response>& parser =
                        *std::get_if<2>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 3: {
                    basic_parser<msg::bind_complete>& parser =
                        *std::get_if<3>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 4: {
                    basic_parser<msg::row_description>& parser =
                        *std::get_if<4>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_curr.emplace().__M_desc =
                            std::move(parser.result.desc);
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 5: {
                    basic_parser<msg::data_row>& parser =
                        *std::get_if<5>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_curr->__M_data.emplace_back(
                            std::move(parser.result.data));
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 6: {
                    basic_parser<msg::no_data>& parser =
                        *std::get_if<6>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 7: {
                    basic_parser<msg::ready_for_query>& parser =
                        *std::get_if<7>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        return cntl().complete(err, std::move(result));
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 8: {
                    basic_parser<msg::command_complete>& parser =
                        *std::get_if<8>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        result.emplace_back(std::move(__M_curr));
                        __M_curr.reset();
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 9: {
                    basic_parser<msg::empty_query_response>& parser =
                        *std::get_if<9>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
                    }
                }
                case 10: {
                    basic_parser<msg::parse_complete>& parser =
                        *std::get_if<10>(&parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return do_read();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(result));
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
template <typename... Parameters, typename CompletionToken>
decltype(auto) chx::sql::postgresql::connection<Stream>::async_execute_oneshot(
    std::string_view sql, std::tuple<Parameters...> params,
    CompletionToken&& completion_token) {
    auto msg = std::tuple(detail::create_parse_msg(sql),
                          detail::create_bind_msg({}, std::move(params)),
                          detail::create_DES_msg());
    return net::async_combine_ng<const std::error_code&, result_sets>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<
            detail::visitor<detail::tags::oneshot>::operation<Stream>>{},
        std::tuple(this), net::enable_reference_count, std::move(msg));
}