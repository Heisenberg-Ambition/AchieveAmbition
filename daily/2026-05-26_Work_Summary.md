````md
# 工作记录（2026-06-01）

## 一、spdlog 集成与使用

### 问题 1：spdlog 是否是头文件库

最初希望在项目中引入 spdlog，并通过 CMake 集成。

#### 结论

spdlog 有两种使用方式：

##### Header Only

```cmake
target_link_libraries(MyApp PRIVATE spdlog::spdlog_header_only)
````

无需编译动态库或静态库。

##### Compiled Mode

```cmake
target_link_libraries(MyApp PRIVATE spdlog::spdlog)
```

需要编译 spdlog。

对于当前项目，推荐 Header Only 模式。

---

### 问题 2：控制台日志格式配置

需求：

* 输出文件名
* 输出行号
* 输出模块信息
* 不输出线程 ID

#### 配置方案

```cpp
spdlog::set_pattern(
    "[%Y-%m-%d %H:%M:%S.%e] "
    "[%l] "
    "[%s:%#] "
    "%v");
```

常用占位符：

| 占位符 | 含义    |
| --- | ----- |
| %l  | 日志等级  |
| %s  | 文件名   |
| %#  | 行号    |
| %!  | 函数名   |
| %t  | 线程 ID |
| %v  | 日志内容  |

---

### 问题 3：printf 风格日志输出异常

代码：

```cpp
LOG_INFO("H264 path: %s", h264_file_path.c_str());
```

输出：

```text
H264 path: %s
```

#### 原因

spdlog 基于 fmt 库，不支持 printf 风格格式化。

#### 正确写法

```cpp
LOG_INFO("H264 path: {}", h264_file_path);
```

或者：

```cpp
LOG_INFO("H264 path: {}", h264_file_path.string());
```

---

## 二、PowerShell 日志重定向问题

### 问题

脚本：

```powershell
.\bin\AdasLogDump_EastLake.exe 2>&1 |
Tee-Object "log/AdasLogDump_EL_$ts.log"
```

出现：

```text
NativeCommandError
```

日志内容被 PowerShell 当成 ErrorRecord。

---

### 原因

第三方库将日志输出到了：

```cpp
stderr
```

PowerShell 5.1 对 stderr 的处理存在历史问题。

---

### 当前环境

```powershell
$PSVersionTable
```

结果：

```text
PSVersion 5.1.26100.8115
```

属于 Windows PowerShell。

---

### 解决方案

推荐直接使用 cmd：

```powershell
cmd /c ".\bin\AdasLogDump_EastLake.exe > log.txt 2>&1"
```

或者升级到 PowerShell 7。

---

## 三、nlohmann::json 序列化问题

### 场景

需要将第三方结构体数据导出到 JSON。

使用：

```cpp
nlohmann::json
```

进行序列化。

---

### 问题 1：JSON 内容变成字符串

输出结果：

```json
{
    "0": [
        "{\n \"accel_pedal_position\": 60.0\n}"
    ]
}
```

---

### 原因

代码：

```cpp
auto temp = canSignal_json.dump(4);

canSignal_json_vec.push_back(temp);
```

此时：

```text
json
↓
dump()
↓
string
↓
push_back
```

最终 JSON 中存储的是字符串。

---

### 正确写法

```cpp
canSignal_json_vec.push_back(canSignal_json);
```

不要在中间调用：

```cpp
dump()
```

---

### 原则

整个构建阶段：

```text
json
↓
json
↓
json
```

最后统一：

```cpp
root.dump()
```

---

### 问题 2：如何输出单行 JSON

原来：

```cpp
root.dump(4)
```

输出：

```json
{
    ...
}
```

格式化缩进。

---

### 解决方案

改为：

```cpp
root.dump()
```

输出：

```json
{"0":[{"speed":10}]}
```

适合作为日志流。

---

## 四、JSONL（NDJSON）日志格式

### 当前场景

CAN 信号按 Frame 持续输出。

---

### 原来的方式

每帧输出：

```json
{
    "0": [...]
}
{
    "1": [...]
}
```

并不是合法 JSON。

---

### 推荐方案

JSON Lines：

```text
{"0":[...]}
{"1":[...]}
{"2":[...]}
```

特点：

* 流式写入
* 文件可持续增长
* 内存占用低
* Python 易处理

---

### Python 读取

```python
import json

