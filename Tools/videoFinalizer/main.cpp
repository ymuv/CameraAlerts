#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
    using namespace std;
    if (argc != 2){
        std::cout << "usagge: appName video.avi" << std::endl;
        return -1;
    }
    std::string file = argv[1];



    cv::VideoCapture capture(file);

    if (!capture.isOpened())
    {
        std::cout << "fail open input" << std::endl;
        return -1;
    }
    cv::Mat mat;
    capture >> mat;

    int videoType =  CV_FOURCC('D', 'I', 'V', '3');

    double fps = capture.get(CV_CAP_PROP_FPS);

    cv::VideoWriter outVideo;
    if (!outVideo.open(
                file +  "_out.avi",
                videoType,
                fps,
                mat.size(),
                true))
    {
        std::cout << "fail open video out file" << std::endl;
        return 1;
    }
    outVideo << mat;
    bool isStop = false;

    while (!isStop)
    {
        capture >> mat;
        if (mat.empty())
        {
            isStop = true;
        }
        else
        {
            outVideo << mat;
        }
    }
    outVideo.release();
    return 0;
}
