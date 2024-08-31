#pragma once

#include <chx/net/io_context.hpp>

namespace chx::sql::postgresql {
struct connect_parameters {
    std::string username;
    std::string password;
    std::string database;
};

template <typename Stream> class connection {
    Stream __M_stream;

  public:
    connection(connection&& other) noexcept(true)
        : __M_stream(std::forward<Stream>(other.__M_stream)) {}
    template <typename Strm>
    connection(Strm&& strm) : __M_stream(std::forward<Strm>(strm)) {}

    constexpr Stream& stream() noexcept(true) { return __M_stream; }
    constexpr net::io_context& get_associated_io_context() noexcept(true) {
        return __M_stream.get_associated_io_context();
    }

    template <typename CompletionToken>
    decltype(auto) async_connect(connect_parameters&& para,
                                 CompletionToken&& completion_token);
    template <typename CompletionToken>
    decltype(auto) async_query(std::string&& query,
                               CompletionToken&& completion_token);
};
template <typename Stream>
connection(connection<Stream>&&) -> connection<Stream>;
template <typename Stream> connection(Stream&) -> connection<Stream&>;
template <typename Stream> connection(Stream&&) -> connection<Stream>;
}  // namespace chx::sql::postgresql

#include "./detail/connect.ipp"