#pragma once

#include "./packets/handshake_v10.hpp"

namespace chx::sql::mysql::detail {
template <typename Cntl, typename... Plugins>
struct auth_plugin : protected Plugins... {
    using Plugins::operator()...;
    using Plugins::process_packet...;

    constexpr bool perform_auth(const packets::handshake_v10& handshake) {
        return (... || ((std::equal(Plugins::auth_plugin_name.begin(),
                                    Plugins::auth_plugin_name.end(),
                                    handshake.auth_plugin_name.begin(),
                                    handshake.auth_plugin_name.end()))
                            ? (Plugins::perform_auth(handshake), true)
                            : false));
    }
};
}  // namespace chx::sql::mysql::detail