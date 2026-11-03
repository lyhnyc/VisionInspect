# VisionInspect 开工总结：D1–D2（2026-11-02 ~ 11-03）

> 三个用途：①给自己复盘 ②面试踩坑叙事的底稿 ③两周后写 test_report 的素材库
> 项目：工业视觉检测系统（求职作品）。模拟相机图 → C++ 核心（OpenCV）做缺陷检测与亚像素测量 → C# 上位机显示判级，两端用 C ABI 隔离。

## 1. 项目是什么（30 秒版）

| 项 | 内容 |
|---|---|
| 仓库 | github.com/lyhnyc/VisionInspect（public，每日 commit） |
| 结构 | visioncore（C++17 / CMake / OpenCV / gtest）+ visionapp（C# .NET 8 WPF）+ docs + CI |
| 指标 | 节拍 ≤120ms、亚像素 <0.1px、检出率 ≥99%（SRS 定稿） |
| 面试证据链 | 每日 commit / 真实数字 / 5 个踩坑叙事 / 可现场运行 / 技术决策记录 |

## 2. 两天时间线

### D1（11-02）立项与骨架
- 30 个文件骨架：CMakeLists（三 target）、interop.h（C ABI 14 个导出函数 + POD 结构）、缺陷检测/卡尺/定位/OCR 四模块、C# 上位机（MVVM + 模拟相机）、SRS/协议/开发日志/CI
- git：init → 首笔 commit `92acb63` → push 上线 GitHub
- tools/gen_defects.py 生成 200 张合成缺陷图（缺陷率 0.35）

### D2（11-03）环境攻坚 + C++ 打通
- VS2022 战役：官网 2026 不支持 Win10 1909 → 新版 Installer 4.11.24 也不支持 → LTSC 17.8 频道装成（MSVC 14.38）；"不支持操作系统"弹窗实为警告非拦截，读安装日志破案
- OpenCV 4.10.0：vcpkg 源码编译被下载墙卡死 → 改官方预编译包 → gh-proxy 分段下载致 payload 损坏（坏包装不上的真凶）→ 切直连重下 174.4MB → 解压至 C:\opencv
- vcpkg 补齐 gtest + eigen3（cmake/ninja/7zip 工具缓存手动预置，绕过下载墙）
- C++ 编译全绿：vcvars64 + Ninja + vcpkg toolchain，干净构建 4 秒
- gtest 7/7 通过：Blob 2 + Caliper 3 + PoseMath 2，共 88ms
- commits：`0678bba`（C# 四轮修复）+ `3a073b0`（D2 编译与测试），均已推送

## 3. 已验证状态

| 项 | 状态 | 证据 |
|---|---|---|
| C# visionapp | 编译绿 | dotnet build -c Release，2.97s |
| C++ visioncore | 编译绿 | Ninja 干净构建 4s |
| 单元测试 | 7/7 绿 | vc_tests.exe，88ms |
| 卡尺亚像素 | 直带 <0.1px，30° 旋转带 ~0.05px | 解析合成图验证 |
| Blob 检测 | 检出 + 分类正确 + 干净场景零误报 | gtest |
| 远端 | 3 笔 commit 在 GitHub | 92acb63 → 0678bba → 3a073b0 |

## 4. 五个坑（面试叙事底稿）

每条按 现象 → 根因 → 修法 → 一句话教训。

### 坑 1：下载的安装器"死活装不上"
- 现象：174MB 的 opencv exe，双击弹 GUI，静默参数 /S /D= 全部无效
- 排查：解析 PE 结构，发现程序头后跟 174MB 附加数据且以 7z 魔数开头 → 是 7z 自解压包；7z 列表 0 files → payload 损坏
- 根因：gh-proxy 分段并发下载断流错位，文件头完好但压缩数据损坏
- 修法：官方源直连重下，7z 直接解压
- 教训：验证文件完整性不能只看大小和"能不能运行"，要验内容物（哈希/格式解析）

### 坑 2：代码没改却编译不过
- 现象：MSVC 满屏 C2001/C2143，肉眼看代码无错
- 根因：源文件 UTF-8 无 BOM，MSVC 默认按 GBK 解析，中文注释的字节序列吞掉后面的引号
- 修法：全部源文件加 UTF-8 BOM
- 教训：Windows 工具链默认编码不是 UTF-8，跨平台源码必须显式管编码

### 坑 3：亚像素测量在旋转场景恒差 1.03px
- 现象：水平带误差 <0.1px，30° 旋转带稳定偏 +1.03px
- 排查：用 Python 把 C++ 算法逐行仿真 → 算法本身无偏（-0.05px）→ 嫌疑转向测试图 → fillPoly 光栅化锯齿是误差源
- 修法：测试图改解析公式合成（1px 线性渐变边缘），误差降到 0.05px 级
- 教训：测试红了先怀疑测试本身；用独立实现交叉验证，才能分开算法误差和测试误差

### 坑 4：CMake 找不到 VS 实例
- 现象：cmake -G "Visual Studio 17 2022" 报 instance not known to the Visual Studio Installer
- 根因：LTSC 17.8 未注册进 VS Installer 元数据，VS 生成器靠元数据找实例
- 修法：vcvars64.bat 导入编译环境 + Ninja 生成器，绕开实例查找
- 教训：VS 生成器只是查找器，真正干活的是 cl.exe，环境可以手动喂

### 坑 5：C# 报 "long long" 不认识
- 现象：P/Invoke 声明照抄 C 头文件，编译失败
- 根因：C 的 long long 不是 C# 类型；Windows 上 C 的 long 是 32 位，句柄在 C# 侧须用 long（64 位）精确对齐
- 教训：跨语言边界，类型按平台 ABI 精确匹配，不靠"长得像"

## 5. 角色分工教训（自留，不进面试）

两天产出真实，但键盘大部分时间在 AI 手上：用户只敲了 git 命令、点了一次解压、切了一次网。结论：产出不等于学习。D3 起的分工：
- 用户亲手：写码、构建、跑测试、commit、push
- AI 负责：读日志查环境的诊断、每条命令讲为什么、把踩坑转成面试叙事

## 6. 数字速查卡（面试随口报）

- 干净构建 4s（Ninja -j12）；全部测试 88ms / 7 tests
- 卡尺亚像素：垂直 <0.1px，旋转 ~0.05px
- Blob：检出+分类正确，干净场景零误报
- OpenCV 4.10.0 预编译包：182,898,530 字节（174.4MB）
- 合成缺陷图：200 张，缺陷率 0.35

## 7. 环境地图

| 什么 | 在哪 |
|---|---|
| 仓库 | C:\Users\lenovo\Desktop\VisionInspect |
| OpenCV 4.10.0 | C:\opencv\opencv\build（lib 在 x64\vc16\lib） |
| vcpkg | C:\dev\vcpkg（gtest / eigen3 已装） |
| 合成缺陷图 | 仓库内 data\sim_images |
| 测试可执行 | build\vc_tests.exe（运行前 PATH 需加 C:\opencv\opencv\build\x64\vc16\bin） |
| C++ 构建方式 | vcvars64 + Ninja（勿用 VS 生成器，见坑 4） |
