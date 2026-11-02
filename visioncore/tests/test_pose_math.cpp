// test_pose_math.cpp —— 位姿/几何工具（Eigen）验证
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include "common/pose_math.h"

using namespace vc;

TEST(PoseMath, RigidTransformRoundtrip) {
    // 刚体变换构造→应用→逆变换，应回到原点（旋转+平移一致性）
    Pose2D pose{3.5, -2.25, 30.0};  // dx, dy, theta_deg
    auto T = pose.toMatrix();
    cv::Point2f p(10, 5);
    auto q = pose.apply(p);
    auto p2 = pose.inverse().apply(q);
    EXPECT_NEAR(p2.x, p.x, 1e-4);
    EXPECT_NEAR(p2.y, p.y, 1e-4);
}

TEST(PoseMath, AngleWrapping) {
    Pose2D a{0, 0, 359.0}, b{0, 0, 1.0};
    EXPECT_NEAR(angleDiff(a.theta, b.theta), 2.0, 1e-6);
}
