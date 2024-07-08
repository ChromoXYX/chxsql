#pragma once

#include <utility>
#include <chx/net/tcp.hpp>

#include "./impl/visitor.hpp"

namespace chx::sql::mysql {
template <typename Stream> class connection {
    Stream __M_stream;

    // construction site🚧
    // connection phase
    std::uint32_t __M_client_cap;

  public:
    template <typename Strm>
    connection(Strm&& strm) : __M_stream(std::forward<Strm>(strm)) {}

    constexpr Stream& stream() noexcept(true) {
        return __M_stream;
    }
    constexpr net::io_context& get_associated_io_context() noexcept(true) {
        return __M_stream.get_associated_io_context();
    }

    template <typename CompletionToken>
    decltype(auto) async_connect(const net::ip::tcp::endpoint& ep,
                                 CompletionToken&& completion_token);

    constexpr void client_cap(std::uint32_t new_value) noexcept(true) {
        __M_client_cap = new_value;
    }
    constexpr std::uint32_t client_cap() const noexcept(true) {
        return __M_client_cap;
    }
};
template <typename Stream> connection(Stream&) -> connection<Stream&>;
template <typename Stream> connection(Stream&&) -> connection<Stream>;
}  // namespace chx::sql::mysql

#include "./impl/connection_phase.ipp"
