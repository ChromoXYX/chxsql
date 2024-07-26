#pragma once

#define CHXSQL_MYSQL_PACKET_BUFFER_SIZE 5

#include "../connection.hpp"
#include "./packets/COM_Query.hpp"
#include "./packets/ColumnDefinition41.hpp"

#include <chx/net/async_write_sequence_exactly.hpp>

namespace chx::sql::mysql::detail {
namespace tags {
struct com_query {};
}  // namespace tags

template <> struct visitor<tags::com_query> {
    template <typename Stream, typename DataMapper, typename CntlType = int>
    struct operation : private DataMapper {
        static_assert(CHXSQL_MYSQL_PACKET_BUFFER_SIZE < 0xffffff + 4);

        template <typename T> using rebind = operation<Stream, DataMapper, T>;
        using cntl_type = CntlType;

        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }

        connection<Stream>& c;
        std::string q;
        std::uint8_t next_sequence_id = 0;
        std::vector<packets::ColumnDefinition41> column_def;
        typename DataMapper::result_set_type result_set;

        std::vector<std::vector<unsigned char>> __M_buf;
        std::size_t __M_buf_sz = 0;
        std::size_t /*__M_packet_buf = 0,*/ __M_packet_begin = 0;
        using pending_packet_data = std::vector<std::vector<unsigned char>>;
        std::vector<pending_packet_data> __M_pending_packets;

        template <typename Strm, typename DM>
        operation(connection<Strm>& conn, DM&& dm, std::string query)
            : DataMapper(std::forward<DM>(dm)), c(conn), q(std::move(query)) {}

        struct ev_com_query_send {};
        struct ev_com_query_recv_init {};
        struct ev_recv {};

        void operator()(cntl_type&) {
            net::async_write_sequence_exactly(
                c.stream(),
                std::make_tuple(
                    packets::COM_Query::construct_header(next_sequence_id, q),
                    std::move(q)),
                cntl().template next_with_tag<ev_com_query_send>());
        }

        void operator()(cntl_type&, const std::error_code& e, std::size_t s,
                        ev_com_query_send) {
            if (!e) {
                __M_buf.resize(1);
                __M_buf[0].resize(CHXSQL_MYSQL_PACKET_BUFFER_SIZE);
                __M_buf_sz = 0;
                __M_packet_begin = 0;
                do_read();
            } else {
                cntl().complete(e, std::move(result_set));
            }
        }

        void operator()(cntl_type&, const std::error_code& e, std::size_t s,
                        ev_recv) {
            if (!e) {
                __M_buf_sz += s;
                return run();
            } else {
                cntl().complete(e, std::move(result_set));
            }
        }

      private:
        void run() {
            std::uint32_t packet_length = 0;
            std::uint8_t sequence_id = 0;
            std::size_t avail_size = 0;
            if (__M_buf.size() == 1) {
                // packet header in current buffer
                if (__M_buf_sz >= __M_packet_begin + 4) {
                    packet_length =
                        fixed_length_integer_from_network<3>(
                            __M_buf.back().data() + __M_packet_begin) +
                        4;
                    if (packet_length == 0) {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(result_set));
                    }
                    sequence_id = fixed_length_integer_from_network<1>(
                        __M_buf.back().data() + __M_packet_begin + 3);
                    if (int r = pre_check(packet_length, sequence_id);
                        r != errc::NO_ERROR) {
                        return cntl().complete(make_ec(r),
                                               std::move(result_set));
                    }
                    avail_size = __M_buf_sz - __M_packet_begin;
                } else {
                    // need more
                    return do_read();
                }
            } else {
                // packet header in old buffer
                if (__M_packet_begin <= CHXSQL_MYSQL_PACKET_BUFFER_SIZE - 4) {
                    // packet header in the same buffer
                    assert(__M_buf.size() >= 2);
                    packet_length = fixed_length_integer_from_network<3>(
                                        __M_buf[0].data() + __M_packet_begin) +
                                    4;
                    if (packet_length == 0) {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(result_set));
                    }
                    sequence_id = fixed_length_integer_from_network<1>(
                        __M_buf[0].data() + __M_packet_begin + 3);
                    if (int r = pre_check(packet_length, sequence_id);
                        r != errc::NO_ERROR) {
                        return cntl().complete(make_ec(r),
                                               std::move(result_set));
                    }
                    avail_size =
                        CHXSQL_MYSQL_PACKET_BUFFER_SIZE - __M_packet_begin +
                        (__M_buf.size() - 2) * CHXSQL_MYSQL_PACKET_BUFFER_SIZE +
                        __M_buf_sz;
                } else {
                    unsigned char buffer[4] = {};
                    const std::size_t part_a =
                        CHXSQL_MYSQL_PACKET_BUFFER_SIZE - __M_packet_begin;
                    const std::size_t part_b = 4 - part_a;
                    if (2 < __M_buf.size() || __M_buf_sz >= part_b) {
                        // part_b still in old buffer, or
                        // current buffer bigger than part b (while packet
                        // header begins in an old buffer)
                        std::memcpy(buffer,
                                    __M_buf[0].data() + __M_packet_begin,
                                    part_a);
                        std::memcpy(buffer + part_a, __M_buf[1].data(), part_b);
                        packet_length =
                            fixed_length_integer_from_network<3>(buffer) + 4;
                        if (packet_length == 4) {
                            return cntl().complete(
                                make_ec(errc::CR_MALFORMED_PACKET),
                                std::move(result_set));
                        }
                        sequence_id = buffer[3];
                        if (int r = pre_check(packet_length, sequence_id);
                            r != errc::NO_ERROR) {
                            return cntl().complete(make_ec(r),
                                                   std::move(result_set));
                        }
                        std::size_t part_b_and_avail = __M_buf_sz;
                        if (2 < __M_buf.size()) {
                            part_b_and_avail +=
                                CHXSQL_MYSQL_PACKET_BUFFER_SIZE *
                                (__M_buf.size() - 2);
                        }
                        avail_size = part_b_and_avail + part_a;
                    } else {
                        // need more
                        return do_read();
                    }
                }
            }
            if (avail_size < packet_length) {
                // need more
                return do_read();
            }
            // now, the whole packet is in buffer
            // currently ranges(concat vector to a single range) is not used,
            // just in a real vector
            if (sequence_id != next_sequence_id++) {
                return cntl().complete(make_ec(errc::CR_MALFORMED_PACKET),
                                       std::move(result_set));
            }
            if (__M_buf.size() == 1) {
                assert(__M_buf_sz >= __M_packet_begin + packet_length);
                auto packet_begin = __M_buf.back().data() + __M_packet_begin,
                     packet_end = __M_buf.back().data() + __M_packet_begin +
                                  packet_length;
                if ((__M_packet_begin += packet_length) ==
                    CHXSQL_MYSQL_PACKET_BUFFER_SIZE) {
                    __M_packet_begin = 0;
                    __M_buf_sz = 0;
                }
                handle_packet(packet_begin, packet_end, packet_length);
            } else {
                // header in an old buffer
                if (packet_length < 0xffffff + 4) {
                    std::vector<unsigned char> c(packet_length);
                    auto pos = c.begin();
                    for_each_buffer(
                        [&](auto&, const auto& begin, const auto& end) {
                            assert(std::distance(begin, end) <=
                                   std::distance(pos, c.end()));
                            pos = std::copy(begin, end, pos);
                        },
                        packet_length);
                    handle_packet(c.begin(), c.end(), packet_length);
                } else {
                    return cntl().complete(
                        make_ec(errc::CR_NET_PACKET_TOO_LARGE),
                        std::move(result_set));
                    __M_pending_packets.emplace_back();
                    for_each_buffer(
                        [&](std::vector<unsigned char>& v, const auto& begin,
                            const auto& end) {
                            if (begin == v.begin() && end == v.end()) {
                                __M_pending_packets.back().emplace_back(
                                    std::move(v));
                            } else {
                                __M_pending_packets.back().emplace_back(begin,
                                                                        end);
                            }
                        },
                        packet_length);
                    return run();
                }
            }
        }
        void do_read() {
            assert(__M_buf.size() > 0 &&
                   __M_buf[0].size() == CHXSQL_MYSQL_PACKET_BUFFER_SIZE);
            if (__M_buf_sz < CHXSQL_MYSQL_PACKET_BUFFER_SIZE) {
                c.stream().async_read_some(
                    net::mutable_buffer(__M_buf.back().data() + __M_buf_sz,
                                        CHXSQL_MYSQL_PACKET_BUFFER_SIZE -
                                            __M_buf_sz),
                    cntl().template next_with_tag<ev_recv>());
            } else {
                __M_buf.emplace_back().resize(CHXSQL_MYSQL_PACKET_BUFFER_SIZE);
                __M_buf_sz = 0;
                c.stream().async_read_some(
                    net::buffer(__M_buf.back()),
                    cntl().template next_with_tag<ev_recv>());
            }
        }

        enum NextPacket {
            OK_ERR_or_FieldCount,
            FieldMetadata,
            RowData
        } nxtp = OK_ERR_or_FieldCount;
        union {
            std::size_t column_count;
        } result_set_status;

        int pre_check(std::uint32_t packet_length, std::uint8_t sequence_id) {
            if (sequence_id != next_sequence_id) {
                return errc::CR_MALFORMED_PACKET;
            }
            return errc::NO_ERROR;
        }
        template <typename RandomAccessIterator>
        void handle_packet(const RandomAccessIterator& begin,
                           const RandomAccessIterator& end,
                           std::uint32_t packet_length) {
            handle_payload(begin + 4, end, packet_length - 4);
        }

        template <typename Iterator>
        void handle_payload(Iterator begin, Iterator end,
                            std::uint32_t payload_length) {
            auto payload_begin = begin;

            switch (nxtp) {
            case OK_ERR_or_FieldCount: {
                std::uint8_t flag = *payload_begin;
                if (flag == 0x00) {
                    packets::OK_Packet ok_packet;
                    auto&& rule = ok_packet.template rule<0x00>(c.client_cap());
                    if (ser2::ParseResult r =
                            rule.parse(ok_packet, payload_begin, end);
                        r == ser2::ParseResult::Ok && payload_begin == end) {
                        return cntl().complete(make_ec(errc::NO_ERROR),
                                               std::move(result_set));
                    } else {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(result_set));
                    }
                } else if (flag == 0xff) {
                    packets::ERR_Packet err_packet;
                    auto&& rule = err_packet.rule(c.client_cap());
                    if (ser2::ParseResult r =
                            rule.parse(err_packet, payload_begin, end);
                        r == ser2::ParseResult::Ok && payload_begin == end) {
                        return cntl().complete(make_ec(errc::NO_ERROR),
                                               std::move(result_set));
                    } else {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(result_set));
                    }
                } else {
                    if (ser2::ParseResult r =
                            ser2::rule(
                                ser2::bind(rules::length_encoded_integer{}))
                                .parse(result_set_status.column_count,
                                       payload_begin, end);
                        r == ser2::ParseResult::Ok && payload_begin == end) {
                        nxtp = FieldMetadata;
                        return run();
                    } else {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(result_set));
                    }
                }
            }
            case FieldMetadata: {
                packets::ColumnDefinition41 field_metadata;
                auto&& rule = field_metadata.rule();
                if (ser2::ParseResult r =
                        rule.parse(field_metadata, payload_begin, end);
                    r == ser2::ParseResult::Ok && payload_begin == end) {
                    if (--result_set_status.column_count == 0) {
                        nxtp = RowData;
                    }
                    column_def.emplace_back(std::move(field_metadata));
                    return run();
                } else {
                    return cntl().complete(make_ec(errc::CR_MALFORMED_PACKET),
                                           std::move(result_set));
                }
            }
            case RowData: {
                std::uint8_t first_byte = *payload_begin;
                if (first_byte == 0xfe && payload_length < 0xffffff) {
                    packets::OK_Packet ok_packet = {};
                    auto&& rule = ok_packet.template rule<0xfe>(c.client_cap());
                    if (ser2::ParseResult r =
                            rule.parse(ok_packet, payload_begin, end);
                        r == ser2::ParseResult::Ok && payload_begin == end) {
                        return cntl().complete(make_ec(errc::NO_ERROR),
                                               std::move(result_set));
                    } else {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(result_set));
                    }
                } else if (first_byte == 0xff) {
                    packets::ERR_Packet err_packet = {};
                    auto&& rule = err_packet.rule(c.client_cap());
                    if (ser2::ParseResult r =
                            rule.parse(err_packet, payload_begin, end);
                        r == ser2::ParseResult::Ok && payload_begin == end) {
                        return cntl().complete(make_ec(err_packet.error_code),
                                               std::move(result_set));
                    } else {
                        return cntl().complete(
                            make_ec(errc::CR_MALFORMED_PACKET),
                            std::move(result_set));
                    }
                } else {
                    // row may be way too long
                    std::error_code ec = DataMapper::operator()(
                        column_def, result_set, payload_begin, end);
                    if (!ec) {
                        return run();
                    } else {
                        return cntl().complete(ec, std::move(result_set));
                    }
                }
            }
            default: {
                assert(false);
            }
            }
        }

        template <typename Fn>
        void for_each_buffer(Fn&& fn, std::uint32_t packet_length) {
            assert(packet_length + __M_packet_begin >
                       CHXSQL_MYSQL_PACKET_BUFFER_SIZE &&
                   __M_buf.size() > 1);
            std::size_t inbuf_sz =
                CHXSQL_MYSQL_PACKET_BUFFER_SIZE - __M_packet_begin;
            assert(packet_length > inbuf_sz);
            std::forward<Fn>(fn)(__M_buf[0],
                                 __M_buf[0].begin() + __M_packet_begin,
                                 __M_buf[0].end());
            __M_buf.erase(__M_buf.begin());
            while (__M_buf.size() > 1) {
                assert(inbuf_sz + CHXSQL_MYSQL_PACKET_BUFFER_SIZE <
                       packet_length);
                std::forward<Fn>(fn)(__M_buf[0], __M_buf[0].begin(),
                                     __M_buf[0].end());
                __M_buf.erase(__M_buf.begin());
                inbuf_sz += CHXSQL_MYSQL_PACKET_BUFFER_SIZE;
            }
            assert(packet_length <= __M_buf_sz + inbuf_sz);
            __M_packet_begin = packet_length - inbuf_sz;
            std::forward<Fn>(fn)(__M_buf[0], __M_buf[0].begin(),
                                 __M_buf[0].begin() + __M_packet_begin);
            if (__M_packet_begin == CHXSQL_MYSQL_PACKET_BUFFER_SIZE) {
                __M_buf.back().resize(CHXSQL_MYSQL_PACKET_BUFFER_SIZE);
                __M_packet_begin = 0;
                __M_buf_sz = 0;
            }
        }
    };
    template <typename Stream, typename DataMapper>
    operation(connection<Stream>&, DataMapper&&, std::string)
        -> operation<Stream, std::decay_t<DataMapper>>;
};
}  // namespace chx::sql::mysql::detail

template <typename Stream>
template <typename DataMapper, typename CompletionToken>
decltype(auto) chx::sql::mysql::connection<Stream>::async_query(
    std::string query, DataMapper&& data_mapper,
    CompletionToken&& completion_token) {
    using result_set_container_type =
        typename std::decay_t<DataMapper>::result_set_type;
    using operation_type =
        decltype(detail::visitor<detail::tags::com_query>::operation(
            *this, std::forward<DataMapper>(data_mapper), std::move(query)));
    return net::async_combine<const std::error_code&,
                              result_set_container_type>(
        get_associated_io_context(),
        std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<operation_type>{}, *this,
        std::forward<DataMapper>(data_mapper), std::move(query));
}