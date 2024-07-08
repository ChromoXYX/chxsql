#pragma once

#include <openssl/evp.h>
#include <memory>

#include "../../exception.hpp"

namespace chx::sql::mysql::detail::auth {
class bad_hash : public exception {
  public:
    using exception::exception;
};

struct hash_ctx {
  private:
    struct md_deleter {
        void operator()(EVP_MD* md) { EVP_MD_free(md); }
    };
    struct ctx_deleter {
        void operator()(EVP_MD_CTX* ctx) { EVP_MD_CTX_free(ctx); }
    };

  public:
    hash_ctx(const char* algo)
        : __M_md(EVP_MD_fetch(nullptr, algo, nullptr), {}),
          __M_ctx(EVP_MD_CTX_new(), {}) {
        if (!md() || !ctx()) {
            throw bad_hash{};
        }
    }

    EVP_MD* md() const noexcept(true) { return __M_md.get(); }
    EVP_MD_CTX* ctx() const noexcept(true) { return __M_ctx.get(); }

    void init() {
        if (!EVP_DigestInit_ex2(ctx(), md(), nullptr)) {
            throw bad_hash{};
        }
    }
    void update(const void* begin, unsigned int len) {
        if (!EVP_DigestUpdate(ctx(), begin, len)) {
            throw bad_hash{};
        }
    }
    template <typename Char> long finalize(Char* out) {
        unsigned int len = 0;
        if (EVP_DigestFinal_ex(ctx(), static_cast<unsigned char*>(out), &len)) {
            return len;
        } else {
            throw bad_hash{};
        }
    }

  private:
    std::unique_ptr<EVP_MD, md_deleter> __M_md;
    std::unique_ptr<EVP_MD_CTX, ctx_deleter> __M_ctx;
};
}  // namespace chx::sql::mysql::detail::auth