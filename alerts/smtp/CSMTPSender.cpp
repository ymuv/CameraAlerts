#include <memory>

#include "CSMTPSender.hpp"
#include "alerts/smtp/SmtpClient-for-Qt/src/SmtpMime"
#include "logging/Log.hpp"

namespace NAlarms
{

CSMTPSender::CSMTPSender(const std::string& runDateTime)
    : mRunDateTime(runDateTime)
{
}

bool CSMTPSender::isConfigFull(const CSMTPAlertConfig& smtpConfig)
{
    if (smtpConfig.mReceiverEMails.empty() ||
            smtpConfig.mSMTPHost.isEmpty() ||
            smtpConfig.mUserName.isEmpty() ||
            smtpConfig.mUserPassword.isEmpty())
    {
        LOG_DEBUG << "SMTP_NO_DATA"
                 << "mail list empty:" << smtpConfig.mReceiverEMails.empty()
                 << "; smpt host empty:" << smtpConfig.mSMTPHost.isEmpty()
                 << "; user name empty:" << smtpConfig.mUserName.isEmpty()
                 << "; user pass empty:" << smtpConfig.mUserPassword.isEmpty();
        return false;
    }
    return true;
}

SMTPResult CSMTPSender::sendEmail(
        const CSMTPAlertConfig& smtpConfig,
        const std::vector<std::string>& fileNames,
        const std::string& alertName,
        const std::string& dateTime,
        const std::string& subjectInfoStr,
        const std::string& infoStr,
        std::string& errorString
        ) const
{
   errorString.clear();
   if (!isConfigFull(smtpConfig))
    {
       return SMTPResult::SMTP_NO_DATA;
    }
     SmtpClient smtp(smtpConfig.mSMTPHost,
                     smtpConfig.mSMTPPort,
                     SmtpClient::TlsConnection);

     smtp.setUser(smtpConfig.mUserName);
     smtp.setPassword(smtpConfig.mUserPassword);

     // Now we create a MimeMessage object. This is the email.

     MimeMessage message;

     EmailAddress sender(smtpConfig.mUserName, smtpConfig.mUserName);
     message.setSender(&sender);

     std::vector<std::shared_ptr<EmailAddress>> toList;
     for (int i = 0; i < smtpConfig.mReceiverEMails.size(); i++)
     {
         std::shared_ptr<EmailAddress> to (new EmailAddress(smtpConfig.mReceiverEMails[i]));
         toList.push_back(to);

        message.addRecipient(to.get());
     }

     QString subject = QString::fromStdString(
                 smtpConfig.mSubjectPrefix + alertName + "_" + subjectInfoStr + "_"+ dateTime);
     message.setSubject(subject);

     message.setInReplyTo(QString::fromStdString(mRunDateTime + "@" + smtpConfig.mComputerName));

     MimeText text;

     text.setText(QString::fromStdString(infoStr));
     message.addPart(&text);

     //files
     std::vector<std::shared_ptr<MimeAttachment>> attachments;

     for (const std::string& fileName: fileNames)
     {
         std::shared_ptr<MimeAttachment> attachment(
                     new MimeAttachment(new QFile(QString::fromStdString(fileName))));
         attachments.push_back(attachment);
         LOG_DEBUG << "add files " << fileName;
         message.addPart(attachment.get());
     }

     if (!smtp.connectToHost()) {
         errorString = "SMTP_CONNECT_FAIL Failed to connect to host! code:" + std::to_string(smtp.getResponseCode());
         LOG_DEBUG << errorString;

         return SMTPResult::SMTP_CONNECT_FAIL;
     }

     if (!smtp.login()) {
         errorString = "SMTP_LOGIN_FAIL Failed to login! code:" + std::to_string(smtp.getResponseCode());
         LOG_DEBUG <<  errorString;
         return SMTPResult::SMTP_LOGIN_FAIL;
     }

     if (!smtp.sendMail(message)) {
         errorString = "SMTP_SEND_FAIL Failed to send mail! code:" + std::to_string(smtp.getResponseCode());
         LOG_DEBUG << errorString;

         return SMTPResult::SMTP_SEND_FAIL;
     }

     smtp.quit();
     LOG_DEBUG << "send ok";

     return SMTPResult::SMTP_SEND_OK;

}
}
