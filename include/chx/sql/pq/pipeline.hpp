#pragma once

#include <chx/net/detail/noncopyable.hpp>
#include <chx/net/io_context.hpp>

namespace chx::sql::pq {
class pipeline {
    CHXNET_NONCOPYABLE
    CHXNET_NONMOVEABLE

    net::io_context* const __M_ctx;
    net::task_decl* __M_listening_task = nullptr;

    std::vector<net::task_decl*> __M_tasks;

  public:
    pipeline(net::io_context& ctx) : __M_ctx(&ctx) {}
};
}  // namespace chx::sql::pq