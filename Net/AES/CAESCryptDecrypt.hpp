#pragma once

#include <string>

#define AES_TYPE EVP_aes_128_cbc

namespace NNet
{
struct CNetCryptionConfig;
class CAESCryptDecrypt
{
public:
    CAESCryptDecrypt() = delete;

    template<class T>
    static bool aesEncrypt(
            const T& ptext,
            T& ctext,
            const CNetCryptionConfig& config);

    template<class T>
    static bool aesEncrypt(
            const std::string& key,
            const std::string& iv,
            const size_t blockSize,
            const T& ptext,
            T& ctext);

    //get key, vector, size from global config
    template <class T>
    static void aesDecrypt(
            const T& ctext,
            T& rtext,
            const CNetCryptionConfig& config);

    template <class T>
    static void aesDecrypt(
            const std::string& key,
            const std::string& iv,
            const T& ctext,
            T& rtext);
};
}
