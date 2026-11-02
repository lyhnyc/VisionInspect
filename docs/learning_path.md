# 学习路线：从"会C++但没用过工程工具"到"能独立做这个项目"

> 你的底子：C/C++ 会（语法没问题）✅ | Git/CMake 没用过 ⬜ | OpenCV 没接触 ⬜ | C# 待学 ⬜
> 策略：**不脱产学工具，边做项目边学，每样工具在第一次用到时花 20 分钟学最小集**。
> 下面是带练任务的学习序列，每一步都有"学什么→做什么→验证"三段。

## 阶段0：环境与Git（D1，约2小时）

### 0.1 装 Git 并跑通（30分钟）
学：Git 三个概念就够——commit(存档)/push(上传)/branch(分支，暂时不用)
```
winget install Git.Git
git config --global user.name "你的名字"
git config --global user.email "你的邮箱"
```
做：进 VisionInspect 目录
```
cd C:\Users\lenovo\Desktop\VisionInspect
git init
git add .
git commit -m "D1: skeleton"
```
验证：`git log` 能看到这条记录 ✅

### 0.2 GitHub 并 push（30分钟）
学：GitHub=网上的仓库托管，面试官看的就是它
1. 注册 github.com → 右上角 + → New repository → 名字 VisionInspect → 不要勾 README
2. 回到命令行：
```
git remote add origin https://github.com/你的用户名/VisionInspect.git
git branch -M main
git push -u origin main
```
验证：浏览器打开你的仓库能看到文件 ✅

### 0.3 vcpkg 装 OpenCV（1小时，挂后台）
学：vcpkg=C++的包管理器，帮你编译安装第三方库（类似Python的pip）
做：按 docs/setup.md 第3、4步执行。**OpenCV 编译要等很久，挂后台，期间去学0.4**。

## 阶段1：OpenCV 最小集（D1晚上~D2，约4小时）

学：OpenCV 就是图像版的数组操作库。你只需要 8 个函数就能做完大半项目：

| 函数 | 作用 | 一句话理解 |
|---|---|---|
| `imread / imwrite` | 读/存图片 | 打开/保存文件 |
| `cvtColor` | 转灰度/色彩空间 | 把彩色图变黑白 |
| `GaussianBlur` | 高斯模糊 | 把图抹匀（去噪/估背景） |
| `adaptiveThreshold` | 自适应阈值 | "比周围亮/暗多少算异常"→黑白图 |
| `morphologyEx` | 形态学开闭 | 擦掉小白点/补小洞 |
| `findContours` | 找轮廓 | 把黑白图里的白色块一个个抠出来 |
| `boundingRect / contourArea` | 外接框/面积 | 量每个块的位置大小 |
| `matchTemplate` | 模板匹配 | 在大图里找小图在哪 |

带练任务（现在就能做，不需要编译环境）：
打开 https://docs.opencv.org/4.x/d7/da8/tutorial_table_of_content_imgproc.html
只看 3 个教程：**Changing-contrast（亮度对比度）、Thresholding、Hit-or-Miss/形态学**。
每个教程的 C++ 代码你都能读懂——因为你 C++ 会，缺的只是"这些函数存在"。

验证任务：写一个 50 行的 main.cpp：读一张你生成的 chip_0000.png → 灰度 → 自适应阈值 → findContours → 在图上画框存出。跑通它，你就已经会 80% 的视觉开发了。

## 阶段2：CMake 只学 20%（D1，30分钟）

学：CMake=跨平台的"makefile生成器"。你只需要读懂我们骨架里的这几行：
```cmake
add_library(visioncore SHARED ...)   # 把这些.cpp编译成dll
target_link_libraries(visioncore PUBLIC ${OpenCV_LIBS})  # 链接OpenCV
add_executable(vc_tests ...)         # 把测试代码编译成exe
```
带练任务：在 `visioncore/CMakeLists.txt` 里给 `vc_tests` 的源文件列表加一行注释，
然后重新 `cmake --build build --config Release`，看输出变化。
验证：`ctest --test-dir build -C Release` 显示 3 个测试通过 ✅（test_caliper 是真实现，直接能过）

## 阶段3：C# 最小集（D7 前学，2小时，不急）

学：你有 C++ 底子，C# 语法半天就顺。D7 之前只学 4 件事：
1. 类和属性（`public string Name { get; set; }`）
2. 事件和委托（+= 订阅）
3. async/await（Task 异步，照葫芦画瓢）
4. NuGet（=C#版的vcpkg/pip）
材料：微软官方 C# 入门 https://learn.microsoft.com/zh-cn/dotnet/csharp/tour-of-csharp/ 只读"类型/类/委托/异步"四节。
到 D7 我会带你逐行过 NativeInterop.cs 和 ViewModel，你现在不用碰。

## 阶段4：概念扫盲（穿插在D2~D6，遇到才学）

| 概念 | 什么时候遇到 | 学多少 |
|---|---|---|
| 单元测试/gtest | D2 第一次写测试 | assert 就是断言，TEST(套件名,用例名){} 就是写一个用例 |
| benchmark | D5 | 计时循环的规范化写法，照骨架抄 |
| 线程/队列 | D6 | `std::thread` + `std::queue` + `mutex`，三个东西 |
| TCP | D8 | Socket 收发 JSON 字符串，C# TcpListener 30行 |

## 心态与节奏

- **你不缺编程能力，缺的是"工具生态 familiarity"——这类东西全是肌肉记忆，用一次就会**
- 每天结束时的验收标准只有一条：`git log` 有今天的 commit，`ctest` 全绿
- 卡住 30 分钟没有头绪就把报错原文发我，不要自己闷头耗
- **别追求先学完再做**——上面每一步都是"边做边学"，两周后回头看你会惊讶进度

## 今天（D1）的检查清单
- [ ] Git 装好，git log 有第一条 commit
- [ ] GitHub 仓库已 push
- [ ] vcpkg 的 OpenCV 在后台编译
- [ ] OpenCV 三个教程读完
- [ ] 50 行 OpenCV 练习写完（可在 vcpkg 编完后立即验证）
