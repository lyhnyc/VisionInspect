// pose_math.cpp —— 2D 位姿/几何工具（Eigen 可用时走 Eigen 路径，否则 OpenCV 路径）
#include "common/pose_math.h"

namespace vc {

cv::Point2f Pose2D::apply(const cv::Point2f& p) const {
    double c = std::cos(theta_deg * CV_PI / 180.0);
    double s = std::sin(theta_deg * CV_PI / 180.0);
    return {static_cast<float>(c * p.x - s * p.y + dx),
            static_cast<float>(s * p.x + c * p.y + dy)};
}

Pose2D Pose2D::fromTo(const cv::Point2f& src, const cv::Point2f& dst, double theta_deg) {
    Pose2D pose;
    pose.theta_deg = theta_deg;
    double c = std::cos(theta_deg * CV_PI / 180.0);
    double s = std::sin(theta_deg * CV_PI / 180.0);
    // dst = R(theta)*src + t  =>  t = dst - R*src
    pose.dx = static_cast<float>(dst.x - (c * src.x - s * src.y));
    pose.dy = static_cast<float>(dst.y - (s * src.x + c * src.y));
    return pose;
}

} // namespace vc
