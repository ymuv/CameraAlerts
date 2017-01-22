#include "CAES.hpp"
#include "CAESCryptDecrypt.hpp"
#include "Net/CNetCryptionConfig.hpp"
#include "Net/IWorker.hpp"
#include "logging/Log.hpp"

namespace NNet
{

template<class T>
void CAES::decrypt(const T& str, INet* net)
{
    if (!net->getCryptionConfig().mKey.empty() && net->getCryptionConfig().mIsUseCryption)
    {
        T decrypted;
        CAESCryptDecrypt::aesDecrypt(
                    str,
                    decrypted,
                    net->getCryptionConfig());

        net->getWorker()->receiveCallBack(decrypted);
    }
    else
    {
        net->getWorker()->receiveCallBack(str);
    }
}

template<class T>
void CAES::crypt(const T& str, INet* net)
{
   if (!net->getCryptionConfig().mKey.empty() && net->getCryptionConfig().mIsUseCryption)
   {
       T crypted;
       if (CAESCryptDecrypt::aesEncrypt(str, crypted, net->getCryptionConfig()))
       {
            net->sendImplementation(crypted);
       }
       else
       {
           LOG_WARN << "CAESCryptDecrypt::aesEncrypt fail";
           //TODO: this
       }
   }
   else
   {
       net->sendImplementation(str);
   }
}

template void CAES::crypt(const QByteArray&, INet* );
template void CAES::decrypt(const QByteArray&, INet*);
}
