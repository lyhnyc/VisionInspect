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
