#pragma once

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <chx/net/detail/tracker.hpp>
#include <chx/net/io_context.hpp>
#include <chx/net/tcp.hpp>
#include <chx/net/detail/remove_rvalue_reference.hpp>
#include <poll.h>

#include "./reply.hpp"

namespace chx::sql::hiredis {
template <typename Stream>
class connection
    : public net::detail::enable_weak_from_this<connection<Stream>> {
    CHXNET_NONCOPYABLE

    template <typename> friend struct net::detail::async_operation;

    redisAsyncContext* __M_ac = nullptr;
    net::io_context* const __M_ctx;
    Stream __M_sock;

    net::cancellation_signal __M_poll_read;
    bool __M_poll_read_cancelled = false;
    net::cancellation_signal __M_poll_write;
    bool __M_poll_write_cancelled = false;

    void add_read() {
        if (__M_poll_read) {
            return;
        }
        __M_poll_read_cancelled = false;
        __M_sock.async_poll(
            POLLIN, bind_cancellation_signal(
                        __M_poll_read, [self = this->weak_from_this()](
                                           const std::error_code& e, int i) {
                            if (self) {
                                self->__M_poll_read.clear();
                            }
                            if (!e && (i & POLLIN) && self &&
                                !self->__M_poll_read_cancelled) {
                                redisAsyncHandleRead(self->__M_ac);
                            }
                        }));
    }

    void del_read() {
        if (!__M_poll_read || __M_poll_read_cancelled) {
            return;
        }
        __M_poll_read_cancelled = true;
        __M_poll_read.emit();
    }

    void add_write() {
        if (__M_poll_write) {
            return;
        }
        __M_poll_write_cancelled = false;
        __M_sock.async_poll(
            POLLOUT, bind_cancellation_signal(
                         __M_poll_write, [self = this->weak_from_this()](
                                             const std::error_code& e, int i) {
                             if (self) {
                                 self->__M_poll_write.clear();
                             }
                             if (!e && (i & POLLOUT) && self &&
                                 !self->__M_poll_write_cancelled) {
                                 redisAsyncHandleWrite(self->__M_ac);
                             }
                         }));
    }

    void del_write() {
        if (!__M_poll_write || __M_poll_write_cancelled) {
            return;
        }
        __M_poll_write_cancelled = true;
        __M_poll_write.emit();
    }

    void cleanup() {
        del_read();
        del_write();
    }

  public:
    template <typename Strm>
    connection(net::io_context& ctx, Strm&& sock)
        : __M_ctx(&ctx), __M_sock(std::forward<Strm>(sock)) {
        redisOptions r = {};
        r.type = REDIS_CONN_USERFD;
        r.endpoint.fd = __M_sock.native_handler();
        r.options = REDIS_OPT_NOAUTOFREEREPLIES;

        auto* ac = redisAsyncConnectWithOptions(&r);
        if (ac->err) {
            redisAsyncFree(ac);
            __CHXNET_THROW_STR(ac->errstr);
        }

        __M_ac = ac;
        __M_ac->ev.data = this;
        __M_ac->ev.addRead = [](void* self) {
            static_cast<connection*>(self)->add_read();
        };
        __M_ac->ev.addWrite = [](void* self) {
            static_cast<connection*>(self)->add_write();
        };
        __M_ac->ev.delRead = [](void* self) {
            static_cast<connection*>(self)->del_read();
        };
        __M_ac->ev.delWrite = [](void* self) {
            static_cast<connection*>(self)->del_write();
        };
        __M_ac->ev.cleanup = [](void* self) {
            static_cast<connection*>(self)->cleanup();
        };
    }

    ~connection() {
        if (__M_ac) {
            __M_sock.release();
        }
    }

    constexpr net::io_context& get_associated_io_context() const
        noexcept(true) {
        return const_cast<net::io_context&>(*__M_ctx);
    }

    constexpr redisAsyncContext* native_handler() noexcept(true) {
        return __M_ac;
    }

    template <typename CompletionToken>
    decltype(auto) async_command(std::string_view format,
                                 CompletionToken&& completion_token);
};
template <typename Strm>
connection(net::io_context&, Strm&&)
    -> connection<typename net::detail::remove_rvalue_reference<Strm&&>::type>;
}  // namespace chx::sql::hiredis

#include "./impl/connection.ipp"
