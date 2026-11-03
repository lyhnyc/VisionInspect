// image_utils.h —— 图像工具函数
#pragma once
#include <opencv2/core.hpp>

namespace vc {

// 光场校正：高斯模糊估背景后做背景除法，消除照明不均
cv::Mat flattenIllumination(const cv::Mat& gray, int blurKsize = 51);

} // namespace vc
