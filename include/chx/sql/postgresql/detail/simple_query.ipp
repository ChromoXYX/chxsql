#pragma once

#include "../connection.hpp"
#include "./basic_type.hpp"
#include "./msg/row_description.hpp"
#include "./msg/data_row.hpp"
#include "./msg/empty_query_response.hpp"
#include "./msg/command_complete.hpp"

#include <chx/net/async_write_some_exactly.hpp>
#include <chx/net/async_combine.hpp>

namespace chx::sql::postgresql::detail {
namespace tags {
struct simple_query {};
}  // namespace tags

template <> struct visitor<tags::simple_query> {
    template <typename Stream, typename CntlType = int> struct operation {
      private:
        struct ev_send_query {};
        struct ev_read_till_ready {};

      public:
        operation(connection<Stream>& c, std::string&& q)
            : __M_conn(c), __M_query(std::move(q)) {}

        template <typename T> using rebind = operation<Stream, T>;
        using cntl_type = CntlType;

        void operator()(cntl_type& cntl) {
            const std::uint32_t raw_sz = 5 + __M_query.size() + 1;
            std::vector<unsigned char> raw(raw_sz);
            unsigned char* ptr = raw.data();
            *(ptr++) = 'Q';
            ptr = integer_to_network(raw_sz - 1, ptr);
            ptr = string_to_network(__M_query.begin(), __M_query.end(), ptr);
            assert(ptr == raw.data() + raw.size());
            net::async_write_some_exactly(
                __M_conn.stream().lowest_layer(), std::move(raw),
                cntl.template next_with_tag<ev_send_query>());
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_send_query) {
            if (!e) {
                do_read_till_ready(buf_begin(), buf_end());
            } else {
                cntl.complete(e, std::move(__M_result));
            }
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_read_till_ready) {
            if (!e) {
                do_read_till_ready(buf_begin() = buf().data(),
                                   buf_end() = buf().data() + s);
            } else {
                cntl.complete(e, std::move(__M_result));
            }
        }

      private:
        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }
        connection<Stream>& __M_conn;
        std::string __M_query;
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

        std::variant<std::monostate, basic_parser<msg::data_row>,
                     basic_parser<msg::row_description>,
                     basic_parser<msg::empty_query_response>,
                     basic_parser<msg::command_complete>,
                     basic_parser<msg::ready_for_query>,
                     basic_parser<msg::error_response>,
                     basic_parser<msg::notice_response>,
                     basic_parser<msg::parameter_status>>
            __M_parsers;
        std::vector<std::optional<result_set>> __M_result;
        std::optional<result_set> __M_curr;
        enum class __CurrRespType : std::uint8_t {
            Unknown,
            WithDataNoDef,
            WithDataWithDef,
            Empty
        } __M_curr_type = __CurrRespType::Unknown;

        void do_read_till_ready(const unsigned char*& begin,
                                const unsigned char*& end) {
            while (begin <= end) {
                switch (__M_parsers.index()) {
                case 0: {
                    if (begin == end) {
                        return perform_read<ev_read_till_ready>();
                    }
                    const std::uint32_t type = *begin;
                    if (type == 'D') {
                        if (__M_curr_type == __CurrRespType::WithDataWithDef) {
                            __M_parsers.template emplace<1>(
                                msg::data_row{static_cast<std::uint16_t>(
                                    __M_curr->__M_def.size())});
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(__M_result));
                        }
                    } else if (type == 'T') {
                        if (__M_curr_type == __CurrRespType::Unknown) {
                            __M_curr_type = __CurrRespType::WithDataNoDef;
                            __M_curr.emplace(result_set{});
                            __M_parsers.template emplace<2>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(__M_result));
                        }
                    } else if (type == 'I') {
                        if (__M_curr_type == __CurrRespType::Unknown) {
                            __M_curr_type = __CurrRespType::Empty;
                            __M_curr.reset();
                            __M_parsers.template emplace<3>();
                        } else {
                            return cntl().complete(
                                make_ec(errc::malformed_message),
                                std::move(__M_result));
                        }
                    } else if (type == 'C') {
                        __M_parsers.template emplace<4>();
                    } else if (type == 'Z') {
                        __M_parsers.template emplace<5>();
                    } else if (type == 'E') {
                        __M_parsers.template emplace<6>();
                    } else if (type == 'N') {
                        __M_parsers.template emplace<7>();
                    } else if (type == 'S') {
                        __M_parsers.template emplace<8>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                    break;
                }
                case 1: {
                    basic_parser<msg::data_row>& parser =
                        *std::get_if<1>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_curr->__M_result.emplace_back(
                            std::move(parser.result.data));
                        __M_parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                }
                case 2: {
                    basic_parser<msg::row_description>& parser =
                        *std::get_if<2>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_curr->__M_def = std::move(parser.result.desc);
                        __M_curr_type = __CurrRespType::WithDataWithDef;
                        __M_parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                }
                case 3: {
                    basic_parser<msg::empty_query_response>& parser =
                        *std::get_if<3>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                }
                case 4: {
                    basic_parser<msg::command_complete>& parser =
                        *std::get_if<4>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_result.emplace_back(std::move(__M_curr));
                        __M_curr.reset();
                        __M_curr_type = __CurrRespType::Unknown;
                        __M_parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                }
                case 5: {
                    basic_parser<msg::ready_for_query>& parser =
                        *std::get_if<5>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        return cntl().complete(std::error_code{},
                                               std::move(__M_result));
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                }
                case 6: {
                    basic_parser<msg::error_response>& parser =
                        *std::get_if<6>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        return cntl().complete(make_ec(parser.result.ec),
                                               std::move(__M_result));
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                }
                case 7: {
                    basic_parser<msg::notice_response>& parser =
                        *std::get_if<7>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
                    }
                }
                case 8: {
                    basic_parser<msg::parameter_status>& parser =
                        *std::get_if<8>(&__M_parsers);
                    ParseResult r = parser(begin, end);
                    if (r == ParseSuccess) {
                        __M_parsers.template emplace<0>();
                        break;
                    } else if (r == ParseNeedMore) {
                        return perform_read<ev_read_till_ready>();
                    } else {
                        return cntl().complete(make_ec(errc::malformed_message),
                                               std::move(__M_result));
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
decltype(auto) chx::sql::postgresql::connection<Stream>::async_simple_query(
    std::string&& query, CompletionToken&& completion_token) {
    return net::async_combine<const std::error_code&,
                              std::vector<std::optional<result_set>>>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<
            detail::visitor<detail::tags::simple_query>::operation<Stream>>{},
        *this, std::move(query));
}
