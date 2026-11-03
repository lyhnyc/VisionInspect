// 模块占位实现（D2/D3 逐个填充，保持 CI 随时可编译可测试）
#include "locate/template_locator.h"
#include "measure/caliper.h"
#include "ocr/mark_inspector.h"

namespace vc {

// ---------- TemplateLocator ----------
bool TemplateLocator::setTemplate(const cv::Mat& tpl) {
    if (tpl.empty()) return false;
    _tpl = tpl.clone();
    return true;
}

bool TemplateLocator::locate(const cv::Mat& image, cv::Point2f& dx_dy,
                             double& theta_deg, double& score) const {
    // TODO(D3): matchTemplate TM_CCOEFF_NORMED + 峰值抛物线亚像素拟合
    //           旋转扫描版：θ ∈ [-8°, 8°] 步进 0.5°，取最高分
    return false;
}

// ---------- Caliper（真实实现见 measure/caliper.cpp） ----------

// ---------- MarkInspector（D4 填充） ----------
bool MarkInspector::inspect(const cv::Mat& roi, MarkResult& out) {
    // TODO(D4): OCR + DMC 解码 + 字符残缺/错位判定
    return false;
}

} // namespace vc
