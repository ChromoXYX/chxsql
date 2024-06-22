#include <chx/sql/mysql.hpp>
#include <iostream>

namespace sql = chx::sql::mysql;
namespace net = chx::net;

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