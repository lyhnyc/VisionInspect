// interop.cpp —— C ABI 导出实现（所有对外函数只在此文件）
// 内部实现类 VisionEngine 不跨边界，句柄即其指针的整数形式
#include "interop.h"
#include "defect/blob_detector.h"
#include "locate/template_locator.h"
#include "measure/caliper.h"
#include <opencv2/core.hpp>
#include <memory>
#include <mutex>

namespace {

struct VisionEngine {
    cv::Mat image;                       // 当前帧（灰度化后）
    cv::Mat templ;
    vc::BlobDetector blob;
    vc::TemplateLocator locator;
    std::mutex mtx;                      // 同一句柄串行化，C# 侧亦可并行多句柄
};

inline VisionEngine* E(long long h) {
    return reinterpret_cast<VisionEngine*>(h);
}
inline long long H(VisionEngine* e) {
    return reinterpret_cast<long long>(e);
}

} // namespace

// 注意：这里用 new/delete 裸指针而非 make_shared，因为句柄语义就是唯一所有权
VC_API long long VC_Create(int) {
    // D1修正：句柄全宽度返回，避免32位截断（教训见 interop.md）
    auto* e = new VisionEngine();
    return H(e);
}

VC_API void VC_Destroy(long long handle) {
    delete E(handle);
}

VC_API int VC_LoadImage(long long handle, const unsigned char* data,
                        int w, int h, int stride, int channels) {
    auto* e = E(handle);
    if (!e || !data || w <= 0 || h <= 0) return VC_ERR_PARAM;
    std::lock_guard<std::mutex> lk(e->mtx);
    int type = (channels == 1) ? CV_8UC1 : CV_8UC3;
    // 用外部分配构造 Mat 再 clone，避免悬垂（调用方可能立刻复用缓冲）
    cv::Mat src(h, w, type, const_cast<unsigned char*>(data), stride);
    if (channels == 3) cv::cvtColor(src, e->image, cv::COLOR_BGR2GRAY);
    else e->image = src.clone();
    return VC_OK;
}

VC_API int VC_SetTemplate(long long handle, const unsigned char* tpl, int w, int h) {
    auto* e = E(handle);
    if (!e || !tpl || w <= 0 || h <= 0) return VC_ERR_PARAM;
    std::lock_guard<std::mutex> lk(e->mtx);
    cv::Mat m(h, w, CV_8UC1, const_cast<unsigned char*>(tpl));
    e->templ = m.clone();
    return e->locator.setTemplate(e->templ) ? VC_OK : VC_ERR_PARAM;
}

VC_API int VC_Locate(long long handle, VcLocateResult* out) {
    auto* e = E(handle);
    if (!e || !out) return VC_ERR_PARAM;
    if (e->image.empty() || e->templ.empty()) return VC_ERR_IMAGE;
    std::lock_guard<std::mutex> lk(e->mtx);
    double score = 0;
    cv::Point2f dx_dy; double theta = 0;
    if (!e->locator.locate(e->image, dx_dy, theta, score)) return VC_ERR_NOT_FOUND;
    out->dx = dx_dy.x; out->dy = dx_dy.y;
    out->theta_deg = static_cast<float>(theta);
    out->score = static_cast<float>(score);
    return VC_OK;
}

VC_API int VC_DetectDefects(long long handle, float min_area, float max_area,
                            VcBlobResult* out_buf, int max_count, int* out_count) {
    auto* e = E(handle);
    if (!e || !out_buf || !out_count || max_count <= 0) return VC_ERR_PARAM;
    if (e->image.empty()) return VC_ERR_IMAGE;
    std::lock_guard<std::mutex> lk(e->mtx);
    std::vector<vc::Blob> blobs;
    e->blob.detect(e->image, min_area, max_area, blobs);
    int n = std::min<int>(static_cast<int>(blobs.size()), max_count);
    for (int i = 0; i < n; ++i) {
        out_buf[i].center = { blobs[i].center.x, blobs[i].center.y };
        out_buf[i].width  = blobs[i].bbox.width;
        out_buf[i].height = blobs[i].bbox.height;
        out_buf[i].area   = blobs[i].area;
        out_buf[i].type   = blobs[i].type;
    }
    *out_count = n;
    return VC_OK;
}

VC_API int VC_MeasureCaliper(long long handle,
                             float p0x, float p0y, float p1x, float p1y,
                             float width, VcMeasureResult* out) {
    auto* e = E(handle);
    if (!e || !out) return VC_ERR_PARAM;
    if (e->image.empty()) return VC_ERR_IMAGE;
    std::lock_guard<std::mutex> lk(e->mtx);
    vc::EdgePair ep;
    if (!e->caliper.measure(e->image,
                            {p0x, p0y}, {p1x, p1y}, width, ep))
        return VC_ERR_NOT_FOUND;
    out->p1 = { ep.p1.x, ep.p1.y };
    out->p2 = { ep.p2.x, ep.p2.y };
    out->distance_px = ep.distance;
    out->edge_strength = ep.strength;
    return VC_OK;
}

VC_API const char* VC_Version() {
    return "visioncore 0.1.0";
}
