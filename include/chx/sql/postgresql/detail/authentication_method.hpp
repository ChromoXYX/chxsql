#pragma once

#include <cstdint>

namespace chx::sql::postgresql::detail {
enum AuthenticationMethod : std::uint32_t {
    TrustOrFinished = 0,
    KerberosV5 = 2,
    CleartextPassword = 3,
    MD5Password = 5,
    GSS = 7,
    SSPI = 9,
    SASL = 10,
};
}