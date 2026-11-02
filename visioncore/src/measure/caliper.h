// caliper.h —— 卡尺亚像素边缘测量
#pragma once
#include <opencv2/core.hpp>

namespace vc {

struct EdgePair {
    cv::Point2f p1;        // 亚像素边缘点1
    cv::Point2f p2;
    float distance = 0;    // 两边缘沿法线距离（像素）
    float strength = 0;    // 平均边缘强度（0~255 归一）
};

// 经典工业卡尺：沿 p0→p1 线段法向做宽度为 w 的投影带，
// 求投影轮廓的亚像素极值点（一阶导零交叉/抛物线插值）
class Caliper {
public:
    // 返回 false 表示未找到有效边缘对
    bool measure(const cv::Mat& gray,
                 cv::Point2f p0, cv::Point2f p1, float band_width,
                 EdgePair& out) const;
};

} // namespace vc
