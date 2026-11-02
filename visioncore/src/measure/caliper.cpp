// caliper.cpp —— 卡尺亚像素测量实现
// 核心思想：把二维边缘检测降维成一维投影轮廓的极值定位，
// 亚像素精度来自抛物线顶点插值（合成图上典型 <0.1px，见 tests/test_caliper.cpp）
#include "caliper.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

namespace vc {

namespace {

// 1D 投影轮廓中的最强上升+下降沿：返回两极值点亚像素索引
bool find_edge_pair(const std::vector<float>& prof, float& i_up, float& i_dn, float& strength) {
    if (prof.size() < 5) return false;
    // 平滑（3点盒滤）
    std::vector<float> s(prof.size());
    for (size_t i = 1; i + 1 < prof.size(); ++i)
        s[i] = 0.25f * prof[i-1] + 0.5f * prof[i] + 0.25f * prof[i+1];
    s.front() = prof.front(); s.back() = prof.back();

    // 一阶差分
    std::vector<float> d(s.size() - 1);
    float dmax_pos = 0, dmin_neg = 0;
    for (size_t i = 0; i < d.size(); ++i) {
        d[i] = s[i+1] - s[i];
        dmax_pos = std::max(dmax_pos, d[i]);
        dmin_neg = std::min(dmin_neg, d[i]);
    }
    if (dmax_pos < 8.0f || -dmin_neg < 8.0f) return false; // 边缘强度门限（0~255）

    int iu = static_cast<int>(std::max_element(d.begin(), d.end()) - d.begin());
    int id = static_cast<int>(std::min_element(d.begin(), d.end()) - d.begin());

    // 抛物线顶点插值：x* = i + 0.5*(d[i-1]-d[i+1])/(d[i-1]-2d[i]+d[i+1])
    auto vertex = [&](const std::vector<float>& g, int i) {
        if (i <= 0 || i + 1 >= static_cast<int>(g.size())) return static_cast<float>(i);
        const float a = g[i-1], b = g[i], c = g[i+1];
        const float denom = a - 2*b + c;
        if (std::fabs(denom) < 1e-6f) return static_cast<float>(i);
        return i + 0.5f * (a - c) / denom;
    };
    i_up = vertex(d, iu);
    i_dn = vertex(d, id);
    strength = 0.5f * (dmax_pos + -dmin_neg) / 255.0f;
    return true;
}

} // namespace

bool Caliper::measure(const cv::Mat& gray,
                      cv::Point2f p0, cv::Point2f p1, float band_width,
                      EdgePair& out) const {
    CV_Assert(gray.type() == CV_8UC1);
    cv::Point2f dir = p1 - p0;
    float len = static_cast<float>(cv::norm(dir));
    if (len < 2.0f || band_width < 2.0f) return false;
    dir /= len;
    cv::Point2f nrm(-dir.y, dir.x); // 法线

    // 沿线段采样 num_samples 个位置，每个位置沿法向取 band_width 均值 → 投影轮廓
    const int num_samples = std::max(16, static_cast<int>(len));
    const int half_band = std::max(2, static_cast<int>(band_width / 2));
    std::vector<float> prof(num_samples, 0.f);

    for (int i = 0; i < num_samples; ++i) {
        const float t = i / static_cast<float>(num_samples - 1);
        cv::Point2f base = p0 + t * len * dir;
        float acc = 0; int cnt = 0;
        for (int j = -half_band; j <= half_band; ++j) {
            cv::Point2f q = base + j * nrm;
            int x = cvRound(q.x), y = cvRound(q.y);
            if (x < 0 || y < 0 || x >= gray.cols || y >= gray.rows) continue;
            acc += gray.at<uchar>(y, x); ++cnt;
        }
        prof[i] = cnt ? acc / cnt : 0.f;
    }

    // 投影轮廓上找边缘对（注意：轮廓的极值对应边缘位置）
    float iu, idn, st;
    if (!find_edge_pair(prof, iu, idn, st)) return false;
    if (std::fabs(idn - iu) < 1.0f) return false;

    cv::Point2f e1 = p0 + (iu / static_cast<float>(num_samples - 1)) * len * dir;
    cv::Point2f e2 = p0 + (idn / static_cast<float>(num_samples - 1)) * len * dir;
    out.p1 = e1; out.p2 = e2;
    out.distance = std::fabs(idn - iu) * (len / (num_samples - 1));
    out.strength = st;
    return true;
}

} // namespace vc
