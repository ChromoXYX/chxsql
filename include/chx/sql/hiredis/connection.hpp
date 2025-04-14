#pragma once

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <chx/net/detail/tracker.hpp>
#include <chx/net/io_context.hpp>
#include <chx/net/tcp.hpp>
#include <chx/net/basic_stream_view.hpp>
#include <chx/net/detail/remove_rvalue_reference.hpp>
#include <poll.h>

namespace chx::sql::hiredis {
class connection : public net::detail::enable_weak_from_this<connection> {
    CHXNET_NONCOPYABLE

    template <typename> friend struct net::detail::async_operation;

    redisAsyncContext* __M_ac = nullptr;
    net::io_context* const __M_ctx;
    net::basic_stream_view<net::stream_base> __M_strm;

    net::cancellation_signal __M_poll_read;
    bool __M_poll_read_cancelled = false;
    net::cancellation_signal __M_poll_write;
    bool __M_poll_write_cancelled = false;

    void add_read() {
        if (__M_poll_read) {
            return;
        }
        __M_poll_read_cancelled = false;
        __M_strm.async_poll(
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
        __M_strm.async_poll(
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
    connection(net::io_context& ctx, const std::string& ip, unsigned short port)
        : __M_ctx(&ctx), __M_strm(ctx) {
        redisOptions r = {};
        r.type = REDIS_CONN_TCP;
        r.endpoint.tcp.ip = ip.c_str();
        r.endpoint.tcp.port = port;
        r.options = REDIS_OPT_NOAUTOFREEREPLIES | REDIS_OPT_NOAUTOFREE;

        // well, redisAsyncConnect* actually calls redisConnect, so it may be
        // block?
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

        redisAsyncSetConnectCallback(
            __M_ac, [](const struct redisAsyncContext* ac, int status) {});

        __M_strm.native_handler(__M_ac->c.fd);
    }

    ~connection() {
        if (__M_ac) {
            redisAsyncFree(std::exchange(__M_ac, nullptr));
        }
    }

    constexpr net::io_context& get_associated_io_context() const
        noexcept(true) {
        return const_cast<net::io_context&>(*__M_ctx);
    }

    constexpr redisAsyncContext* native_handler() noexcept(true) {
        return __M_ac;
    }

    template <typename... Args, typename CompletionToken>
    decltype(auto) async_command(const std::string& format,
                                 std::tuple<Args...> args,
                                 CompletionToken&& completion_token);
};
}  // namespace chx::sql::hiredis

#include "./impl/connection.ipp"
