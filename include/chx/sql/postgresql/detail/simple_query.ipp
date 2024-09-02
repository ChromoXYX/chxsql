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
struct simple_query {};
}  // namespace tags

template <> struct visitor<tags::simple_query> {
    template <typename Stream, typename CntlType = int> struct operation {
      private:
        struct ev_send_query {};

      public:
        template <typename T> using rebind = operation<Stream, T>;
        using cntl_type = CntlType;

        void operator()(cntl_type& cntl) {
            const std::uint32_t raw_sz = 5 + __M_query.size() + 1;
            std::vector<unsigned char> raw(raw_sz);
            unsigned char* ptr = raw.data();
            *(ptr++) = 'Q';
            ptr = integer_to_network(raw_sz, ptr);
            ptr = string_to_network(__M_query.begin(), __M_query.end(), ptr);
            assert(ptr == raw.data() + raw.size());
            net::async_write_some_exactly(
                __M_conn.stream().lowest_layer(), std::move(raw),
                cntl.template next_with_tag<ev_send_query>());
        }

        void operator()(cntl_type& cntl, const std::error_code& e,
                        std::size_t s, ev_send_query) {
            if (!e) {
                
            } else {
                cntl.complete(e);
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
    };
};
}  // namespace chx::sql::postgresql::detail