#pragma once

#include <memory>
#include <libpq-fe.h>
#include "./exception.hpp"
#include <chx/net/error_code.hpp>

namespace chx::sql::pq {
inline std::shared_ptr<PGconn> connectdb(const std::string& conn_info) {
    struct deleter {
        void operator()(PGconn* conn) const { PQfinish(conn); }
    };
    std::shared_ptr<PGconn> r(PQconnectdb(conn_info.c_str()),
                              [](PGconn* c) { PQfinish(c); });
    if (PQstatus(r.get()) == CONNECTION_BAD) {
        __CHXNET_THROW_STR_WITH(PQerrorMessage(r.get()), exception);
    }
    PQsetnonblocking(r.get(), 1);
    return r;
}
}  // namespace chx::sql::pq
