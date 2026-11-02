# VisionInspect —— IC 封装视觉检测系统

面向半导体封装测试场景（分选机/AOI 视觉工位）的完整视觉检测系统：

**相机采集 → 定位 → 缺陷检测 → 尺寸测量 → 标记 OCR → 判级分档 → TCP 上报**

| 指标 | 目标 |
|---|---|
| 视觉节拍 | ≤ 120 ms/颗（分选机产线语义） |
| 亚像素测量精度 | < 0.1 px（合成图真值验证） |
| 连续稳定性 | ≥ 1 h 无崩溃，异常自恢复 |

## 架构

```
┌─────────────────── visionapp (C# / WPF / .NET 8) ───────────────────┐
│  Views (MVVM)   Services: Camera | Tcp | Serial | Db | Pipeline     │
│                        ▲                                            │
│                        │ P/Invoke (纯 C ABI, docs/interop.md)       │
┌────────────────────────┴────────── visioncore (C++17/CMake) ────────┐
│  locate (模板定位) · measure (卡尺亚像素) · defect (BLOB)            │
│  ocr (标记检测) · common (Pose2D/Eigen, 图像工具)                   │
│  依赖: OpenCV · Eigen · (IPP 性能热点)                              │
└─────────────────────────────────────────────────────────────────────┘
        ▲ SQLite 检测记录        ▲ anomalib ONNX 推理（DL 复核通道）
```

## 目录

```
docs/          SRS · 设计说明 · 测试报告 · 使用手册 · 协议 · 互操作规范
visioncore/    C++ 算法库（本仓库核心资产）
visionapp/     C# WPF 上位机
tools/         合成缺陷图生成器 · 模拟PLC · 模拟MES
models/        anomalib 导出的 ONNX
interview/     讲稿与数字速查卡（私有）
```

## 构建

```bash
# C++（vcpkg 安装 opencv4 eigen3 gtest benchmark 后）
cmake -S visioncore -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
ctest --test-dir build -C Release

# C#（先把 visioncore.dll 拷入 visionapp 输出目录）
dotnet build visionapp -c Release
dotnet run --project visionapp
```

## 演示流程

1. 连接相机（模拟）→ 2. 单次触发看缺陷框 → 3. 连续运行（120ms 节拍）→
4. TCP 助手发 `{"cmd":"trigger"}` 远程触发 → 5. 统计页看良率 → 6. 导出 CSV。

## 进度（每日 commit，见 git log）

- [x] D1 骨架 + C ABI + CI
- [ ] D2 预处理 + BLOB v0
- [ ] D3 定位 + 标定 + 卡尺亚像素
- [ ] D4 OCR/标记检测
- [ ] D5 IPP benchmark
- [ ] D6 三级流水线
- [ ] D7 WPF 端到端 + SQLite
- [ ] D8 TCP（测试机/MES 联动）
- [ ] D9 串口 + anomalib 保底
- [ ] D10 DL 融合判级
- [ ] D11 稳定性 + 故障注入
- [ ] D12 文档三件套
- [ ] D13 演示视频 + v1.0
- [ ] D14 面试预演
