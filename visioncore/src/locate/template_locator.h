// 剩余 C++ 头文件骨架占位（D3/D4 填充实现）
#pragma once
#include <opencv2/core.hpp>

namespace vc {

// 模板定位：matchTemplate + 亚像素抛物线拟合
class TemplateLocator {
public:
    bool setTemplate(const cv::Mat& tpl);
    // 在 image 中定位模板，返回平移与旋转（先支持平移+小角度，旋转扫描版 D3 加）
    bool locate(const cv::Mat& image, cv::Point2f& dx_dy,
                double& theta_deg, double& score) const;
private:
    cv::Mat _tpl;
};

} // namespace vc
