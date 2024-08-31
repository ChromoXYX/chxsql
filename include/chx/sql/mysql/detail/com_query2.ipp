#pragma once

/*
24.08.27
new com_query impl. all handmade, no chxser2, because chxser2 cannot work with
streaming :(
*/

#include "../error_code.hpp"
#include "../connection.hpp"
#include "./basic_packet.hpp"
#include "./fixed_integer.hpp"
#include "./packets/ERR_Packet.hpp"
#include "./packets/EOF_Packet.hpp"
#include "./packets/OK_Packet.hpp"
#include "./packets/ColumnDefinition41_Packet.hpp"
#include "./packets/ColumnCount_Packet.hpp"
#include "./packets/Resultset_Packet.hpp"
#include "../result_set.hpp"

#include <chx/net/async_write_some_exactly.hpp>

namespace chx::sql::mysql::detail {
namespace tags {
struct com_query2 {};
}  // namespace tags

template <> struct visitor<tags::com_query2> {
    template <typename Stream, typename CntlType = int>
    struct operation : private visitor<tags::basic_packet>::operation<
                           operation<Stream, CntlType>> {
      private:
        struct ev_send_query {};
        struct ev_read {};

        using PayloadResult = packets2::PayloadResult;
        using PacketResult = packets2::PacketResult;
        using packet_parser_type =
            visitor<tags::basic_packet>::operation<operation>;
        friend packet_parser_type;

      public:
        template <typename T> using rebind = operation<Stream, T>;
        using cntl_type = CntlType;

        constexpr operation(connection<Stream>& c, std::string&& query)
            : conn(c), sql_query(std::move(query)) {}

        void operator()(cntl_type&) {
            std::vector<unsigned char> buffer(sql_query.size() + 5);
            fixed_length_integer_to_network<3>(sql_query.size() + 1,
                                               buffer.begin());
            buffer[3] = next_sequence_id++;
            buffer[4] = 0x03;
            std::copy(sql_query.begin(), sql_query.end(), buffer.begin() + 5);
            net::async_write_some_exactly(
                conn.stream(), std::move(buffer),
                cntl().template next_with_tag<ev_send_query>());
        }

        void operator()(cntl_type&, const std::error_code& e, std::size_t s,
                        ev_send_query) {
            if (!e) {
                __M_buf.resize(256);
                conn.stream().lowest_layer().async_read_some(
                    net::buffer(__M_buf),
                    cntl().template next_with_tag<ev_read>());
            } else {
                cntl().complete(e, std::move(__M_result_set));
            }
        }

        void operator()(cntl_type&, const std::error_code& e, std::size_t s,
                        ev_read) {
            if (!e) {
                const unsigned char* begin = __M_buf.data();
                PacketResult r = packet_parser_type::feed(begin, begin + s);
                switch (r) {
                case packets2::PacketPaused: {
                    if (begin == __M_buf.data() + s) {
                        return cntl().complete(__M_ec,
                                               std::move(__M_result_set));
                    } else {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(__M_result_set));
                    }
                }
                case packets2::PacketNeedMore: {
                    assert(begin == __M_buf.data() + s);
                    return conn.stream().lowest_layer().async_read_some(
                        net::buffer(__M_buf),
                        cntl().template next_with_tag<ev_read>());
                }
                case packets2::PacketIncomplete:
                case packets2::PacketTrailingData:
                case packets2::PacketMalformed:
                    return cntl().complete(make_ec(errc::CR_MALFORMED_PACKET),
                                           std::move(__M_result_set));
                case packets2::PacketInternalError:
                    return cntl().complete(
                        __M_ec ? __M_ec : make_ec(errc::CR_UNKNOWN_ERROR),
                        std::move(__M_result_set));
                default:
                    assert(false);
                }
            } else {
                cntl().complete(e, std::move(__M_result_set));
            }
        }

      private:
        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }
        connection<Stream>& conn;
        std::string sql_query;
        std::uint8_t next_sequence_id = 0;

