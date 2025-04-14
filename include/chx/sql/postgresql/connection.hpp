#pragma once

#include <chx/net/io_context.hpp>

namespace chx::sql::postgresql {
namespace detail {
template <typename Tag> struct visitor;
}
struct connect_parameters {
    std::string username;
    std::string password;
    std::string database;
};

template <typename Stream> class connection {
    CHXNET_NONCOPYABLE

    template <typename Tag> friend struct detail::visitor;

    Stream __M_stream;

    std::vector<unsigned char> __M_buffer;
    const unsigned char *__M_buf_begin = nullptr, *__M_buf_end = nullptr;

  public:
    template <typename Strm>
    connection(Strm&& strm)
        : __M_stream(std::forward<Strm>(strm)), __M_buffer(4096) {}

    constexpr Stream& stream() noexcept(true) { return __M_stream; }
    constexpr net::io_context& get_associated_io_context() noexcept(true) {
        return __M_stream.get_associated_io_context();
    }

    template <typename CompletionToken>
    decltype(auto) async_connect(connect_parameters&& para,
                                 CompletionToken&& completion_token);
    template <typename CompletionToken>
    [[deprecated]] decltype(auto)
    async_simple_query(std::string&& query, CompletionToken&& completion_token);

    template <typename CompletionToken>
    decltype(auto) async_parse(std::string prepared_statement_name,
                               std::string prepared_statement,
                               CompletionToken&& completion_token);
    template <typename... Parameters, typename CompletionToken>
    decltype(auto) async_execute(const std::string& prepared_statement_name,
                                 std::tuple<Parameters...> parameters,
                                 CompletionToken&& completion_token);
    template <typename... Parameters, typename CompletionToken>
    decltype(auto)
    async_execute_last(const std::string& prepared_statement_name,
                       std::tuple<Parameters...> parameters,
                       CompletionToken&& completion_token);

    template <typename... Parameters, typename CompletionToken>
    decltype(auto) async_execute_oneshot(std::string_view sql,
                                         std::tuple<Parameters...> parameters,
                                         CompletionToken&& completion_token);
};
template <typename Stream>
connection(connection<Stream>&&) -> connection<Stream>;
template <typename Stream> connection(Stream&) -> connection<Stream&>;
template <typename Stream> connection(Stream&&) -> connection<Stream>;
}  // namespace chx::sql::postgresql

#include "./detail/connect.ipp"
#include "./detail/simple_query.ipp"
#include "./detail/parse_query.ipp"
#include "./detail/execute.ipp"
#include "./detail/oneshot.ipp"
