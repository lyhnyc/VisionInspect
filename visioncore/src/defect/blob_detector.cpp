// blob_detector.cpp —— BLOB 缺陷检测实现
#include "blob_detector.h"
#include <opencv2/imgproc.hpp>

namespace vc {

cv::Mat BlobDetector::binarize(const cv::Mat& gray) const {
    CV_Assert(gray.type() == CV_8UC1);
    // 1) 光场校正：大核背景估计后做差（消除照明不均）
    cv::Mat bg, norm;
    cv::GaussianBlur(gray, bg, cv::Size(0, 0), 25);
    cv::absdiff(gray, bg, norm);
    cv::normalize(norm, norm, 0, 255, cv::NORM_MINMAX);

    // 2) 自适应阈值：对局部照度鲁棒
    cv::Mat bin;
    cv::adaptiveThreshold(norm, bin, 255,
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY_INV, 31, 10);

    // 3) 形态学开运算：去噪点、断开细连接
    cv::Mat k = cv::getStructuringElement(cv::MORPH_ELLIPSE, {3, 3});
    cv::morphologyEx(bin, bin, cv::MORPH_OPEN, k);
    return bin;
}

void BlobDetector::detect(const cv::Mat& gray, float min_area, float max_area,
                          std::vector<Blob>& out) const {
    out.clear();
    cv::Mat bin = binarize(gray);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(bin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& c : contours) {
        const double area = cv::contourArea(c);
        if (area < min_area || area > max_area) continue;

        cv::Rect bbox = cv::boundingRect(c);
        cv::Moments mu = cv::moments(c);
        Blob b;
        b.center = { static_cast<float>(mu.m10 / mu.m00),
                     static_cast<float>(mu.m01 / mu.m00) };
        b.bbox = bbox;
        b.area = static_cast<float>(area);

        // 粗分类：细长比（外接矩形长边/短边）
        const float w = static_cast<float>(bbox.width);
        const float h = static_cast<float>(bbox.height);
        const float aspect = std::max(w, h) / std::max(1.0f, std::min(w, h));
        const float fill  = static_cast<float>(area) / std::max(1.0f, w * h);
        if (aspect > 5.0f && fill < 0.4f)      b.type = 1; // 划痕
        else if (fill > 0.7f && area > 0.6f * max_area) b.type = 2; // 缺失
        else                                   b.type = 3; // 污点
        out.push_back(b);
    }
}

} // namespace vc
