#include <chx/sql/mysql.hpp>
#include <chx/ser2/cast_getter.hpp>
#include <iostream>
#include <chx/sql/mysql/impl/packets/handshake_response.hpp>
#include <chx/sql/mysql/impl/auth/caching_sha2_password.hpp>
#include <chx/log/color.hpp>
#include <chx/sql/mysql/data_mapper.hpp>
#include <chx/ser2/list.hpp>

#include <chrono>

namespace sql = chx::sql::mysql;
namespace log = chx::log;
namespace net = chx::net;
namespace ser2 = chx::ser2;
namespace rules = sql::detail::rules;

struct client_demo {
    sql::connection<net::ip::tcp::socket&> sql_connection;
    int cnt = 0;
    std::chrono::time_point<std::chrono::system_clock> start_tp;

    client_demo(sql::connection<net::ip::tcp::socket&> c)
        : sql_connection(std::move(c)) {}

    void start() {
        sql_connection.async_connect(
            {net::ip::address_v4::from_string("127.0.0.1"), 3306},
            [&](const std::error_code& e) {
                std::cout << "SQL Connection " << e.message() << "\n";
                log::printf(CHXLOG_STR("SQL Connection %s\n"), e.message());
                if (!e) {
                    do_query();
                }
            });
    }

    void do_query() {
        start_tp = std::chrono::system_clock::now();
        sql_connection.async_query(
            "select * from all_data_types", sql::map_data_mapper(),
            [this](
                const std::error_code& e,
                std::vector<std::map<std::string, std::optional<std::string>>>
                    result) {
                auto end_tp = std::chrono::system_clock::now();
                if (e) {
                    std::cout << "SQL Query " << e.message() << "\n";
                    return;
                }
                for (auto& map : result) {
                    for (auto& [k, v] : map) {
                        std::cout << k << "=" << (v ? *v : "NULL") << ",";
                    }
                    std::cout << "\n";
                }
                std::cout << "SQL Query " << e.message() << "\n";
                std::cout
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                           end_tp - start_tp)
                           .count()
                    << "ms\n";
                if (cnt++ < 10)
                    do_query();
            });
    }
};

int test(int) { return 1; }

int main() {
    net::io_context ctx;
    net::ip::tcp::socket sock(ctx);
    sock.open();
    sql::connection connection(sock);
    client_demo demo(std::move(connection));
    demo.start();
    ctx.run();
}
