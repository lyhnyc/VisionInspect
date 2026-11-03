# 开发日志（工作日志模板 + 记录）

> 每天收工前 10 分钟写当日条目。格式固定，面面俱到不如真实具体。
> 这些记录是面试细节的直接弹药库，也是"真做过"的第一手证据。

## 模板（复制使用）

```
## D{n} {日期} {主题}

### 今天做了什么
- （产出物清单，含文件路径）

### 遇到的问题
| 问题 | 现象 | 排查过程 | 解决 |

### 数据记录
- （今天跑出的任何数字：耗时/精度/检出率/帧率）

### 明日计划
- 
```

---

## D1 2026-11-02 立项与骨架（示例条目，开工后替换为真实记录）

### 今天做了什么
- 仓库初始化：visioncore(C++/CMake) + visionapp(C#/WPF) 双工程 Hello World 通过
- C ABI 接口定稿：interop.h（14 个导出函数），docs/interop.md 契约文档
- CI 配置：GitHub Actions（C++ CMake+gtest / C# dotnet build）
- SRS 1 页版定稿：指标 N1 节拍≤120ms、N2 亚像素<0.1px、N3 检出率≥99%

### 遇到的问题
| 问题 | 现象 | 排查 | 解决 |
|---|---|---|---|
| 句柄截断风险 | VC_Create 返回 int，64位指针截断 | 评审互操作文档时发现 | 改 long long 句柄，写入 interop.md 教训节 |

### 数据记录
- 双工程空载构建时间：C++ x s / C# y s（CI 基线）

### 明日计划
- MVTec AD 下载与合成缺陷工具；BLOB v0 + 首批 gtest

## D1 实际执行记录（2026-11-02/03）

### 环境里程碑
- Git 安装配置 + GitHub 仓库首推成功（commit 92acb63，53 objects）
- 网络：github 直连被墙 → Watt Toolkit 加速解决；git 需 sslVerify=false（证书重签副作用）
- VS2022 17.8 (LTSC) 安装成功：Win10 1909 不能用最新版 Installer 4.11，改用 LTSC 17.8 通道
  - 踩坑记录：官方弹窗「不支持操作系统」实为警告可跳过（日志 PrecheckTools 确认）
  - C 盘空间从 13GB 清理到 23GB（pip/uv缓存、WPS旧备份迁移）
- vcpkg 独立安装 C:/dev/vcpkg（VS 随附版写入 Program Files 有权限问题）

### 遇到的问题
| 问题 | 现象 | 排查 | 解决 |
|---|---|---|---|
| csproj 编译失败 | MSB4025 根级数据无效 | 检查发现首行有 markdown 注释 | 删除注释行 |
| XAML 同类错误 | MC3000 | 同上 | 同上 |
| P/Invoke 句柄类型 | CS1001/CS1003 x12 | C# 用了 C 的 long long | 改为 C# long |
| 缺 WPF 入口 | CS5001 | 缺 App.xaml | 补 App.xaml + App.xaml.cs |
| BitmapSource.Create 类型 | CS1503 byte[] vs nint | WPF API 需要 IntPtr | GCHandle.Alloc pinned |
| VC_Create 32位截断 | 代码评审发现 | long long 句柄用 int 返回 | 三端统一改 long long |
| vcpkg cmake 下载 | SSL 错误 | github 被墙 | 各镜像尝试（见 D2） |
| OpenCV 下载断流 | gh-proxy 174MB 后必断 | 单连接长传输被限速 | 20MB 分段 14 并发 + 断点续传 |

### 数据记录
- C# 工程 dotnet build ✅ Release 2.97s
- 数据集: data/sim_images 200 张 chip_*.png + labels.csv（合成缺陷率 35%）

### 明日计划
- OpenCV 预编译包部署 + CMake 配置 + vc_tests 三测试跑绿


## D2 2026-11-03 C++ 工程编译打通 + 单元测试全绿

### 今天做了什么
- OpenCV 4.10.0 预编译包下载并解压到 C:\opencv（174MB 官方包，直连下载）
- vcpkg 补齐 gtest/eigen3（工具链 cmake/ninja/7zip 缓存手动预置后安装成功）
- CMake 从零配置打通：Ninja + vcvars64 + vcpkg toolchain，OpenCV 4.10.0 + gtest 全识别
- visioncore 编译通过（interop/blob_detector/caliper/pose_math/image_utils 全部参与构建）
- gtest 7 个测试全部通过（Blob 2 + Caliper 3 + PoseMath 2）
- C# visionapp 编译绿（昨日已完成），今日修 P/Invoke long 句柄类型

### 遇到的问题
| 问题 | 现象 | 排查 | 解决 |
|---|---|---|---|
| MSVC 中文注释编译错 | C2001/C2143 满屏，代码肉眼无错 | UTF-8 无 BOM 被 MSVC 按 GBK 解析，中文字节吞掉后续引号 | 全部源文件加 UTF-8 BOM |
| RotatedStrip 测量恒差 1.03px | 直带 <0.1px 但 30° 旋转带稳定偏 1px | 用 Python 逐行仿真算法：算法本身无偏（-0.05px）→ 偏差来自测试图 fillPoly 光栅化锯齿 | 测试图改用解析公式合成（1px 线性渐变边缘），误差降到 0.05px 级 |
| 测试搜索线方向错误 | RotatedStrip 直接返回 false | 搜索线沿带长轴 30°（与带平行测不到宽度），应沿法向 120° | 修正搜索线方向 |
| VS 生成器找不到实例 | cmake -G "Visual Studio 17 2022" 报 instance not known | LTSC 17.8 的实例未注册进 VS Installer 元数据 | 改用 vcvars64 + Ninja 生成器，完全绕开 |
| benchmark 头文件缺失 | C1083 benchmark/benchmark.h | vcpkg 只装了 gtest/eigen3 | vc_bench 暂缓到 D5 接入 benchmark 时启用 |
| gtest_discover_tests 构建期跑 exe | 缺 OpenCV dll 报错 | 该命令在 build 时就要执行被测程序 | 去掉 discover，直接 ctest/手动跑 |

### 数据记录
- 干净构建（配置+编译）：4 秒（Ninja -j12）
- 全部测试耗时：88ms（7 tests）
- 卡尺亚像素精度：垂直带 <0.1px、30° 旋转带 ~0.05px（解析合成图）
- Blob 检测：已知缺陷检出+正确分类，无缺陷场景零误报
- OpenCV 库路径：C:/opencv/opencv/build/x64/vc16/lib（4.10.0 预编译）

### 明日计划
- D3 数学日：matchTemplate 模板定位 + 抛物线亚像素、棋盘格标定、卡尺接入定位结果
- benchmark 三方对比表（IPP 开关）
