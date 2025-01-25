#pragma once

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <chx/net.hpp>
#include <chx/net/detail/tracker.hpp>
#include <sys/poll.h>

namespace chx::sql::hiredis {
class adaptor : public net::detail::enable_weak_from_this<adaptor> {
    CHXNET_NONCOPYABLE

    redisAsyncContext* const __M_ctx = nullptr;

    net::ip::tcp::socket __M_socket;

    net::cancellation_signal __M_poll_read;
    bool __M_poll_read_cancelled = false;
    net::cancellation_signal __M_poll_write;
    bool __M_poll_write_cancelled = false;

    void add_read() {
        if (__M_poll_read) {
            return;
        }
        __M_poll_read_cancelled = false;
        __M_socket.async_poll(
            POLLIN, bind_cancellation_signal(
                        __M_poll_read, [self = weak_from_this()](
                                           const std::error_code& e, int i) {
                            if (self) {
                                self->__M_poll_read.clear();
                            }
                            if (!e && (i & POLLIN) && self &&
                                !self->__M_poll_read_cancelled) {
                                redisAsyncHandleRead(self->__M_ctx);
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
        __M_socket.async_poll(
            POLLOUT, bind_cancellation_signal(
                         __M_poll_write, [self = weak_from_this()](
                                             const std::error_code& e, int i) {
                             if (self) {
                                 self->__M_poll_write.clear();
                             }
                             if (!e && (i & POLLOUT) && self &&
                                 !self->__M_poll_write_cancelled) {
                                 redisAsyncHandleWrite(self->__M_ctx);
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
    adaptor(net::io_context& ctx, redisAsyncContext* c)
        : __M_socket(ctx, c->c.fd), __M_ctx(c) {
        __M_ctx->ev.data = this;
        __M_ctx->ev.addRead = [](void* self) {
            static_cast<adaptor*>(self)->add_read();
        };
        __M_ctx->ev.addWrite = [](void* self) {
            static_cast<adaptor*>(self)->add_write();
        };
        __M_ctx->ev.delRead = [](void* self) {
            static_cast<adaptor*>(self)->del_read();
        };
        __M_ctx->ev.delWrite = [](void* self) {
            static_cast<adaptor*>(self)->del_write();
        };
        __M_ctx->ev.cleanup = [](void* self) {
            static_cast<adaptor*>(self)->cleanup();
        };
    }

    ~adaptor() { __M_socket.release(); }
};
}  // namespace chx::sql::hiredis
