// test_caliper.cpp —— 卡尺亚像素精度的真值验证（D3 的核心证据）
// 方法：合成已知宽度的亮带，真值宽度精确已知；
//       验证 |测量宽度-真值| < 0.1px（亚像素 <1/10 像素）
#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>
#include "measure/caliper.h"

using namespace vc;

namespace {
// 生成宽 width_px 的垂直亮带（抗锯齿），亮度 contrast，背景 0
cv::Mat make_strip(int w, int h, double center_x, double width_px, uchar contrast) {
    cv::Mat img = cv::Mat::zeros(h, w, CV_8UC1);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            const double x0 = center_x - width_px / 2.0;
            const double x1 = center_x + width_px / 2.0;
            // 线性边缘过渡 → 真值边缘位置在 x0 与 x1
            const double cov = std::min(1.0, std::max(0.0, std::min(x + 1.0, x1) - std::max(double(x), x0)));
            img.at<uchar>(y, x) = cv::saturate_cast<uchar>(contrast * cov);
        }
    return img;
}
} // namespace

TEST(Caliper, SubPixelAccuracy_SyntheticStrip) {
    Caliper cal;
    struct Case { double center, width; };
    const Case cases[] = { {100.0, 20.0}, {100.3, 20.7}, {99.6, 15.2}, {101.1, 30.5} };
    for (const auto& c : cases) {
        cv::Mat img = make_strip(200, 200, c.center, c.width, 200);
        // 搜索线：水平穿过亮带中心
        EdgePair ep;
        ASSERT_TRUE(cal.measure(img, {c.center - 60.f, 100.f}, {c.center + 60.f, 100.f},
                                30.f, ep)) << "width=" << c.width;
        const double measured = ep.distance;
        EXPECT_NEAR(measured, c.width, 0.10)   // 亚像素精度要求：0.1px
            << "center=" << c.center << " width=" << c.width;
    }
}

TEST(Caliper, RejectsWeakEdges) {
    // 低对比度（<门限）时应返回 false，而不是输出垃圾值
    cv::Mat img = make_strip(200, 200, 100.0, 20.0, 10); // contrast=10 → 低于门限8*2?
    Caliper cal;
    EdgePair ep;
    // contrast 10 → 一阶差分幅度 ~5 < 8 门限 → 拒绝
    EXPECT_FALSE(cal.measure(img, {40.f, 100.f}, {160.f, 100.f}, 30.f, ep));
}

TEST(Caliper, RotatedStrip) {
    // 旋转亮带：验证法线投影的正确性（角度补偿）
    cv::Mat img = cv::Mat::zeros(240, 240, CV_8UC1);
    cv::Point2f c(120, 120);
    const double width = 25.0;
    cv::RotatedRect rr(c, cv::Size2f(200.f, static_cast<float>(width)), 30.f);
    cv::Mat mask = cv::Mat::zeros(240, 240, CV_8UC1);
    cv::Point2f vtx[4]; rr.points(vtx);
    cv::fillPoly(mask, std::vector<std::vector<cv::Point>>{{
        cv::Point(vtx[0]), cv::Point(vtx[1]), cv::Point(vtx[2]), cv::Point(vtx[3])}}, 200);
    img = cv::max(img, mask);

    Caliper cal;
    EdgePair ep;
    // 沿 30° 方向的搜索线
    cv::Point2f d(std::cos(30 * CV_PI / 180), std::sin(30 * CV_PI / 180));
    ASSERT_TRUE(cal.measure(img, c - 80.f * d, c + 80.f * d, 30.f, ep));
    EXPECT_NEAR(ep.distance, width, 0.5); // 旋转+离散采样放宽到 0.5px（正式版优化后收紧）
}
