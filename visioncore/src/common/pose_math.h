// pose_math.h —— 2D 位姿（Eigen 实现，接口用 cv 类型便于与图像代码互通）
#pragma once
#include <opencv2/core.hpp>
#ifdef TARGET_HAS_EIGEN
#include <Eigen/Dense>
#endif
#include <cmath>

namespace vc {

struct Pose2D {
    float dx = 0, dy = 0, theta = 0;   // 平移 + 旋转（度）

#ifdef TARGET_HAS_EIGEN
    Eigen::Affine2f toEigen() const {
        Eigen::Affine2f t = Eigen::Affine2f::Identity();
        t.rotate(Eigen::Rotation2Df(theta * float(CV_PI) / 180.f));
        t.translate(Eigen::Vector2f(dx, dy));
        return t;
    }
#endif

    // 3x3 齐次矩阵（OpenCV 路径）
    cv::Matx33f toMatrix() const {
        const float r = theta * float(CV_PI) / 180.f;
        const float c = std::cos(r), s = std::sin(r);
        return {c, -s, dx,
                s,  c, dy,
                0,  0, 1};
    }

    cv::Point2f apply(cv::Point2f p) const {
        const auto m = toMatrix();
        return cv::Point2f(m(0,0)*p.x + m(0,1)*p.y + m(0,2),
                           m(1,0)*p.x + m(1,1)*p.y + m(1,2));
    }

    Pose2D inverse() const {
        Pose2D inv;
        const float r = -theta * float(CV_PI) / 180.f;
        const float c = std::cos(r), s = std::sin(r);
        inv.theta = -theta;
        inv.dx = -(c*dx - s*dy);
        inv.dy = -(s*dx + c*dy);
        return inv;
    }
};

// 角度差归一化到 (-180, 180]
inline float angleDiff(float a, float b) {
    // 返回 从 b 转到 a 的最短有符号角（右手系，逆时针为正）
    float d = std::fmod(b - a + 540.0f, 360.0f) - 180.0f;
    return d;
}

} // namespace vc
