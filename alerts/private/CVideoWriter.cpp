
#include <opencv2/imgproc/imgproc.hpp>
#include <thread>

#include "CVideoWriter.hpp"
#include "Image/CImageBuffer.hpp"
#include "Image/CMatWithTimeStamp.hpp"
#include "alerts/CSMTPAlertConfig.hpp"
#include "common/CMainFunctions.hpp"
#include "logging/Log.hpp"

const int CVideoWriter::sVideoType = CV_FOURCC('D', 'I', 'V', 'X');

//    if (isWriteMaskAndLastImageToFile && config.mIsSaveMaskAndLastImageToVideoPath)
//    {
//        if (!matMask.empty())
//        {
//            cv::imwrite(name + "_mask.jpg", matMask);
//        }
//        cv::imwrite(name + "_1.jpg", list[list.size() -1]->getMat());
//        cv::imwrite(name + "_2.jpg", list[list.size() -2]->getMat());
//    }


bool CVideoWriter::writeVideo(
        const std::string& name,
        const CImageBuffer& buffer,
        size_t bufferId,
        const std::string& info,
        const NAlarms::CSMTPAlertConfig& config,
        bool isWriteMaskAndLastImageToFile)
{
    if (!buffer.isBufferExistAndFull(bufferId))
    {
        return false;
    }
    cv::VideoWriter outputVideo;
    try
    {

        const auto lastMatPtr =buffer.getLast(bufferId);
        if (lastMatPtr)
        {
            auto videoSize = lastMatPtr->getMat().size();
            if (!outputVideo.open(name, CVideoWriter::sVideoType, config.mVideoFps, videoSize, true))
            {
                LOG_WARN << "fail save video" << name;
                return false;
            }

            const auto& list = buffer.getList(bufferId);
            const auto& longList = buffer.getLongList(bufferId);
            for (const auto matPtr : longList)
            {
                outputVideo << matPtr->getMat(ImageType::IMAGE_RGB_FULL_WITH_TEXT);
            }
            pMatWithTimeStamp pMat;
            for (const auto matPtr : list)
            {
                outputVideo << matPtr->getMat(ImageType::IMAGE_RGB_FULL_WITH_TEXT);
                pMat = matPtr; // TODO: del pMat, use lastMatPtr
            }
            outputVideo << lastMatPtr->getMat(ImageType::IMAGE_RGB_FULL);

            if (config.mMaskCountToVideo > 0)
            {
                int imageId = list.size() - config.mMaskCountToVideo;
                if (imageId < 0)
                {
                    imageId = 0;
                }

                for (; imageId < list.size(); imageId++)
                {
                    auto const matPtr = list.at(imageId);
                    if(matPtr->hasMask())
                    {
                        const cv::Mat mask = matPtr->getMask();
                        if (!mask.empty() && CMainFunctions::isMatTypeSame(mask, pMat->getMask()))
                        {
                            outputVideo << mask;
                        }
                    }
                }
            }
            const cv::Mat matMask = lastMatPtr->getMask();
            if (!matMask.empty() && CMainFunctions::isMatTypeSame(matMask, pMat->getMask()))
            {
                outputVideo << matMask;
            }

            if (!info.empty())
            {
                cv::Mat pic = cv::Mat::zeros(pMat->getMat().rows, pMat->getMat().cols, pMat->getMat().type());

                int lineId = 0;
                int fontFace = CV_FONT_HERSHEY_SIMPLEX;
                double fontScale = 0.7;
                int thickness = 1;
                int baseline = 0;
                int offset = 5;

                std::stringstream ss(info);
                std::string line;
                cv::Size textSize = cv::getTextSize(line, fontFace, fontScale, thickness, &baseline);
                while(std::getline(ss, line,'\n'))
                {
                    cv::putText(pic,
                                line,
                                cv::Point(20, 20 + lineId* (textSize.height + offset)),
                                fontFace,
                                fontScale,
                                cv::Scalar(255, 255, 255),
                                thickness,
                                8,
                                false);
                    lineId ++;
                }

                outputVideo << pic;
                outputVideo << pic;
            }
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERROR << "\n\n\n\ncatch exception" << exc.what();
        return false;
    }
    catch (...)
    {
        LOG_ERROR << "\n\n\n\ncatch exception: unknown error";
        return false;
    }
    outputVideo.release();
    return true;
}
