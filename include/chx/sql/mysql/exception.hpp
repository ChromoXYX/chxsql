#pragma once

#include <chx/net/exception.hpp>

namespace chx::sql::mysql {
class exception : public net::exception {
  public:
    using net::exception::exception;
};

class malformed_packet : public exception {
  public:
    using exception::exception;
};
}  // namespace chx::sql::mysql