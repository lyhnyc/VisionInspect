// blob_detector.h —— BLOB 缺陷检测（连通域）
#pragma once
#include <opencv2/core.hpp>
#include <vector>

namespace vc {

struct Blob {
    cv::Point2f center;
    cv::Rect    bbox;
    float       area = 0;
    int         type = 0;   // 0=未知 1=划痕 2=缺失 3=污点
};

// 依据长宽比/细长度做粗分类：划痕（细长）、缺失（大面积实心）、污点（其余）
class BlobDetector {
public:
    // 光场校正 + 自适应阈值 + 形态学 + 连通域
    void detect(const cv::Mat& gray, float min_area, float max_area,
                std::vector<Blob>& out) const;

    // 暴露中间步骤供测试：阈值化
    cv::Mat binarize(const cv::Mat& gray) const;
};

} // namespace vc
