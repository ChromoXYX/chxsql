#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <openssl/evp.h>

namespace chx::sql::mysql::detail {
inline bool cacheing_sha2_pw(unsigned char* out,
                             const unsigned char* plain_password,
                             std::size_t plain_password_n,
                             const unsigned char* nonce1, std::size_t nonce1_n,
                             const unsigned char* nonce2,
                             std::size_t nonce2_n) noexcept(true) {
    unsigned int len = 0;

    constexpr auto __md_deleter = [](EVP_MD* ptr) { EVP_MD_free(ptr); };
    std::unique_ptr<EVP_MD, decltype(__md_deleter)> md(
        EVP_MD_fetch(nullptr, "SHA256", nullptr), __md_deleter);
    constexpr auto __ctx_deleter = [](EVP_MD_CTX* ptr) {
        EVP_MD_CTX_free(ptr);
    };
    std::unique_ptr<EVP_MD_CTX, decltype(__ctx_deleter)> mdctx(EVP_MD_CTX_new(),
                                                               __ctx_deleter);
    if (!EVP_DigestInit_ex2(mdctx.get(), md.get(), nullptr)) {
        return false;
    }
    if (!EVP_DigestUpdate(mdctx.get(), plain_password, plain_password_n)) {
        return false;
    }
    unsigned char digest_a[EVP_MAX_MD_SIZE] = {};
    if (!EVP_DigestFinal_ex(mdctx.get(), digest_a, &len)) {
        return false;
    }
    assert(len == 32);
    if (!EVP_DigestInit_ex2(mdctx.get(), md.get(), nullptr)) {
        return false;
    }
    if (!EVP_DigestUpdate(mdctx.get(), digest_a, 32)) {
        return false;
    }
    unsigned char digest_b[EVP_MAX_MD_SIZE] = {};
    if (!EVP_DigestFinal_ex(mdctx.get(), digest_b, &len)) {
        return false;
    }
    assert(len == 32);

    if (!EVP_DigestInit_ex2(mdctx.get(), md.get(), nullptr)) {
        return false;
    }
    if (!EVP_DigestUpdate(mdctx.get(), digest_b, 32)) {
        return false;
    }
    if (!EVP_DigestUpdate(mdctx.get(), nonce1, nonce1_n)) {
        return false;
    }
    if (!EVP_DigestUpdate(mdctx.get(), nonce2, nonce2_n)) {
        return false;
    }
    unsigned char digest_c[EVP_MAX_MD_SIZE] = {};
    if (!EVP_DigestFinal_ex(mdctx.get(), digest_c, &len)) {
        return false;
    }
    assert(len == 32);

    for (int i = 0; i < 32; ++i) {
        out[i] = digest_a[i] ^ digest_c[i];
    }
    return true;
}
}  // namespace chx::sql::mysql::detail