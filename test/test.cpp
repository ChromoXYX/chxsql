#include <chx/sql/mysql.hpp>
#include <chx/ser2/cast_getter.hpp>
#include <iostream>
#include <chx/sql/mysql/impl/packets/handshake_response.hpp>
#include <chx/sql/mysql/impl/auth/caching_sha2_password.hpp>

namespace sql = chx::sql::mysql;
namespace net = chx::net;
namespace ser2 = chx::ser2;
namespace rules = sql::detail::rules;

int main() {
    net::io_context ctx;
    net::ip::tcp::socket sock(ctx);
    sock.open();
    sql::connection connection(sock);
    connection.async_connect(
        {net::ip::address_v4::from_string("127.0.0.1"), 3306},
        [](const std::error_code& e) { std::cout << e.message() << "\n"; });
    ctx.run();
}
