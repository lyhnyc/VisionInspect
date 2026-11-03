// mark_inspector.h —— 标记检测（OCR + DMC + 质量判定）
#pragma once
#include <opencv2/core.hpp>
#include <string>

namespace vc {

struct MarkResult {
    std::string ocr_text;     // OCR 识别内容
    bool dmc_ok = false;      // DataMatrix 解码成功
    bool char_defect = false; // 字符残缺
    bool char_shift  = false; // 字符错位
    float confidence = 0;
};

class MarkInspector {
public:
    // roi: 标记区（已定位旋正）；判定字符残缺用连通域投影，错位用模板字符位置比对
    bool inspect(const cv::Mat& roi, MarkResult& out);
};

} // namespace vc
