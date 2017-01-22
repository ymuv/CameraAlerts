#include "Image/CImageBuffer.hpp"
#include "Image/videoInput/CInputConfig.hpp"
#include "Image/videoInput/CVideoCapture.hpp"
#include "logging/Log.hpp"

namespace NVideoInput
{
CVideoCapture::CVideoCapture(const CInputConfig& inputConfig)
    : mInputConfig(inputConfig)
    , mImageBuffer(CImageBuffer::getBufferInstance())
{
    size_t bufferSize = 0;

    for (const auto& entry: mInputConfig.mInputList)
    {
        if (entry.mType == InputType::CAMERA)
        {
            mInputCaptures.push_back(cv::VideoCapture(entry.mCameraId));
            //TODO: mBufferSize to int
            mImageBuffer.resizeBuffersToSize(bufferSize + 1, entry.mBufferSize, entry.mLongBufferSize);

            mImageBuffer.setBufferInfo(bufferSize, "cam" + std::to_string(entry.mCameraId));
            if (entry.mFrameHeight > 0 && entry.mFrameWidth > 0)
            {
                mInputCaptures[mInputCaptures.size() - 1].
                        set(CV_CAP_PROP_FRAME_HEIGHT, entry.mFrameHeight);

                mInputCaptures[mInputCaptures.size() - 1].
                        set(CV_CAP_PROP_FRAME_WIDTH, entry.mFrameWidth);
            }
            bufferSize++;
        }
        else if (entry.mType == InputType::FILE)
        {
            static int id = 0;
            mInputCaptures.push_back(cv::VideoCapture());
            mImageBuffer.resizeBuffersToSize(bufferSize + 1, entry.mBufferSize, entry.mLongBufferSize);
            mImageBuffer.setBufferInfo(bufferSize, "file" + std::to_string(id++));
            bufferSize++;
        }
        else if (entry.mType == InputType::URL)
        {
            //mInputCaptures.push_back(cv::VideoCapture()); //stub for mInputCaptures.size == CInputConfig.size
        }
    }
    mBufferSizeWithoutConnection = mInputCaptures.size();

}

bool CVideoCapture::open(const size_t bufferId)
{
    if (bufferId >= mInputConfig.mInputList.size())
    {
        LOG_WARN << "buffer ID out of range " << bufferId << "; must be 0.."
                 << mInputConfig.mInputList.size();
        return false;
    }
    const CInputConfigEntry& entry = mInputConfig.mInputList[bufferId];
    if (entry.mType == InputType::URL)
    {
        LOG_WARN << "try open url. FIX IT";
        return false;
    }
    else if (entry.mType == InputType::CAMERA)
    {
        if (mInputCaptures[bufferId].isOpened())
        {
            if (!mInputCaptures[bufferId].open(entry.mCameraId))
            {
                LOG_FATAL  << "Could not open the input camera with id" << bufferId
                           << mImageBuffer.getBufferInfo(bufferId);
                return false;
            }
        }
    }
    else if (entry.mType == InputType::FILE)
    {
        if (!mInputCaptures[bufferId].isOpened())
        {
            try
            {
                mInputCaptures[bufferId].open(entry.mFile);

                if (!mInputCaptures[bufferId].open(entry.mFile))
                {
                    LOG_FATAL  << "Could not open the input file with id" << bufferId
                               << mImageBuffer.getBufferInfo(bufferId);
                    return false;
                }
            }
            catch (const std::exception& exc)
            {
                LOG_WARN << "exception occurred;" << exc.what() << "\n"
                         << "fail open file " << entry.mFile;
                return false;
            }
            catch (...)
            {
                LOG_WARN << "unknown exception occurred; fail open file " << entry.mFile;
                return false;
            }
        }
    }

    return true;
}

bool CVideoCapture::processNextFrame(const size_t bufferId)
{
    if (!mInputCaptures[bufferId].isOpened())
    {
        if (!open(bufferId))
            return false;
    }
    cv::Mat mat;
    mInputCaptures[bufferId] >> mat;
    if (!mat.empty())
    {
        size_t imageId = mImageBuffer.getNextId(bufferId);

        mImageBuffer.push_back(
                    bufferId,
                    std::make_shared<CMatWithTimeStamp>(
                        mat,
                        mInputConfig.mInputList[bufferId].mResizeScaleFactor,
                        mInputConfig.mInputList[bufferId].mResizeScaleFactorForNet,
                        imageId));
        return true;
    }
    else
    {
        LOG_ERROR << "img is empty for buffId" << bufferId;
        return false;
    }
}

CVideoCapture::~CVideoCapture()
{
    for (size_t i = 0; i < mInputCaptures.size(); i++)
    {
        mInputCaptures[i].release();
    }
}

size_t CVideoCapture::getBufferSizeWithoutConnection() const
{
    return mBufferSizeWithoutConnection;
}
}
