# VisionInspect 开发环境一键准备（PowerShell，管理员或开发者命令行）
# 逐段执行；每步的"验证"通过再进行下一步

# 1) 基础工具
winget install Git.Git CMake.CMake -e
git --version; cmake --version

# 2) Visual Studio 2022（若未装，社区版免费）——需要两个 workload：
#    - 使用 C++ 的桌面开发
#    - .NET 桌面开发（WPF）
# 已装 VS 则用 Visual Studio Installer 勾选缺失的 workload

# 3) vcpkg（C++ 依赖管理）
git clone https://github.com/microsoft/vcpkg C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat
# 环境变量（永久）：
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\dev\vcpkg", "User")

# 4) C++ 依赖（一次性，各 5~20 分钟）
C:\dev\vcpkg\vcpkg install opencv4:x64-windows eigen3:x64-windows gtest:x64-windows benchmark:x64-windows

# 5) .NET 8 SDK
winget install Microsoft.DotNet.SDK.8 -e
dotnet --version

# 6) Intel IPP（oneAPI Base Toolkit 组件，免费）
# 官网下载 oneAPI Base Toolkit 安装器 → 仅勾选 Intel IPP
# 默认路径: C:\Program Files (x86)\Intel\oneAPI\ipp\latest

# 7) 海康 MVS SDK（D6 真机才需要，可先跳过）
# https://www.hikrobotics.com/cn/machinevision/service/download → 机器视觉工业相机客户端 MVS

# 8) 虚拟串口（D9 用）
# com0com: https://sourceforge.net/projects/com0com/  → 建对 COM10<->COM11

# 9) 数据集
# MVTec AD: https://www.mvtec.com/company/research/datasets/mvtec-ad
# 只需要 metal 类（~800MB），放 data/mvtec/metal/
# 合成图（本项目主要训练/测试源）：
# python tools/gen_defects.py --out data/sim_images -n 200

# 10) 验证 C++ 侧构建
cmake -S visioncore -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# 11) 验证 C# 侧构建
dotnet build visionapp -c Release

# 12) 初始化 git 并首推
git init
git add .
git commit -m "D1: repo skeleton, C ABI contract, CI, SRS"
git branch -M main
git remote add origin https://github.com/<你的用户名>/VisionInspect.git
git push -u origin main
