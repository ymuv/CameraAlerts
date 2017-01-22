#pragma once

#include <QString>
#include <QStringList>
#include <string>
#include <vector>

#include "CAlertConfig.hpp"
namespace NAlarms
{
struct CSMTPAlertConfig : CAlertConfig
{
    int mSendTo;
    int mSendFrom;

    QString mSMTPHost;
    int mSMTPPort;

    QString mUserName;
    QString mUserPassword;

    std::string mSubjectPrefix;

    std::string mComputerName;

    QStringList mReceiverEMails;

    std::string mZipPassword;
    size_t mLastImagesCountToEmailEncrypted;

    std::string mRunExternalCommands;

    std::string mTmpDir;
    size_t mLastImagesCountToEmailInZip; //send images to e-mail. Images save to mTmpDir (if mTmpDir is not empy)
    double mVideoFps;
    int mMaskCountToVideo;
    bool mIsWriteVideoToEmail; ///send video to e-mail. Video saves to mTmpDir (if mTmpDir is not empy)

    inline
    const char* getZipPassword() const
    {
        if (mZipPassword.empty())
        {
            return nullptr;
        }
        return mZipPassword.c_str();
    }
};
}

