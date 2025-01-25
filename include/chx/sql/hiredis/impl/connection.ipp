#pragma once

#include "../connection.hpp"

namespace chx::sql::hiredis::detail {
namespace tags {
struct connection {};
}  // namespace tags
}  // namespace chx::sql::hiredis::detail

template <>
struct chx::net::detail::async_operation<
    chx::sql::hiredis::detail::tags::connection> {
    template <typename Stream, typename BindCompletionToken>
    decltype(auto) async_command(sql::hiredis::connection<Stream>* self,
                                 std::string_view format,
                                 BindCompletionToken&& bind_completion_token) {
        auto& ctx = self->get_associated_io_context();
        redisAsyncContext* ac = self->__M_ac;
        io_context::task_t* task = ctx.acquire();
        task->__M_cancel_type = task->__CT_no_cancel;

        redisAsyncCommand(
            ac,
            [](redisAsyncContext* ac, void* rep, void* t) {
                io_context::task_t* task = static_cast<io_context::task_t*>(t);
                task->__M_additional = reinterpret_cast<std::uint64_t>(rep);
                io_uring_sqe* sqe = task->__M_ctx->get_sqe(task);
                io_uring_prep_nop(sqe);
            },
            task, format.begin());

        return async_token_init(
            task->__M_token.emplace(async_token_generate(
                task,
                [](auto& token, io_context::task_t* t) -> int {
                    redisReply* rep =
                        reinterpret_cast<redisReply*>(t->__M_additional);
                    token(std::error_code{}, sql::hiredis::reply(rep));
                    return 0;
                },
                bind_completion_token)),
            bind_completion_token);
    }
};

template <typename Stream>
template <typename CompletionToken>
decltype(auto) chx::sql::hiredis::connection<Stream>::async_command(
    std::string_view format, CompletionToken&& completion_token) {
    return net::detail::async_operation<detail::tags::connection>()
        .async_command(
            this, format,
            net::detail::async_token_bind<const std::error_code&, reply>(
                std::forward<CompletionToken>(completion_token)));
}
