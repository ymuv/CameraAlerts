#include <vector>

#include "common/CMainFunctions.hpp"
#include "common/CMatWithTimeStamp.hpp"
#include "common/CImageBuffer.hpp"
#include "common/CConfig.hpp"
#include "common/CAlertNotification.hpp"
#include "logging/Log.hpp"

static double scaleFactor = 1;
int main(int argc, char** argv)
{
    CMainFunctions::parseParams(argc, argv, "__INSTALL_BIN_DIR__""");

    auto &buffer = CImageBuffer::getBufferInstance();
    buffer.resizeBuffersToSize(1, argc - 1, 0, 0);
    auto& config = CConfig::getInstance();
    config.mAlertNotificationConfig->mTmpDir = "";
    config.mAlertNotificationConfig->mIsProcessFirstAlertOnly  = 1;

    CAlertNotification alert(*CConfig::getInstance().mAlertNotificationConfig, 0);
    std::vector<std::shared_ptr<NAlgorithms::IAlertAlgorithm>> algorithms;
    CMainFunctions::addAlgorithm(algorithms);
    for (int i = 1; i  < argc; i++)
    {
        cv::Mat image;
        image = cv::imread(argv[i]);
        pMatWithTimeStamp mat;
        mat.reset(new CMatWithTimeStamp(image, scaleFactor, 1, ImageType::IMAGE_RGB_FULL));
        buffer.push_back(0, mat);
    }
    if (buffer.size(0) > 0)
    {
        LOG_WARN << buffer.isFull(0);
        cv::Mat mask;
        CMainFunctions::doAlgo(alert, 0, algorithms, "");
        cv::imshow("mask", mask);
        cv::waitKey(1);
        cv::waitKey(0);
    }


    return 0;
}
