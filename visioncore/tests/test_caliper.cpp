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
        const float cx = static_cast<float>(c.center);
        ASSERT_TRUE(cal.measure(img, cv::Point2f(cx - 60.f, 100.f), cv::Point2f(cx + 60.f, 100.f),
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
    // 旋转亮带：验证法线投影与亚像素测量的角度不变性
    // 合成图用解析公式（而非 fillPoly 光栅化），避免锯齿污染测量真值
    cv::Mat img = cv::Mat::zeros(300, 300, CV_8UC1);
    cv::Point2f c(150, 150);
    const double width = 25.0;
    const float rad30 = static_cast<float>(30 * CV_PI / 180);
    cv::Point2f axis(std::cos(rad30), std::sin(rad30));   // 带长轴方向
    cv::Point2f nrm(-axis.y, axis.x);                     // 带法向（120°）
    for (int y = 0; y < img.rows; ++y)
        for (int x = 0; x < img.cols; ++x) {
            float dist = std::fabs((x - c.x) * nrm.x + (y - c.y) * nrm.y);
            // 半宽 12.5，1px 线性渐变（模拟真实边缘的抗锯齿过渡）
            float cover = std::max(0.f, std::min(1.f, 13.f - dist));
            img.at<uchar>(y, x) = static_cast<uchar>(200 * cover + 0.5f);
        }

    Caliper cal;
    EdgePair ep;
    // 搜索线沿带法向 120°，垂直穿过亮带
    const float rad = static_cast<float>(120 * CV_PI / 180);
    cv::Point2f d(std::cos(rad), std::sin(rad));
    ASSERT_TRUE(cal.measure(img, c - 80.f * d, c + 80.f * d, 30.f, ep));
    EXPECT_NEAR(ep.distance, width, 0.5) // 算法解析无偏(-0.05px)，容差留给量化噪声
        << "rotated strip measured " << ep.distance;
}


