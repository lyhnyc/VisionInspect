# 跨语言互操作规范（interop.md）

> 本文档是 C++ visioncore 与 C# visionapp 的**契约**，修改需双方评审（即：改这里必须同步改两侧代码并跑 CI）。

## 1. 总原则

| 规则 | 说明 |
|---|---|
| 纯 C ABI | 只导出 `extern "C"` 函数；禁止 STL/类/异常跨边界 |
| 句柄所有权 | `VC_Create` 返回句柄，C# 持有；`VC_Destroy` 释放（RAII 封装 `VisionEngine : IDisposable`） |
| 缓冲区归属 | **调用方分配**（C# new 数组传入 + max_count），库只写不 malloc 外借——杜绝跨堆释放 |
| 字符串 | UTF-8，只读返回（版本号等常量），不返回需释放的指针 |
| 错误处理 | 返回错误码 int，0=OK；库内 catch-all，禁止异常逃逸 |
| 并发 | 同一句柄内部加锁串行；需要并行时 C# 侧开多个句柄 |
| 对齐 | POD 结构体 `#pragma pack(1)` ↔ C# `[StructLayout(Pack=1)]`，字段顺序一一对应 |

## 2. 为什么选 C ABI 而不是 C++/CLI（技术决策记录）

| 方案 | 优点 | 缺点 | 结论 |
|---|---|---|---|
| **纯 C ABI（采纳）** | ABI 稳定；C# 版本无关；linux 可移植（未来相机工控机） | 手写绑定层 | ✅ 工业软件跨版本部署多，稳定优先 |
| C++/CLI | 自动封装 | 仅 Windows；.NET Core 支持折腾；编译器版本耦合 | ✗ |
| 进程隔离（gRPC） | 崩溃隔离 | 序列化延迟与节拍冲突（≤120ms 预算） | ✗ 作为备选在设计中说明 |

## 3. 句柄宽度教训（已修正）

初版 `VC_Create` 返回 `int` 句柄（指针截断），64 位下有碰撞风险。
**正式版句柄一律 `long long`**，C# 侧 `long handle`。此坑记录于开发日志 D1。

## 4. 数据流（一次检测）

```
C# Frame(byte[]) ──VC_LoadImage──▶ Mat(灰度clone)
                  ──VC_Locate───▶ VcLocateResult (值拷贝返回)
                  ──VC_DetectDefects──▶ VcBlobResult[] (调用方缓冲)
                  ──VC_MeasureCaliper──▶ VcMeasureResult
```

## 5. 增补接口的流程

新算法（如 OCR）→ 先在此文档加条目 → interop.h 加声明 → interop.cpp 实现 → C# NativeInterop.cs 同步 → gtest 冒烟 → CI 绿 → 合入。
