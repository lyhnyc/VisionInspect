// pose_math.cpp —— 2D 位姿工具（核心逻辑在 pose_math.h 头文件内联实现）
#include "common/pose_math.h"

namespace vc {

// 角度归一化到 [0, 360)
inline float normalizeAngle(float deg) {
    float d = std::fmod(deg, 360.0f);
    if (d < 0) d += 360.0f;
    return d;
}

} // namespace vc
