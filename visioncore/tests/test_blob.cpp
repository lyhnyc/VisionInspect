// test_blob.cpp —— BLOB 缺陷检测的真值验证（合成缺陷图）
#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>
#include "defect/blob_detector.h"

using namespace vc;

namespace {
// 背景纹理（模拟照明不均）+ 已知缺陷
cv::Mat make_scene(cv::Size sz, const cv::Rect& defect_rect, int defect_type) {
    cv::Mat img(sz, CV_8UC1);
    // 背景加渐变光照
    for (int y = 0; y < sz.height; ++y)
        for (int x = 0; x < sz.width; ++x)
            img.at<uchar>(y, x) = cv::saturate_cast<uchar>(120 + 30 * std::sin(x / 50.0));
    cv::Mat roi = img(defect_rect);
    switch (defect_type) {
        case 1: roi.setTo(40);  break;   // 暗块（模拟污点/缺失）
        case 2:                                  // 划痕：细亮线
            cv::line(img, defect_rect.tl(), defect_rect.br(), cv::Scalar(10), 2);
            break;
        default: roi.setTo(200); break;
    }
    return img;
}
} // namespace

TEST(Blob, DetectsKnownDefectAndClassifies) {
    BlobDetector det;
    cv::Rect gt(80, 60, 12, 60);   // 细长缺陷 → 划痕
    cv::Mat img = make_scene({200, 200}, gt, 2);

    std::vector<Blob> blobs;
    det.detect(img, 20.f, 5000.f, blobs);
    ASSERT_FALSE(blobs.empty());
    bool hit = false;
    for (const auto& b : blobs) {
        // 中心落在真值框附近（容差 3px）
        if (std::fabs(b.center.x - (gt.x + gt.width / 2.0)) < 3 &&
            std::fabs(b.center.y - (gt.y + gt.height / 2.0)) < 3) {
            hit = true;
            EXPECT_GT(b.area, 20.f);
        }
    }
    EXPECT_TRUE(hit) << "未在真值位置检出缺陷";
}

TEST(Blob, NoFalsePositive_OnCleanScene) {
    BlobDetector det;
    cv::Mat img = make_scene({200, 200}, {0, 0, 1, 1}, 0); // 无缺陷
    std::vector<Blob> blobs;
    det.detect(img, 20.f, 5000.f, blobs);
    EXPECT_LE(blobs.size(), 1u) << "干净背景误检过多";
}
