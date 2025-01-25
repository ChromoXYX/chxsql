#pragma once

#include "../async_exec.hpp"
#include "../result_set.hpp"
#include "../cast.hpp"
#include <optional>

namespace chx::sql::pq::detail::tags {
// struct exec {};
struct exec_params {};
}  // namespace chx::sql::pq::detail::tags

template <>
struct chx::net::detail::async_operation<
    chx::sql::pq::detail::tags::exec_params> {
    template <typename CntlType = int> struct operation {
        template <typename T> using rebind = operation<T>;
        using cntl_type = CntlType;

        std::shared_ptr<PGconn> const __M_conn;
        stream_base __M_strm;
        operation(io_context& ctx, const std::shared_ptr<PGconn>& c)
            : __M_conn(c), __M_strm(ctx, PQsocket(c.get())) {}
        ~operation() { __M_strm.release(); }

        std::optional<sql::pq::result_set> __M_r;

        constexpr cntl_type& cntl() noexcept(true) {
            return static_cast<cntl_type&>(*this);
        }

        void operator()(cntl_type& cntl) { do_poll(); }

        struct ev_poll {};
        void operator()(cntl_type& cntl, const std::error_code& ec, int flags,
                        ev_poll) {
            if (!ec) {
                if (flags & POLLOUT) {
                    PQflush(__M_conn.get());
                }
                if (flags & POLLIN) {
                    PQconsumeInput(__M_conn.get());
                }

                if (PQisBusy(__M_conn.get())) {
                    return do_poll();
                }
                PGresult *r = nullptr, *t = nullptr;
                r = t = PQgetResult(__M_conn.get());
                if (t == nullptr) {
                    return cntl.complete(ec, __M_r);
                }
                while (!PQisBusy(__M_conn.get()) &&
                       (t = PQgetResult(__M_conn.get()))) {
                    if (r) {
                        PQclear(r);
                    }
                    r = t;
                    PQconsumeInput(__M_conn.get());
                }
                if (r) {
                    __M_r.emplace(sql::pq::result_set(r));
                } else {
                    __M_r.reset();
                }
                if (t == nullptr) {
                    // that is
                    return cntl.complete(ec, __M_r);
                } else {
                    return do_poll();
                }
            } else {
                cntl.complete(ec, std::nullopt);
            }
        }

        void do_poll() {
            __M_strm.async_poll(POLLIN | POLLOUT,
                                cntl().template next_with_tag<ev_poll>());
        }
    };
};

template <typename... Params, typename CompletionToken>
decltype(auto) chx::sql::pq::async_exec_params(
    net::io_context& ctx, const std::shared_ptr<PGconn>& conn,
    std::string_view query, std::tuple<Params...> params,
    CompletionToken&& completion_token) {
    std::array<std::string, sizeof...(Params)> s;
    const char* p[sizeof...(Params)] = {};
    std::apply(
        [&](auto&&... ts) {
            int i = 0;
            (..., (s[i] = cast_selector<decltype(ts)>().to(
                       std::forward<decltype(ts)>(ts)),
                   p[i] = s[i].c_str(), ++i));
        },
        params);
    PQsendQueryParams(conn.get(), query.data(), sizeof...(Params), nullptr, p,
                      nullptr, nullptr, 0);
    return net::async_combine<const std::error_code&,
                              std::optional<result_set>>(
        ctx, std::forward<CompletionToken>(completion_token),
        net::detail::type_identity<typename net::detail::async_operation<
            detail::tags::exec_params>::operation<>>{},
        ctx, conn);
}

template <typename CompletionToken>
decltype(auto) chx::sql::pq::async_exec(net::io_context& ctx,
                                        const std::shared_ptr<PGconn>& conn,
                                        std::string_view query,
                                        CompletionToken&& completion_token) {
    return async_exec_params(ctx, conn, query, std::tuple<>{},
                             std::forward<CompletionToken>(completion_token));
}
