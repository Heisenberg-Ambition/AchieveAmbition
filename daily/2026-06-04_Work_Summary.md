# MCAP + Foxglove + CMake 工程问题总结

## 日期

2026-06-04

---

# 目标

研究使用 MCAP C++ SDK 生成 MCAP 文件，并在 Foxglove 中进行查看和验证。

---

# 遇到的问题

## 1. 程序链接阶段失败

编译过程中出现：

```text
FAILED: [code=4294967295] F:/Southlake/Mcap_Cpp/bin/TheGreatTry.exe

C:\Windows\system32\cmd.exe /C
...
cmake.exe -E vs_link_exe
...
```

错误发生在：

```text
Linking CXX executable TheGreatTry.exe
```

阶段，而不是编译阶段。

说明：

* `.cpp` 文件已经成功编译
* `.obj` 文件已经生成
* 问题出现在最终链接阶段

---

## 2. 如何定位真实错误

Ninja 只显示：

```text
FAILED: [code=4294967295]
```

并不能直接看到根因。

推荐使用：

```bash
cmake --build build --verbose
```

或者：

```bash
ninja -v
```

查看完整链接命令。

重点关注：

```text
LNK2019
LNK2001
LNK1104
LNK1120
```

等真正的 MSVC Linker 错误。

---

## 3. 常见原因分析

### DLL 对应的 LIB 未链接

例如：

```cmake
target_link_libraries(TheGreatTry
    PRIVATE
    mcap
)
```

遗漏：

```cmake
target_link_libraries(TheGreatTry
    PRIVATE
    mcap
    zstd
    lz4
)
```

会导致：

```text
unresolved external symbol
```

---

### Debug / Release 混用

例如：

```text
mcap.lib
```

来自：

```text
Release
```

而工程当前是：

```text
Debug
```

会产生运行库不一致问题。

---

### Runtime Library 不匹配

例如：

```text
/MD
```

与

```text
/MT
```

混用。

典型错误：

```text
LNK2038:
mismatch detected for RuntimeLibrary
```

需要统一：

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreadedDLL")
```

或者：

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreadedDebugDLL")
```

确保所有第三方库一致。

---

## 4. MCAP SDK 集成注意事项

需要确认：

### Header

```cpp
#include <mcap/writer.hpp>
```

能够正确找到。

---

### Library

需要链接：

```cmake
target_link_libraries(TheGreatTry
    PRIVATE
    mcap
)
```

---

### 依赖项

MCAP 可能依赖：

```text
zstd
lz4
```

如果使用压缩功能，需要同时链接。

---

# Foxglove 查看 MCAP

## 基本流程

### 创建 MCAP

```cpp
mcap::McapWriter writer;
```

写入：

```cpp
Schema
Channel
Message
```

---

### 生成文件

```text
test.mcap
```

---

### 使用 Foxglove 打开

菜单：

```text
Open file
```

选择：

```text
test.mcap
```

---

## 显示为空的可能原因

即使文件生成成功，Foxglove 中仍可能没有数据。

常见原因：

### 未注册 Schema

```cpp
writer.addSchema(...)
```

遗漏。

---

### 未注册 Channel

```cpp
writer.addChannel(...)
```

遗漏。

---

### Topic 名称错误

例如：

```text
camera/front
```

与期望不一致。

---

### Log Time 不合法

例如：

```cpp
message.logTime = 0;
```

可能导致时间轴无法显示。

推荐：

```cpp
auto now =
std::chrono::duration_cast<
    std::chrono::nanoseconds>(
        std::chrono::system_clock::now()
        .time_since_epoch())
        .count();
```

---

# 后续计划

## 第一阶段

获取完整 Link Error

执行：

```bash
cmake --build build --verbose
```

或者：

```bash
ninja -v
```

记录完整错误日志。

---

## 第二阶段

验证：

```text
mcap.lib
```

是否被正确链接。

检查：

```cmake
target_link_libraries(...)
```

配置。

---

## 第三阶段

生成最小 MCAP 示例：

```text
1 Schema
1 Channel
1 Message
```

确保：

```text
Foxglove
```

能够正确打开并显示。

---

# 经验总结

本次问题的关键点：

1. 编译成功 ≠ 链接成功。
2. Ninja 的错误码通常不能反映真实原因。
3. 必须查看完整 Linker 输出。
4. MCAP 集成时需要同时关注：

   * Header
   * Library
   * Runtime Library
   * 第三方依赖
5. Foxglove 能打开文件并不代表数据结构正确，需要检查：

   * Schema
   * Channel
   * Message
   * Timestamp

后续应首先定位真实的 Linker Error，再继续验证 MCAP 文件生成逻辑。
