#pragma once

#include <chx/net/exception.hpp>

namespace chx::sql::pq {
class exception : public net::exception {
  public:
    using net::exception::exception;
};
}  // namespace chx::sql::pq
