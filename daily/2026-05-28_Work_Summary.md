# ADAS Log Dump / FFmpeg / PowerShell 调试总结

## 日期

2026-05-28

---

# 1. FFmpeg VideoWriter 输出文件格式

## 问题

使用：

```cpp
avformat_alloc_output_context2(
    &fmt_ctx_,
    nullptr,
    nullptr,
    output_path.c_str());
```

时，不确定最终生成的视频格式是什么。

---

## 结论

FFmpeg 会根据：

```cpp
output_path
```

的文件后缀自动推断封装格式。

例如：

| 文件后缀   | 封装格式     |
| ------ | -------- |
| `.mp4` | MP4      |
| `.mkv` | Matroska |
| `.avi` | AVI      |
| `.ts`  | MPEG-TS  |

---

## 示例

```cpp
Open("test.mp4");
```

最终：

```text
H264 + MP4 container
```

---

# 2. FFmpeg 获取 FPS 与播放器不一致

## 问题

代码：

```cpp
av_q2d(stream->avg_frame_rate)
```

返回：

```text
25 FPS
```

但：

```text
PotPlayer 显示 30 FPS
```

---

## 原因

H264 裸流：

```text
.h264
```

通常：

* 没有标准 FPS 信息
* 或 FPS 信息不可信

播放器会：

* 猜测 FPS
* 根据时间戳推断
* 根据容器默认值推断

---

## FFmpeg 中几个 FPS 字段

| 字段                | 含义    |
| ----------------- | ----- |
| `avg_frame_rate`  | 平均帧率  |
| `r_frame_rate`    | 原始帧率  |
| `time_base`       | 时间基   |
| `ticks_per_frame` | 帧tick |

---

## 经验

对于裸流：

```text
不要完全信任 FPS
```

最好：

* 外部指定 FPS
* 或从业务逻辑中获取 FPS

---

# 3. std::regex 提取时间戳

## 使用

```cpp
const std::regex pattern("\\d{14}");
```

---

## 含义

匹配：

```text
14位连续数字
```

例如：

```text
20250526153045
```

---

# 4. std::string 替换

## 使用

```cpp
str.replace(pos, len, "");
```

---

## 示例

```cpp
folder_name.replace(
    pos,
    std::string("-weeklycalib").size(),
    "");
```

---

# 5. Calibration 路径匹配逻辑

## 目录格式

```text
20250526-weeklycalib
```

---

## 视频格式

```text
front_20250526153045.h264
```

---

## 核心问题

需要：

```text
YYYYMMDD
```

转换为：

```text
YYYYMMDD000000
```

才能和视频时间：

```text
YYYYMMDDHHMMSS
```

进行比较。

---

## 最终方案

使用：

```cpp
std::map<long long, std::filesystem::path>
```

保存：

```text
timestamp -> calib_path
```

然后：

```cpp
lower_bound()
```

寻找：

```text
最近且不大于视频时间
```

的 calibration。

---

# 6. nlohmann::json 读取 JSON 文件

## 使用方式

```cpp
std::ifstream ifs("config.json");

json j;

ifs >> j;
```

---

## 推荐

使用：

```cpp
j.value("key", default_value)
```

避免：

```cpp
j["key"]
```

自动插入默认值。

---

# 7. spdlog Debug 日志不显示

## 表现

只能看到：

```text
info / error
```

看不到：

```text
trace / debug
```

---

## 原因

### 编译期开关

```cpp
SPDLOG_ACTIVE_LEVEL
```

未设置。

---

## 正确方案

### CMake

```cmake
target_compile_definitions(
    your_target
    PUBLIC
    SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
)
```

---

## 运行时等级

```cpp
spdlog::set_level(spdlog::level::trace);
```

---

## 注意

必须：

```text
compile-time + runtime
```

同时开启。

---

# 8. PowerShell 5 与 spdlog 问题

## 表现

### 使用：

```powershell
2>&1 | Tee-Object
```

时：

* debug 丢失
* warn 丢失
* ANSI color 异常
* 第三方日志丢失

---

## 根因

Windows PowerShell 5：

* ANSI 支持差
* native stdout/stderr 管道兼容差
* Tee-Object 会破坏 stderr 流

---

# 9. PowerShell 7 解决问题

## 安装

安装：

```text
PowerShell 7.4.16
```

---

## 使用

启动：

```powershell
pwsh
```

---

## 验证

```powershell
$PSVersionTable
```

---

## 结果

PowerShell 7：

* ANSI 正常
* spdlog color 正常
* Tee-Object 正常
* UTF8 正常
* native stderr 支持更好

---

# 10. FFmpeg stderr 日志仍然缺失

## 表现

看不到：

```text
[h264 @ ...]
Failed setup for format dxva2_vld
```

---

## 原因

这些日志：

不是 spdlog。

而是：

```text
FFmpeg av_log -> stderr
```

---

## 解决方案

不要：

```powershell
PowerShell pipeline
```

而是：

```powershell
cmd /c
```

进行真正的：

```text
stdout + stderr
```

底层 fd 合并。

---

# 11. 最终稳定日志脚本

## 推荐版本

```powershell
# Create log directory
New-Item -ItemType Directory -Force "log" | Out-Null

# Timestamp
$ts = Get-Date -Format "yyyyMMdd_HHmmss"

# Log file
$logFile = "log/AdasLogDump_EL_$ts.log"

# Command
$command = '.\bin\AdasLogDump_EastLake.exe F:\Southlake\AdasLogDumps_EL\input\EastLake_Real\CLIP-20260118095904 F:\Southlake\AdasLogDumps_EL\output -b 2>&1'

# Execute
cmd /c $command | Tee-Object -FilePath $logFile

# Exit code
$exitCode = $LASTEXITCODE

Write-Host ""
Write-Host "Process Exit Code: $exitCode"
Write-Host ""

exit $exitCode
```

---

# 12. 工业界推荐日志方案

## 推荐

不要依赖：

```text
PowerShell Tee
```

而是：

```text
logger 自己写文件
```

---

## 推荐结构

### Console Sink

```cpp
stdout_color_sink_mt
```

---

### File Sink

```cpp
basic_file_sink_mt
```

---

## Multi-sink Logger

```cpp
console + file
```

同时输出。

---

# 13. 今天的核心经验总结

---

## FFmpeg

* 裸流 FPS 不可靠
* stderr 与 spdlog 是两套系统
* av_log 默认走 stderr

---

## spdlog

* runtime level 不等于 compile-time level
* `SPDLOG_ACTIVE_LEVEL` 非常关键
* PowerShell 5 对 ANSI 支持很差

---

## Windows

* PowerShell 5 非现代终端
* PowerShell 7 改善巨大
* cmd.exe 的 stderr 合并比 PowerShell 更底层、更稳定

---

## 工业最佳实践

* logger 自己写文件
* 不依赖 Tee-Object
* console + file 双 sink
* 使用 PowerShell 7
* 使用 UTF8
* 使用 stdout_color_sink_mt

---
