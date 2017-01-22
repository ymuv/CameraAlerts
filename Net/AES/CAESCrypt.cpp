#include <memory>
#include <openssl/evp.h>
#include <vector>
#include <QByteArray>

#include "Net/AES/CAESCryptDecrypt.hpp"
#include "Net/CNetCryptionConfig.hpp"
#include "logging/Log.hpp"

namespace NNet
{
using EVP_CIPHER_CTX_free_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

template <class T>
bool CAESCryptDecrypt::aesEncrypt(
        const std::string& key,
        const std::string& iv,
        const size_t blockSize,
        const T& ptext,
        T& ctext)
{
    EVP_add_cipher(AES_TYPE());
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    int rc = EVP_EncryptInit_ex(
                ctx.get(),
                AES_TYPE(),
                NULL,
                reinterpret_cast<const unsigned char *>( key.data()),
                reinterpret_cast<const unsigned char *>(iv.data())
                );
    if (rc != 1)
    {
        LOG_ERROR << "EVP_EncryptInit_ex failed" << rc;
        return false;
    }

    //     // Recovered text expands upto BLOCK_SIZE
    ctext.resize(ptext.size() + blockSize);
    int out_len1 = ctext.size();

    //rc = EVP_EncryptUpdate(ctx.get(), (unsigned char*)&ctext[0], &out_len1, (const unsigned char*)&ptext[0], (int)ptext.size());
    rc = EVP_EncryptUpdate(
                ctx.get(),
                (unsigned char*)ctext.data(),
                &out_len1,
                (const unsigned char*)ptext.data(),
                (int)ptext.size());

    if (rc != 1)
    {
        LOG_ERROR << "EVP_EncryptUpdate failed" << rc;
        return false;
    }

    int out_len2 = ctext.size() - out_len1;
    rc = EVP_EncryptFinal_ex(
                ctx.get(),
                (unsigned char*)ctext.data()+out_len1,
                &out_len2);
    if (rc != 1)
    {
        LOG_ERROR << "EVP_EncryptFinal_ex failed" << rc;
        return false;
    }

    //     // Set cipher text size now that we know it
    ctext.resize(out_len1 + out_len2);
    return true;
}

template <class T>
bool CAESCryptDecrypt::aesEncrypt(
        const T& ptext,
        T& ctext,
        const CNetCryptionConfig& config)
{
    if (!config.mKey2.empty() && !config.mKey3.empty())
    {
        T tmp;
        return aesEncrypt(config.mKey, config.mInitialVector, config.mKeySize, ptext, ctext) &&
                aesEncrypt(config.mKey2, config.mInitialVector, config.mKey2Size, ctext, tmp) &&
                aesEncrypt(config.mKey3, config.mInitialVector, config.mKey3Size, tmp, ctext);
    }
    else
    {
        return aesEncrypt(config.mKey, config.mInitialVector, config.mKeySize, ptext, ctext);
    }
}

template bool CAESCryptDecrypt::aesEncrypt(
    const QByteArray&,
    QByteArray&,
    const CNetCryptionConfig&);
}