with open("signal.jsonl") as f:
    for line in f:
        obj = json.loads(line)
```

---

## 五、filesystem 路径问题

### 问题

```cpp
std::filesystem::path
```

是否支持：

```cpp
"calib\\20251210185800\\CALIB-CAM.json"
```

---

### 结论

支持。

但推荐：

```cpp
"calib/20251210185800/CALIB-CAM.json"
```

或者：

```cpp
path("calib")
    / "20251210185800"
    / "CALIB-CAM.json";
```

---

### 相对路径基准

相对于：

```cpp
std::filesystem::current_path()
```

即当前工作目录。

---

### 风险

VS 调试、PowerShell、双击 exe 时：

```cpp
current_path()
```

可能不同。

---

### 推荐方案

使用 exe 所在目录。

```cpp
fs::path exe_dir =
    getExeDir();
```

获取：

```text
project/bin
```

然后：

```cpp
fs::path calib_path =
    exe_dir.parent_path()
    / "calib"
    / "20251210185800"
    / "CALIB-CAM.json";
```

最终：

```text
project/calib/20251210185800/CALIB-CAM.json
```

稳定可靠。

---

## 六、ImageView 数据结构分析

结构：

```cpp
struct ImageView
{
    PixelFormat format;

    int width;
    int height;

    uint8_t* planes[4];
    int steps[4];

    std::vector<uint8_t> data;
};
```

---

### planes 含义

表示图像各 Plane 的起始地址。

例如：

#### NV12

```text
planes[0] -> Y
planes[1] -> UV
```

#### YUV420P

```text
planes[0] -> Y
planes[1] -> U
planes[2] -> V
```

---

### steps 含义

对应：

```text
stride
pitch
```

即每行字节数。

---

### 与 FFmpeg 对应关系

| ImageView | AVFrame       |
| --------- | ------------- |
| planes[]  | data[]        |
| steps[]   | linesize[]    |
| width     | width         |
| height    | height        |
| format    | AVPixelFormat |

本质上是 AVFrame 的轻量封装。

---

## 七、FFmpeg 视频编码流程梳理

目标：

```text
ImageView
↓
MP4
```

---

### 编码流程

```text
ImageView
↓
AVFrame
↓
H264 Encoder
↓
AVPacket
↓
MP4 Muxer
↓
output.mp4
```

---

### FFmpeg 主要模块

| 模块       | 作用    |
| -------- | ----- |
| avutil   | 基础设施  |
| avcodec  | 编码器   |
| avformat | MP4封装 |
| swscale  | 格式转换  |

---

### 关键问题

必须确认：

```cpp
PixelFormat
```

具体格式：

* NV12
* YUV420P
* RGB24
* BGR24

---

### 是否需要 swscale

| 格式      | 是否需要转换 |
| ------- | ------ |
| NV12    | 否      |
| YUV420P | 否      |
| RGB24   | 是      |
| BGR24   | 是      |

---

### 推荐封装

```cpp
class VideoEncoder
{
public:
    bool Open(...);

    bool Encode(
        const ImageView& image);

    void Close();
};
```

避免业务代码直接操作：

```cpp
AVFrame
AVPacket
AVCodecContext
```

提高可维护性。

---

# 今日收获

1. 完成 spdlog 集成与日志格式配置。
2. 理清 PowerShell 5.1 对 stderr 的特殊处理。
3. 修复 nlohmann::json 被误序列化为字符串的问题。
4. 理解 JSONL 日志格式及流式输出方案。
5. 明确 filesystem 相对路径与 exe 路径的区别。
6. 理解 ImageView 中 planes/steps 的真实含义。
7. 梳理 FFmpeg 从原始图像到 MP4 的完整编码流程。
8. 为后续封装 VideoEncoder 奠定基础。

```
```
