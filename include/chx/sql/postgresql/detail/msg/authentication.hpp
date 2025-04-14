#pragma once

#include "../basic_type.hpp"
#include "../make_success.hpp"
#include "../authentication_method.hpp"
#include <cassert>
#include <variant>

namespace chx::sql::postgresql::detail::msg {
struct authentication {
    std::variant<integer<std::uint32_t>, integer<std::uint32_t>, string<>>
        parsers;
    std::variant<AuthenticationMethod, std::uint32_t, std::string> result;

    constexpr AuthenticationMethod authentication_method() noexcept(true) {
        switch (result.index()) {
        case 0: {
            return *std::get_if<0>(&result);
        }
        case 1: {
            return MD5Password;
        }
        case 2: {
            return SASL;
        }
        default:
            assert(false);
        }
    }

    static constexpr std::uint8_t message_type = 'R';
    constexpr ParseResult on_message_type(std::uint8_t type) const
        noexcept(true) {
        return type == 'R' ? ParseSuccess : ParseMalformed;
    }
    constexpr ParseResult on_message_length(std::uint32_t len) const
        noexcept(true) {
        return ParseSuccess;
    }
    ParseResult on_body(const unsigned char* begin,
                        const unsigned char* end) noexcept(true) {
        const std::size_t idx = parsers.index();
        if (idx == 0) {
            integer<std::uint32_t>& parser = *std::get_if<0>(&parsers);
            ParseResult r = parser(begin, end);
            if (r == ParseSuccess) {
                // auth method
                const std::uint32_t auth_method = parser.value();
                parser = {};
                if (auth_method == TrustOrFinished) {
                    result.template emplace<0>(TrustOrFinished);
                    return make_success(begin, end);
                } else if (auth_method == KerberosV5) {
                    result.template emplace<0>(KerberosV5);
                    return make_success(begin, end);
                } else if (auth_method == CleartextPassword) {
                    result.template emplace<0>(CleartextPassword);
                    return make_success(begin, end);
                } else if (auth_method == MD5Password) {
                    // md5
                    parsers.template emplace<1>();
                    return on_body(begin, end);
                } else if (auth_method == GSS) {
                    result.template emplace<0>(GSS);
                    return make_success(begin, end);
                } else if (auth_method == SSPI) {
                    result.template emplace<0>(SSPI);
                    return make_success(begin, end);
                } else if (auth_method == SASL) {
                    // sasl
                    parsers.template emplace<2>();
                    return on_body(begin, end);
                } else {
                    return ParseMalformed;
                }
            } else {
                return r;
            }
        } else if (idx == 1) {
            // md5
            integer<std::uint32_t>& parser = *std::get_if<1>(&parsers);
            ParseResult r = parser(begin, end);
            if (r == ParseSuccess) {
                result.template emplace<1>(htonl(parser.value()));
                parser = {};
                return make_success(begin, end);
            } else {
                return r;
            }
        } else {
            string<>& parser = *std::get_if<2>(&parsers);
            ParseResult r = parser(begin, end);
            if (r == ParseSuccess) {
                result.template emplace<2>(parser.value());
                parser = {};
                return make_success(begin, end);
            } else {
                return r;
            }
        }
    }
};
}  // namespace chx::sql::postgresql::detail::msg