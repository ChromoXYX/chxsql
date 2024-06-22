#pragma once

#include <utility>
#include <chx/net/tcp.hpp>

#include "./impl/visitor.hpp"

namespace chx::sql::mysql {
template <typename Stream> class connection {
    template <typename T> friend struct detail::visitor;

    Stream __M_stream;

  public:
    template <typename Strm>
    connection(Strm&& strm) : __M_stream(std::forward<Strm>(strm)) {}

    constexpr Stream& get_underlying_stream() noexcept(true) {
        return __M_stream;
    }
    constexpr net::io_context& get_associated_io_context() noexcept(true) {
        return __M_stream.get_associated_io_context();
    }

    template <typename CompletionToken>
    decltype(auto) async_connect(const net::ip::tcp::endpoint& ep,
                                 CompletionToken&& completion_token);
};
template <typename Stream> connection(Stream&) -> connection<Stream&>;
template <typename Stream> connection(Stream&&) -> connection<Stream>;
}  // namespace chx::sql::mysql

#include "./impl/connection_phase.ipp"
