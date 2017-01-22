#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "alerts/CFinalVideoWriter.hpp"
#include "alerts/CFinalVideoWriterConfig.hpp"
#include "common/CMainFunctions.hpp"
#include "logging/Log.hpp"

namespace NAlarms
{

const int CFinalVideoWriter::sVideoType =  CV_FOURCC('D', 'I', 'V', 'X');
//const int CFinalVideoWriter::sVideoType =  CV_FOURCC('D', 'I', 'V', '3');

std::shared_ptr<CFinalVideoWriter> CFinalVideoWriter::getInstance(const CFinalVideoWriterConfig& config)
{
    static std::shared_ptr<CFinalVideoWriter> instance(new CFinalVideoWriter(config));
    return instance;
}

CFinalVideoWriter::CFinalVideoWriter(const CFinalVideoWriterConfig& config)
    : mConfig(config)
{

}

void CFinalVideoWriter::writeVideo(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        const std::string& alertAlgorithm,
        const std::string& alertInfo,
        int newFfterAlertFrameCount)
{
    std::lock_guard<std::mutex> lock(mFinalVideoMutex);
    for (const auto bufferId : bufferIds)
    {
        for (size_t i = 0; i < mConfig.mSaveFinalVideoPaths.size(); i++)
        {
            const auto& path = mConfig.mSaveFinalVideoPaths[i];
            CMainFunctions::makeDirIfNotExist(path);

            if (!buff.isBufferExistAndFull(bufferId))
            {
                continue;
            }
            FinalVideosMap& fv = mFinalVideoMap[bufferId];
            if (newFfterAlertFrameCount >= 0)
            {
                fv.mAfterAlertFramesCount = newFfterAlertFrameCount;
            }
            FinalVideo& finaVideoStuct =  fv.mFinalVideoMap[path];
            static const std::string fileFormat = ".avi";
            auto lastMat = buff.getLast(bufferId);
            if (lastMat && lastMat->getId() > finaVideoStuct.mLastImageId)
            {
                CMatWithTimeStampLocker locker(lastMat);
                std::string videoFileName;
                if (!finaVideoStuct.mVideoWriter.isOpened() || finaVideoStuct.mDate != lastMat->getDate())
                {
                    try
                    {
                        finaVideoStuct.mVideoWriter.release();
                    } catch (const std::exception& exc)
                    {
                        LOG_WARN << "catch exception " << exc.what();
                    }

                    const auto pathWithDate = path + "/" +  lastMat->getDate();
                    CMainFunctions::makeDirIfNotExist(pathWithDate);

                    videoFileName = pathWithDate + "/"
                            + std::to_string(CMainFunctions::sPid) + "_"
                            + std::to_string(bufferId) + "_"
                            + buff.getBufferInfo(bufferId) + "__"
                            //+ std::to_string(bufferId) + "_"
                            + lastMat->getDateTimeWithMS()
                            + "_" + std::to_string(lastMat->getId())
                            + fileFormat;
                    finaVideoStuct.mDate = lastMat->getDate();
                }

                writeFinalVideo(
                            buff,
                            bufferId,
                            finaVideoStuct.mVideoWriter,
                            videoFileName,
                            finaVideoStuct.mLastImageId,
                            alertAlgorithm,
                            alertInfo);
                finaVideoStuct.mLastImageId = lastMat->getId();
            }
        }
    }
}

void CFinalVideoWriter::onAlert(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        const std::string& alertAlgorithm,
        const std::string& alertInfo)
{
    writeVideo(buff, bufferIds, alertAlgorithm, alertInfo, 0);
}

void CFinalVideoWriter::checkAndSendForce(
        const CImageBuffer&, const std::vector<size_t>&)
{
}

void CFinalVideoWriter::checkAndSendReply(
        const CImageBuffer& buff, const std::vector<size_t>& bufferIds)
{
    if (mConfig.mSaveFinalVideoPaths.empty())
    {
        return;
    }
    for (const auto bufferId : bufferIds)
    {
        FinalVideosMap& fv = mFinalVideoMap[bufferId];
        if (fv.mAfterAlertFramesCount < 0)
        {
            continue;
        }
        fv.mAfterAlertFramesCount++;
        if (fv.mAfterAlertFramesCount >= mConfig.mFrameAfterAlertToVideo)
        {
            fv.mAfterAlertFramesCount = -1;
        }

        for(auto& iterator : fv.mFinalVideoMap)
        {
            FinalVideo& finaVideoStuct =  iterator.second;
            if (buff.isFull(bufferId))
            {
                if (buff.getList(bufferId).last()->getId() > finaVideoStuct.mLastImageId)
                {
                    std::lock_guard<std::mutex> lock(mFinalVideoMutex);
                    //double check
                    if (buff.getList(bufferId).last()->getId() > finaVideoStuct.mLastImageId)
                    {
                        if (!buff.isBufferExistAndFull(bufferId))
                        {
                            continue;
                        }
                        writeFinalVideoAfter(buff, finaVideoStuct.mVideoWriter, bufferId);
                        finaVideoStuct.mLastImageId = buff.getList(bufferId).last()->getId();
                    }
                }
            }
        }
    }
}

bool CFinalVideoWriter::writeFinalVideo(
        const CImageBuffer& buff,
        size_t bufferId,
        cv::VideoWriter& video,
        const std::string& videoName,
        size_t lastWriterImageId,
        const std::string& algoName,
        const std::string& algoInfo)
{
    try
    {
        const auto buffer = buff.getList(bufferId);
        auto size = buffer.size();
        const auto frame = buffer[size - 1];
        if (!video.isOpened())
        {
            if (!video.open(
                        videoName,
                        sVideoType,
                        mConfig.mVideoFps,
                        frame->getMat().size(),
                        true))
            {
                LOG_WARN << "fail save video" << videoName;
                return false;
            }
        }
        {
            cv::Mat mat;
            int imageId = buffer.size() - mConfig.mFrameBeforeAlertToVideo;
            if (imageId < 0)
            {
               //write from long buffer
               const auto longList = buff.getLongList(bufferId);
               {
                  int longBufferImageId = longList.size() + imageId;
                  if (longBufferImageId < 0)
                  {
                     longBufferImageId = 0;
                     cv::Mat longMat;
                     for (; longBufferImageId < longList.size() - 1; longBufferImageId++)
                     {
                        const auto matPtr = longList[longBufferImageId];
                        if (matPtr)
                        {
                           CMatWithTimeStampLocker locker(matPtr);
                           if (matPtr && (matPtr->getId() > lastWriterImageId))
                           {
                              matPtr->getMat().copyTo(longMat);
                              CMatWithTimeStamp::writeText(
                                       longMat,
                                       "__fromLong_" + std::to_string(longList.size()  - longBufferImageId) + "__"
                                       + matPtr->getDateTimeWithMS(),
                                       std::to_string(matPtr->getId()),
                                       1);
                              video << longMat;
                           }
                        }
                     }
                  }
               }
               imageId = 0;
            }
            for (; imageId < buffer.size() - 1; imageId++)
            {
                auto const matPtr = buffer[imageId];
                if (matPtr && matPtr->getId() > lastWriterImageId)
                {
                    CMatWithTimeStampLocker locker(matPtr);
                    matPtr->getMat().copyTo(mat);
                    if (imageId !=  buffer.size() - 1)
                    {
                        CMatWithTimeStamp::writeText(
                                    mat,
                                    "__prev_" + std::to_string(buffer.size()  - imageId) + "__"
                                    + matPtr->getDateTimeWithMS(),
                                    std::to_string(matPtr->getId()));
                        video << mat;
                    }
                }
            }
            video << frame->getMat(ImageType::IMAGE_RGB_FULL_WITH_TEXT);
        }
        if (mConfig.mIsWriteMaskToFinalVideo)
        {
            const cv::Mat matMask = frame->getMask();
            if (!matMask.empty() &&
                    CMainFunctions::isMatTypeSame(matMask, frame->getMat()))
            {
                cv::Mat mat;
                matMask.copyTo(mat);
                CMatWithTimeStamp::writeText(
                            mat,
                            algoName  + "\n" + algoInfo,
                            std::to_string(frame->getId()));
                video << mat;
            }
            else
            {
                cv::Mat mat;
                frame->getMat().copyTo(mat);
                CMatWithTimeStamp::writeText(
                            mat,
                            algoName  + "\n" + algoInfo,
                            std::to_string(frame->getId()));
                video << mat;
            }
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERROR << "catch exception" << exc.what() << "when try process file " << videoName;
        return false;
    }

    catch (...)
    {
        LOG_ERROR << "catch exception: unknown error";
        return false;
    }
    return true;
}

void CFinalVideoWriter::writeFinalVideoAfter(const CImageBuffer& buff, cv::VideoWriter& video, size_t idsForWrite)
{
    try
    {
        if (!video.isOpened())
        {
            return;
        }
        if (buff.empty(idsForWrite))
        {
            return;
        }

        const auto matPtr= buff.getLast(idsForWrite);
        if (matPtr)
        {
            cv::Mat mat;

            matPtr->getMat().copyTo(mat);
            CMatWithTimeStamp::writeText(
                        mat,
                        "After " + std::to_string(idsForWrite) + "__" + matPtr->getDateTimeWithMS(),
                        std::to_string(matPtr->getId()));
            video << mat;
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERROR << "catch exception" << exc.what() << "when write video  " ;
    }
    catch (...)
    {
        LOG_ERROR << "catch exception: unknown error";
    }
}

void CFinalVideoWriter::onWarning(
        const CImageBuffer& buff,
        const std::vector<size_t>& bufferIds,
        const std::string& alertAlgorithm,
        const std::string& alertInfo)
{
    writeVideo(buff, bufferIds, alertAlgorithm, alertInfo, -1);
}
}
