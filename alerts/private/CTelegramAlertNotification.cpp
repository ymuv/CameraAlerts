#include "alerts/CTelegramAlertNotification.hpp"
#include "common/CDateTime.hpp"

#ifdef __BUILD_TELEGRAM_BOT__
#include <thread>

#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "alerts/CTelegramAlertNotificationConfig.hpp"
#include "alerts/CCommonAlertConfig.hpp"
#include "common/CConfig.hpp"
#include "common/CMainFunctions.hpp"
#include "common/CMemoryUssage.hpp"
#include "logging/Log.hpp"
#include "serial/CSerialPorts.hpp"
#include "serial/CSerialPortReader.hpp"
#include "tgbot/tgbot.h"
#endif

namespace NAlarms
{

#ifdef __BUILD_TELEGRAM_BOT__
namespace
{

const std::string sCommands =
        "/Help"
        "\n/Stat"
        "\n/HomeIn /setHomeIn"
        "\n/HomeOut /SetHomeOut"
        "\n/MotionOn /MotionOff"
        "\n/Get";

void sendMessageToAll(
        std::shared_ptr<TgBot::Bot> mBot,
        const CTelegramAlertNotificationConfig& mConfig,
        const std::string message,
        int64_t requestUserID = 0,
        bool isSendToVerifyUsers = false)
{
    std::vector<int64_t> usersForSend = mConfig.mAlertUsers;
    if (requestUserID > 0)
    {
        usersForSend.push_back(requestUserID);
    }
    if (isSendToVerifyUsers)
    {
        std::copy(mConfig.mVerifyUsers.begin(), mConfig.mVerifyUsers.end(), std::back_inserter(usersForSend));
    }

    std::sort(usersForSend.begin(), usersForSend.end());
    std::vector<int64_t>::iterator it;
    it = std::unique(usersForSend.begin(), usersForSend.end());
    usersForSend.resize(std::distance(usersForSend.begin(), it));

    for (const auto alertUser : usersForSend)
    {
        try
        {
            mBot->getApi().sendMessage(alertUser, message);
        }
        catch (const std::exception& exc)
        {
            LOG_ERROR << "catch exception" << exc.what();
        }
        catch (...)
        {
            LOG_ERROR << "catch exception: unknown error";
        }
    }
}

void sendImageToAllUser(
        std::shared_ptr<TgBot::Bot> bot,
        const CTelegramAlertNotificationConfig& config,
        TgBot::InputFile::Ptr filePtr,
        const std::string& infoStr)
{
    for (const auto alertUser : config.mAlertUsers)
    {
        try
        {
            bot->getApi().sendPhoto(alertUser, filePtr, infoStr);
        }
        catch (const std::exception& exc)
        {
            LOG_ERROR << "catch exception" << exc.what();
        }
        catch (...)
        {
            LOG_ERROR << "catch exception: unknown error";
        }
    }
}

void sendAlertAsync(
        std::shared_ptr<TgBot::Bot> bot,
        const CTelegramAlertNotificationConfig& config,
        const CImageBuffer buff,
        const std::vector<size_t> bufferIds,
        const std::string alertAlgorithm,
        const std::string alertInfo)
{
    std::vector<uchar> matBuffer;

    for (auto bufferId : bufferIds)
    {
        bool isMaskSendet = false;
        if (!buff.isBufferExistAndFull(bufferId))
        {
            continue;
        }
        const auto lastMat = buff.getLast(bufferId);
        if (!lastMat)
        {
            continue;
        }
        CMatWithTimeStampLocker locker(lastMat);
        const std::string& date = lastMat->getDateTimeWithMS();

        TgBot::InputFile::Ptr f(new TgBot::InputFile);
        f->mimeType = "image/jpeg";

        //send mask
        if (!lastMat->getMask().empty())
        {
            cv::imencode(".jpg", lastMat->getMask(), matBuffer);
            f->fileName = std::to_string(bufferId) + "_" + lastMat->getDateTimeWithMS() + "_mask.jpg";
            f->data = std::string(matBuffer.begin(), matBuffer.end());
            sendImageToAllUser(
                        bot,
                        config, f,
                        "mask " + lastMat->getDateTimeWithMS() + " "
                        + CImageBuffer::getBufferInstance().getBufferInfo(bufferId)
                        + " id:" + std::to_string(bufferId) + "\n"
                        + alertAlgorithm + "\n" + alertInfo + "\n" + sCommands);
            isMaskSendet = true;
        }

        matBuffer.clear();

        if (config.mIsSendImagesWhenMaskSendet || !isMaskSendet)
        {
            cv::imencode(".jpg", lastMat->getMat(), matBuffer);
            f->fileName = std::to_string(bufferId) + "_" + lastMat->getDateTimeWithMS() + ".jpg";
            f->data = std::string(matBuffer.begin(), matBuffer.end());
            sendImageToAllUser(
                        bot,
                        config, f,
                        date + " im id" + std::to_string(lastMat->getId())
                        + " " + CImageBuffer::getBufferInstance().getBufferInfo(bufferId)
                        + " id:" + std::to_string(bufferId) + "\n" +
                        alertAlgorithm + "\n" + alertInfo + "\n" + sCommands);
        }
    }
}

void sendLastPhotoAsync(
        std::shared_ptr<TgBot::Bot> mBot,
        const TgBot::Message::Ptr& message)
{
    const auto& buff = CImageBuffer::getBufferInstance();
    auto buffSize = buff.getBuffersSize();
    std::vector<uchar> matBuffer;

    for (size_t bufferId = 0; bufferId < buffSize; bufferId++)
    {
        if (!buff.isBufferExistAndFull(bufferId))
        {
            continue;
        }
        const auto lastMat = buff.getLast(bufferId);
        if (!lastMat)
        {
            continue;
        }
        const std::string& date = lastMat->getDateTimeWithMS();

        matBuffer.clear();
        cv::imencode(".jpg", lastMat->getMat(), matBuffer);

        TgBot::InputFile::Ptr f(new TgBot::InputFile);
        f->mimeType = "image/jpeg";
        f->fileName = lastMat->getDateTimeWithMS() + ".jpg";
        f->data = std::string(matBuffer.begin(), matBuffer.end());

        try
        {
            mBot->getApi().sendPhoto(message->from->id, f, std::to_string(bufferId) + "_" + date + ".jpg" + "\n" + sCommands);
        }
        catch (const std::exception& exc)
        {
            LOG_ERROR << "catch exception" << exc.what();
        }
        catch (...)
        {
            LOG_ERROR << "catch exception: unknown error";
        }
    }
}

void GetAllImages(const TgBot::Message::Ptr& message)
{
    if (!CTelegramAlertNotification::getInstance(*CConfig::getInstance().mCommonAlertConfig->mTelegramConfig)
            ->isUserInVerifyList(message))
    {
        return;
    }

    std::thread photoThread(
                sendLastPhotoAsync,
                CTelegramAlertNotification::getInstance(
                    *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig)->getBot(),
                message);
    photoThread.detach();
}

void getStatus(const TgBot::Message::Ptr& message)
{
    const auto bot = CTelegramAlertNotification::getInstance(
                *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig);
    if (!bot->isUserInVerifyList(message))
    {
        return;
    }

    std::string status =
            "Status: " + (CConfig::getInstance().mRunStatus.mHomeStatus == CRunStatus::HomeStatus::HOME_OUT
                          ? std::string("Home out") : std::string("In Home"))
            + "\nIs Run motion algo:" + std::to_string(CConfig::getInstance().mRunStatus.mIsRunMotionAlgo)
            + "\ncurrent mem ussage: " + std::to_string(CMemoryUssage::processMemoryUssage())
            + "\nmax mem ussage: " + std::to_string(CMemoryUssage::processMaxMemoryUssage())
            + "\nRun at: " + CMainFunctions::sRunTime
            + "\n" + sCommands;
    CImageBuffer buff = CImageBuffer::getBufferInstance();
    for (size_t i = 0; i < CImageBuffer::getBufferInstance().getBuffersSize(); i++)
    {
        status += "\n" + std::to_string(i) + " " + CImageBuffer::getBufferInstance().getBufferInfo(i)
                + " size:" + std::to_string(CImageBuffer::getBufferInstance().size(i))
                + " is full:" + std::to_string(CImageBuffer::getBufferInstance().isFull(i));
        if (buff.size(i) > 0)
        {
            const auto lastMat = buff.getLast(i);
            if (lastMat)
            {
                status += " last dt:" + lastMat->getDateTimeWithMS();
            }
        }
    }

    CTelegramAlertNotification::getInstance(*CConfig::getInstance().mCommonAlertConfig->mTelegramConfig)->
            getBot()->getApi().sendMessage(message->chat->id, status);
}

void setHomeIn(const TgBot::Message::Ptr& message)
{
    const auto bot = CTelegramAlertNotification::getInstance(
                *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig);
    if (!bot->isUserInVerifyList(message))
    {
        return;
    }
    CConfig::getInstance().mRunStatus.mHomeStatus = CRunStatus::HomeStatus::HOME_IN;
    CConfig::getInstance().mRunStatus.mIsRunMotionAlgo = false;
    const std::string infoMessage = "Text & Motion Systems disable on " + CDateTime::getDateTimeAsString()
            + "\n" + sCommands;
    sendMessageToAll(bot->getBot(), bot->getConfig(), infoMessage, message->chat->id);
}

void setHomeOut(const TgBot::Message::Ptr& message)
{
    const auto bot = CTelegramAlertNotification::getInstance(
                *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig);
    if (!bot->isUserInVerifyList(message))
    {
        return;
    }

    CConfig::getInstance().mRunStatus.mHomeStatus = CRunStatus::HomeStatus::HOME_OUT;
    CConfig::getInstance().mRunStatus.mIsRunMotionAlgo = true;
    const std::string infoMessage = "Text & Motion Systems enable on " +CDateTime::getDateTimeAsString()
            + "\n" + sCommands;
    sendMessageToAll(bot->getBot(), bot->getConfig(), infoMessage, message->chat->id);
}

void writeToSerial(const TgBot::Message::Ptr& message)
{
    if (!CTelegramAlertNotification::getInstance(*CConfig::getInstance().mCommonAlertConfig->mTelegramConfig)
            ->isUserInVerifyList(message))
    {
        return;
    }
    for (const auto& port : NSerial::CSerialPorts::getInstance().getPorts())
    {
        LOG_WARN << message->text << &port;
    }
}

void botLoop(std::shared_ptr<TgBot::Bot> bot)
{
    try {
        TgBot::TgLongPoll longPoll(*bot);
        while (1)
        {
            longPoll.start();
        }
    } catch (const std::exception& e)
    {
        LOG_WARN << "error " << e.what();
    }
}

void help(const TgBot::Message::Ptr& message)
{
    const auto bot = CTelegramAlertNotification::getInstance(
                *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig);
    if (!bot->isUserInVerifyList(message))
    {
        return;
    }

    CTelegramAlertNotification::getInstance(
        *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig)->getBot()->getApi().
            sendMessage(message->chat->id, sCommands);
}

void motionAlarmOn(const TgBot::Message::Ptr& message)
{
    const auto bot = CTelegramAlertNotification::getInstance(
                *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig);
    if (!bot->isUserInVerifyList(message))
    {
        return;
    }
    CConfig::getInstance().mRunStatus.mIsRunMotionAlgo = true;
    const std::string infoMessage = "Motion System enable on " +CDateTime::getDateTimeAsString()
            + "\n" + sCommands;;
    sendMessageToAll(bot->getBot(), bot->getConfig(), infoMessage, message->chat->id);
}

void motionAlarmOff(const TgBot::Message::Ptr& message)
{
    const auto bot = CTelegramAlertNotification::getInstance(
                *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig);
    if (!bot->isUserInVerifyList(message))
    {
        return;
    }
    CConfig::getInstance().mRunStatus.mIsRunMotionAlgo = false;
    const std::string infoMessage = "Motion System disable on " +CDateTime::getDateTimeAsString()
            + "\n" + sCommands;;
    sendMessageToAll(bot->getBot(), bot->getConfig(), infoMessage, message->chat->id);
}

} //namespace
#endif

CTelegramAlertNotification::CTelegramAlertNotification(
        const CTelegramAlertNotificationConfig& config)
    : mConfig(config)
    , mLastSendTime(CDateTime::getSecondsSinceEpoch())
    , mIsFirstTime(true)
{
#ifdef __BUILD_TELEGRAM_BOT__
    mBot.reset(new TgBot::Bot(config.mToken));
    if (!mConfig.mToken.empty())
    {
        mBot->getEvents().onCommand({"g", "G", "get", "Get"},GetAllImages);
        mBot->getEvents().onCommand({"g", "G", "get", "Get"},GetAllImages);
        mBot->getEvents().onCommand({"write", "Write"}, writeToSerial);

        mBot->getEvents().onCommand({"HomeIn", "homeIn", "setHomeIn", "SetHomeIn"}, setHomeIn);
        mBot->getEvents().onCommand({"HomeOut", "homeOut", "setHomeout", "SetHomeOut"}, setHomeOut);

        mBot->getEvents().onCommand({"Stat", "stat", "Status", "status", "s", "S"}, getStatus);
        mBot->getEvents().onCommand({"h", "H", "help", "Help", "start"}, help);

        mBot->getEvents().onCommand({"MotionOff", "motionOff"}, motionAlarmOff);
        mBot->getEvents().onCommand({"MotionOn", "motionOn"}, motionAlarmOn);
        mBot->getEvents().onUnknownCommand(help);
        mBot->getEvents().onNonCommandMessage(help);

        std::thread bootThreadLoop(botLoop, mBot);
        bootThreadLoop.detach();

        sendMessageToAll(mBot, mConfig, "System start on " + CDateTime::getDateTimeAsString()
                         + "\n\n" + sCommands, 0, true);
    }
#endif
}

const CTelegramAlertNotificationConfig& CTelegramAlertNotification::getConfig() const
{
    return mConfig;
}

std::shared_ptr<TgBot::Bot> CTelegramAlertNotification::getBot() const
{
    return mBot;
}

std::shared_ptr<CTelegramAlertNotification> CTelegramAlertNotification::getInstance(const CTelegramAlertNotificationConfig& config)
{
    static std::shared_ptr<CTelegramAlertNotification> instance(
                new CTelegramAlertNotification(config));
    return instance;
}

void CTelegramAlertNotification::onAlert(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        const std::string& alertAlgorithm,
        const std::string& alertInfo)
{
#ifdef __BUILD_TELEGRAM_BOT__
    if (!mBot)
    {
        LOG_WARN << "! bot ";
        return;
    }
    auto diffTime = CDateTime::getSecondsSinceEpoch() - mLastSendTime;
    if (mIsFirstTime && (diffTime > mConfig.mFirstTimeSkip))
    {
        mIsFirstTime = false;
    }

    if ((diffTime > mConfig.mMinIntervalRepeatSend)
            && !mIsFirstTime)
    {
        mLastSendTime = CDateTime::getSecondsSinceEpoch();

        std::thread thread(
                    sendAlertAsync,
                    mBot,
                    std::cref(mConfig),
                    buff,
                    bufferIds,
                    alertAlgorithm,
                    alertInfo);
        thread.detach();
    }
#endif
}

bool CTelegramAlertNotification::isUserInVerifyList(std::shared_ptr<TgBot::Message> message) const
{
#ifdef __BUILD_TELEGRAM_BOT__
    if (std::find(mConfig.mVerifyUsers.begin(), mConfig.mVerifyUsers.end(), message->chat->id)
            != mConfig.mVerifyUsers.end())
    {
        return true;
    }
    LOG_WARN << "user not found " << message->chat->id << message->from->id;
    CTelegramAlertNotification::getInstance(
        *CConfig::getInstance().mCommonAlertConfig->mTelegramConfig)->getBot()->getApi().
            sendMessage(message->chat->id, "You not allowed to send query, your id is " +
                        std::to_string(message->chat->id));
#endif
    return false;
}

//void CTelegramAlertNotification::checkAndSendForce(const CImageBuffer&, const std::vector<size_t>&)
//{
//}

}
