// ============================================================
// interop.h —— VisionCore 对外唯一接口（纯 C ABI，供 C# P/Invoke）
// 规则（写进 docs/interop.md，永不破坏）：
//   1. 只用 C 类型：指针/句柄/POD 结构体，禁止 STL/类跨边界
//   2. 所有返回 int 为错误码：0=OK，负数=错误（见 VC_ERR_*）
//   3. 内存规则：句柄由 VC_Create/VC_Destroy 管理；
//      输出缓冲区由调用方（C#）分配并传入容量，库不 malloc 后外借
//   4. 所有字符串输出为 UTF-8，由调用方释放无需（写入调用方缓冲）
// ============================================================
#pragma once
#ifdef _WIN32
  #define VC_API extern "C" __declspec(dllexport)
#else
  #define VC_API extern "C"
#endif

// ---------- 错误码 ----------
enum VC_ERR {
    VC_OK              = 0,
    VC_ERR_HANDLE      = -1,   // 句柄无效
    VC_ERR_PARAM       = -2,   // 参数为空/越界
    VC_ERR_BUF_TOO_SMALL = -3, // 输出缓冲区容量不足
    VC_ERR_IMAGE       = -4,   // 图像数据非法（空/尺寸0）
    VC_ERR_NOT_FOUND   = -5,   // 模板/目标未命中
    VC_ERR_INTERNAL    = -99,
};

// ---------- 结果 POD（与 C# struct 一一对齐，pack(1)） ----------
#pragma pack(push, 1)
struct VcPointF {
    float x;
    float y;
};

struct VcLocateResult {        // 模板定位结果
    float dx;                  // X 平移（像素）
    float dy;
    float theta_deg;           // 旋转角
    float score;               // 匹配置信度 0~1
};

struct VcBlobResult {          // BLOB 缺陷结果
    VcPointF center;           // 缺陷中心
    float    width;            // 外接矩形宽（像素）
    float    height;
    float    area;             // 面积（像素）
    int      type;             // 0=未知 1=划痕 2=缺失 3=污点
};

struct VcMeasureResult {       // 卡尺测量结果
    VcPointF p1;               // 边缘对端点（亚像素）
    VcPointF p2;
    float    distance_px;      // 距离（亚像素）
    float    edge_strength;    // 边缘强度（可作置信度）
};
#pragma pack(pop)

// ---------- 生命周期 ----------
VC_API long long VC_Create(int handle_out_dummy);          // 见 interop.cpp：返回句柄
VC_API void VC_Destroy(long long handle);

// ---------- 图像传入（调用方持有内存，库内只读使用） ----------
// data: BGR 或灰度连续内存；w/h/stride；channels=1 或 3
VC_API int  VC_LoadImage(long long handle, const unsigned char* data,
                         int w, int h, int stride, int channels);

// ---------- 算法功能 ----------
// 模板定位：template_data 为模板图（灰度），先 Set 后 Locate
VC_API int  VC_SetTemplate(long long handle, const unsigned char* tpl,
                           int w, int h);
VC_API int  VC_Locate(long long handle, VcLocateResult* out);

// 缺陷检测（BLOB）：输出最多 max_count 个，实际数量写 out_count
VC_API int  VC_DetectDefects(long long handle,
                             float min_area, float max_area,
                             VcBlobResult* out_buf, int max_count, int* out_count);

// 卡尺测量：p0,p1 为搜索线段两端（当前图坐标），width 为投影带宽
VC_API int  VC_MeasureCaliper(long long handle,
                              float p0x, float p0y, float p1x, float p1y,
                              float width, VcMeasureResult* out);

// ---------- 版本 ----------
VC_API const char* VC_Version();
