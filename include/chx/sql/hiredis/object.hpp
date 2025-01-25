#pragma once

#include <hiredis/hiredis.h>

namespace chx::sql::hiredis {
enum class Type {

};

class object {
    redisReply* __M_p = nullptr;

  public:
    constexpr Type type() const noexcept(true) {}
};
}  // namespace chx::sql::hiredis