        enum Stage {
            InitialGuess,
            S1_ERR,
            S1_OK,
            S2_ColumnCount,
            S2_ColnumDef,
            S2_ResultSetRowGuess,
            S2_ResultSet,
            S2_ERR,
            S2_OK_EOF,
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
        PayloadResult on_payload_data(const unsigned char* begin,
                                      const unsigned char* end) {
            using namespace packets2;
            switch (stage) {
            case InitialGuess: {
                if (begin == end) {  // need more
                    return PayloadNeedMore;
                }
                const std::uint8_t first_byte = *begin;
                if (first_byte == 0xff) {
                    emplace_err_packet_parser();
                    stage = S1_ERR;
                } else if (first_byte == 0x00) {
                    emplace_ok_packet_parser();
                    stage = S1_OK;
                } else {
                    emplace_column_count_packet_parser();
                    stage = S2_ColumnCount;
                }
                return on_payload_data(begin, end);
            }
            case S1_ERR: {
                ERR_Packet::parser& parser = err_packet_parser();
                if (PayloadResult r = parser(
                        begin, end, packet_parser_type::remain_bytes() == 0);
                    r == PayloadSuccess) {
                    ERR_Packet packet = parser.value();
                    __M_ec = make_ec(packet.error_code);
                    parser = {};
                    stage = Finished;
                    return make_pause(begin, end);
                } else {
                    return r;
                }
            }
            case S1_OK: {
                OK_Packet::parser<>& parser = ok_packet_parser();
                if (PayloadResult r =
                        parser(conn.client_cap(), begin, end,
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
            case S2_ColumnCount: {
                ColumnCount_Packet::parser& parser =
                    column_count_packet_parser();
                if (PayloadResult r = parser(begin, end); r == PayloadSuccess) {
                    __M_column_count = parser.value().column_count;
                    parser = {};

                    emplace_column_definition_packet_parser();
                    stage = S2_ColnumDef;
                    return make_success(begin, end);
                } else {
                    return r;
                }
            }
            case S2_ColnumDef: {
                if (__M_result_set.__M_def.size() < __M_column_count) {
                    ColumnDefinition41_Packet::parser& parser =
                        column_definition_packet_parser();
                    if (PayloadResult r = parser(begin, end);
                        r == PayloadSuccess) {
                        __M_result_set.__M_def.emplace_back(
                            parser.value().name);
                        parser = {};
                        return make_success(begin, end);
                    } else {
                        return r;
                    }
                } else {
                    emplace_resultset_packet_parser();
                    stage = S2_ResultSetRowGuess;
                }
            }
            case S2_ResultSetRowGuess: {
                if (packet_parser_type::remain_bytes() +
                        std::distance(begin, end) !=
                    packet_parser_type::payload_length()) {
                    return PayloadInternalError;
                }
                if (begin == end) {
                    return PayloadNeedMore;
                }
                const std::uint8_t first_byte = *begin;
                if (first_byte == 0xff) {
                    emplace_err_packet_parser();
                    stage = S2_ERR;
                } else if (first_byte == 0xfe &&
                           packet_parser_type::payload_length() < 0xffffff) {
                    emplace_ok_packet_eof_parser();
                    stage = S2_OK_EOF;
                } else {
                    emplace_resultset_packet_parser();
                    stage = S2_ResultSet;
                }
                return on_payload_data(begin, end);
            }
            case S2_ERR: {
                ERR_Packet::parser& parser = err_packet_parser();
                if (PayloadResult r = parser(
                        begin, end, packet_parser_type::remain_bytes() == 0);
                    r == PayloadSuccess) {
                    ERR_Packet packet = parser.value();
                    __M_ec = make_ec(packet.error_code);
                    parser = {};
                    stage = Finished;
                    return make_pause(begin, end);
                } else {
                    return r;
                }
            }
            case S2_OK_EOF: {
                OK_Packet::parser<0xfe>& parser = ok_packet_eof_parser();
                if (PayloadResult r =
                        parser(conn.client_cap(), begin, end,
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
            case S2_ResultSet: {
                Resultset_Packet::parser& parser = resultset_packet_parser();
                if (PayloadResult r = parser(__M_column_count, begin, end);
                    r == PayloadSuccess) {
                    __M_result_set.__M_result.emplace_back(
                        std::move(parser.value().column_data));
                    parser = {};
                    stage = S2_ResultSetRowGuess;
                    return make_success(begin, end);
                } else {
                    return r;
                }
            }
            default:
                assert(false);
            }
        }

        std::variant<packets2::ERR_Packet::parser, packets2::EOF_Packet::parser,
                     packets2::OK_Packet::parser<>,
                     packets2::ColumnCount_Packet::parser,
                     packets2::ColumnDefinition41_Packet::parser,
                     packets2::Resultset_Packet::parser,
                     packets2::OK_Packet::parser<0xfe>>
            __M_payload_parser = {};
        std::size_t __M_column_count = 0;
        result_set __M_result_set;

        constexpr auto& err_packet_parser() {
            return std::get<0>(__M_payload_parser);
        }
        constexpr auto& eof_packet_parser() {
            return std::get<1>(__M_payload_parser);
        }
        constexpr auto& ok_packet_parser() {
            return std::get<2>(__M_payload_parser);
        }
        constexpr auto& column_count_packet_parser() {
            return std::get<3>(__M_payload_parser);
        }
        constexpr auto& column_definition_packet_parser() {
            return std::get<4>(__M_payload_parser);
        }
        constexpr auto& resultset_packet_parser() {
            return std::get<5>(__M_payload_parser);
        }
        constexpr auto& ok_packet_eof_parser() {
            return std::get<6>(__M_payload_parser);
        }

        constexpr void emplace_err_packet_parser() {
            __M_payload_parser.template emplace<0>();
        }
        constexpr void emplace_eof_packet_parser() {
            __M_payload_parser.template emplace<1>();
        }
        constexpr void emplace_ok_packet_parser() {
            __M_payload_parser.template emplace<2>();
        }
        constexpr void emplace_column_count_packet_parser() {
            __M_payload_parser.template emplace<3>();
        }
        constexpr void emplace_column_definition_packet_parser() {
            __M_payload_parser.template emplace<4>();
        }
        constexpr void emplace_resultset_packet_parser() {
            __M_payload_parser.template emplace<5>();
        }
        constexpr void emplace_ok_packet_eof_parser() {
            __M_payload_parser.template emplace<6>();
        }

        std::vector<unsigned char> __M_buf;
        std::error_code __M_ec = {};
    };
};
}  // namespace chx::sql::mysql::detail

template <typename Stream>
template <typename CompletionToken>
decltype(auto) chx::sql::mysql::connection<Stream>::async_query(
    std::string&& query, CompletionToken&& completion_token) {
    using operation =
        decltype(detail::visitor<detail::tags::com_query2>::operation(
            *this, std::move(query)));
    return net::async_combine_reference_count<const std::error_code&,
                                              result_set>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<operation>{}, *this, std::move(query));
}
