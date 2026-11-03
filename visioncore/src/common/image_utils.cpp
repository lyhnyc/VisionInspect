// image_utils.cpp —— 图像工具函数（光场校正等）
#include "common/image_utils.h"
#include <opencv2/imgproc.hpp>

namespace vc {

cv::Mat flattenIllumination(const cv::Mat& gray, int blurKsize) {
    CV_Assert(gray.type() == CV_8UC1 || gray.type() == CV_32FC1);
    cv::Mat bg, out;
    cv::GaussianBlur(gray, bg, cv::Size(blurKsize, blurKsize), 0);
    // 背景除法：I_out = I / bg * 255
    cv::Mat g32, bg32;
    gray.convertTo(g32, CV_32F);
    bg.convertTo(bg32, CV_32F);
    cv::divide(g32, bg32, out, 255.0);
    out.convertTo(out, gray.type());
    return out;
}

} // namespace vc
