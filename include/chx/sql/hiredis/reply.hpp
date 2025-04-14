#pragma once

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <memory>

namespace chx::net::detail {
template <typename Tag> struct async_operation;
}

namespace chx::sql::hiredis {
class reply {
    // since reply is fully polymorphic, it's useless to design a wrapper for
    // it.
    template <typename> friend struct net::detail::async_operation;

    struct __free_reply {
        void operator()(redisReply* r) const noexcept(true) {
            ::freeReplyObject(r);
        }
    };

    std::unique_ptr<redisReply, __free_reply> __M_ptr;

    reply(redisReply* r) noexcept(true) : __M_ptr(r) {}

  public:
    reply(reply&&) = default;
    reply& operator=(reply&&) = default;

    redisReply* operator->() const noexcept(true) { return __M_ptr.get(); }
    operator bool() const noexcept(true) { return __M_ptr.get(); }
};
}  // namespace chx::sql::hiredis
