// template_locator.cpp —— 模板定位（D3 完成实现；当前为可编译占位）
#include "locate/template_locator.h"
#include <opencv2/imgproc.hpp>

namespace vc {

bool TemplateLocator::setTemplate(const cv::Mat& tpl) {
    if (tpl.empty()) return false;
    _tpl = tpl.clone();
    return true;
}

bool TemplateLocator::locate(const cv::Mat& image, cv::Point2f& dx_dy,
                             double& theta_deg, double& score) const {
    // TODO(D3): TM_CCOEFF_NORMED + 3x3 抛物线亚像素峰 + 旋转扫描 [-8°, 8°] 步进 0.5°
    (void)image; (void)dx_dy; (void)theta_deg; (void)score;
    return false;
}

} // namespace vc
