#pragma once

#include <poll.h>
#include <libpq-fe.h>
#include <chx/net/stream_base.hpp>

namespace chx::sql::pq {
template <typename... Params, typename CompletionToken>
decltype(auto)
async_exec_params(net::io_context& ctx, const std::shared_ptr<PGconn>& conn,
                  std::string_view query, std::tuple<Params...> params,
                  CompletionToken&& completion_token);

template <typename CompletionToken>
decltype(auto)
async_exec(net::io_context& ctx, const std::shared_ptr<PGconn>& conn,
           std::string_view query, CompletionToken&& completion_token);

}  // namespace chx::sql::pq

#include "./impl/async_exec.ipp"