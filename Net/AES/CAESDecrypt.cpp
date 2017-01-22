#include <memory>
#include <openssl/evp.h>
#include <vector>
#include <QByteArray>

#include "CAESCryptDecrypt.hpp"
#include "Net/CNetCryptionConfig.hpp"
#include "logging/Log.hpp"

namespace NNet
{
using EVP_CIPHER_CTX_free_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

template <class T>
void CAESCryptDecrypt::aesDecrypt(
        const T& ctext,
        T& rtext,
        const CNetCryptionConfig& config)
{
    if (!config.mKey2.empty() && !config.mKey3.empty())
    {
        T tmp;
        aesDecrypt(config.mKey3, config.mInitialVector, ctext,  rtext);
        aesDecrypt(config.mKey2, config.mInitialVector, rtext,  tmp);
        aesDecrypt(config.mKey,  config.mInitialVector, tmp, rtext);
    }
    else
    {
        aesDecrypt(config.mKey, config.mInitialVector, ctext, rtext);
    }
}

template <class T>
void CAESCryptDecrypt::aesDecrypt(
        const std::string& key,
        const std::string& iv,
        const T& ctext,
        T& rtext)
{
    EVP_add_cipher(AES_TYPE());
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    int rc = EVP_DecryptInit_ex(
                ctx.get(),
                AES_TYPE(),
                NULL,
                reinterpret_cast<const unsigned char *>(key.data()),
                reinterpret_cast<const unsigned char *>(iv.data()));
    if (rc != 1)
    {
        LOG_WARN << "EVP_DecryptInit_ex failed" << rc;
    }

    // Recovered text contracts upto BLOCK_SIZE
    rtext.resize(ctext.size());
    int outLen1 = rtext.size();

    //rc = EVP_DecryptUpdate(ctx.get(), (u_char*)&rtext, &out_len1, (const u_char*)&ctext[0], (int)size);
    rc = EVP_DecryptUpdate(
                ctx.get(),
                (unsigned char*)rtext.data(),
                &outLen1,
                (const unsigned char*)ctext.data(),
                (int)ctext.size());
    if (rc != 1)
    {
        LOG_WARN << "EVP_DecryptUpdate failed" << rc;
    }

    int outLen2 = rtext.size() - outLen1;
    rc = EVP_DecryptFinal_ex(ctx.get(), (unsigned char*)ctext.data() +outLen1, &outLen2);
    if (rc != 1)
    {
        LOG_DEBUG << "EVP_DecryptFinal_ex failed" << rc << ctext.size();
    }

    // Set recovered text size now that we know it
    rtext.resize(outLen1 + outLen2);
}

template void CAESCryptDecrypt::aesDecrypt(
    const QByteArray&,
    QByteArray&,
    const CNetCryptionConfig&);
}
