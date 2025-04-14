#pragma once

#include "../connection.hpp"

#include "../reply.hpp"

namespace chx::sql::hiredis::detail {
namespace tags {
struct connection {};
}  // namespace tags
}  // namespace chx::sql::hiredis::detail

template <>
struct chx::net::detail::async_operation<
    chx::sql::hiredis::detail::tags::connection> {
    template <typename... Args, typename BindCompletionToken>
    decltype(auto) async_command(sql::hiredis::connection* self,
                                 const std::string& format,
                                 std::tuple<Args...> args,
                                 BindCompletionToken&& bind_completion_token) {
        auto& ctx = self->get_associated_io_context();
        redisAsyncContext* ac = self->__M_ac;
        io_context::task_t* task = ctx.acquire();
        task->__M_cancel_type = task->__CT_no_cancel;

        std::apply(
            [&](auto&&... args) {
                int r = redisAsyncCommand(
                    ac,
                    [](redisAsyncContext* ac, void* rep, void* t) {
                        io_context::task_t* task =
                            static_cast<io_context::task_t*>(t);
                        task->__M_additional_ptr = rep;
                        io_uring_sqe* sqe = task->__M_ctx->get_sqe(task);
                        io_uring_prep_nop(sqe);
                    },
                    task, format.c_str(), args...);
            },
            args);

        return async_token_init(
            task->__M_token.emplace(async_token_generate(
                task,
                [](auto& token, io_context::task_t* t) -> int {
                    redisReply* rep =
                        static_cast<redisReply*>(t->__M_additional_ptr);
                    token(std::error_code{}, sql::hiredis::reply(rep));
                    return 0;
                },
                bind_completion_token)),
            bind_completion_token);
    }
};

template <typename... Args, typename CompletionToken>
decltype(auto) chx::sql::hiredis::connection::async_command(
    const std::string& format, std::tuple<Args...> args,
    CompletionToken&& completion_token) {
    return net::detail::async_operation<detail::tags::connection>()
        .async_command(
            this, format, std::move(args),
            net::detail::async_token_bind<const std::error_code&, reply>(
                std::forward<CompletionToken>(completion_token)));
}
