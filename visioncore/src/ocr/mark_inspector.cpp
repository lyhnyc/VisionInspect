// mark_inspector.cpp —— 标记检测（D4 完成实现；当前为可编译占位）
#include "ocr/mark_inspector.h"

namespace vc {

bool MarkInspector::inspect(const cv::Mat& roi, MarkResult& out) {
    // TODO(D4): OCR + DataMatrix 解码 + 连通域投影判字符残缺 + 模板位置比对判错位
    (void)roi; (void)out;
    return false;
}

} // namespace vc
