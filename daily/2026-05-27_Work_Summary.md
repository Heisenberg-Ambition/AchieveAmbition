# FFmpeg NV12 视频编码开发问题总结

## 日期

2026-06-01

---

# 背景

目标是将已有的 NV12 格式 YUV 图像数据：

```cpp
uint8_t* y_data;
int32_t y_step;

uint8_t* uv_data;
int32_t uv_step;
```

编码为 H264 MP4 视频。

视频参数：

* 分辨率：1920 × 1080
* FPS：30
* 输入格式：NV12
* 编码器：H264

开发过程中完成了 FFmpeg VideoWriter 类的封装，并对内存管理、编码流程以及视频信息获取进行了梳理。

---

# 问题一：AVPacket 初始化方式已废弃

## 现象

代码中使用：

```cpp
AVPacket pkt;
av_init_packet(&pkt);
```

编译出现警告：

```text
'av_init_packet' is deprecated
```

---

## 原因

FFmpeg 新版本已经废弃：

```cpp
av_init_packet()
```

原因是 AVPacket 内部结构越来越复杂，已经不推荐手动初始化。

---

## 解决方案

采用以下方式：

### 栈对象

```cpp
AVPacket pkt = {};
```

或者：

### 堆对象

```cpp
AVPacket* pkt = av_packet_alloc();
```

推荐使用栈对象：

```cpp
AVPacket pkt = {};
```

无需额外释放对象本身，仅需：

```cpp
av_packet_unref(&pkt);
```

释放内部引用资源。

---

# 问题二：AVPacket 内存泄漏风险

## 现象

代码：

```cpp
AVPacket* pkt = av_packet_alloc();

if (ret < 0)
{
    return false;
}
```

出现提前 return。

---

## 原因

发生错误时：

```cpp
av_packet_free(&pkt);
```

没有执行。

导致：

```text
AVPacket 内存泄漏
```

---

## 解决方案

方案一：

每个 return 前执行：

```cpp
av_packet_free(&pkt);
```

方案二（推荐）：

使用栈对象：

```cpp
AVPacket pkt = {};
```

彻底避免泄漏问题。

---

# 问题三：Frame 直接引用外部 NV12 Buffer

## 现象

最初实现：

```cpp
frame_->data[0] = y_data;
frame_->data[1] = uv_data;
```

直接引用外部数据。

---

## 风险

编码器可能异步工作：

```cpp
avcodec_send_frame(...)
```

返回后：

```text
编码器未必立即完成编码
```

此时：

```text
外部 Buffer 被复用
```

可能导致：

* 花屏
* 崩溃
* 绿色画面
* 随机异常

---

## 解决方案

采用 FFmpeg 自己管理 Frame Buffer：

```cpp
frame_ = av_frame_alloc();

av_frame_get_buffer(frame_, 32);
```

写入数据时：

```cpp
av_frame_make_writable(frame_);
```

然后 memcpy 到 Frame Buffer。

---

# 问题四：NV12 拷贝方式错误风险

## 现象

容易直接进行：

```cpp
memcpy(...)
```

一次性拷贝全部数据。

---

## 原因

实际场景：

```cpp
width != stride
```

例如：

```text
width = 1920
stride = 2048
```

直接拷贝会导致：

* 越界
* 图像错位
* 花屏

---

## 正确方式

逐行拷贝：

### Y Plane

```cpp
for (int y = 0; y < height_; ++y)
{
    memcpy(
        frame_->data[0] + y * frame_->linesize[0],
        y_data + y * y_stride,
        width_);
}
```

### UV Plane

```cpp
for (int y = 0; y < height_ / 2; ++y)
{
    memcpy(
        frame_->data[1] + y * frame_->linesize[1],
        uv_data + y * uv_stride,
        width_);
}
```

---

# 问题五：H264/H265 裸流获取 FPS

## 现象

尝试从：

```text
.h264
.h265
.hevc
```

获取 FPS。

---

## 原因

裸流通常不包含 Container Metadata。

缺少：

* Duration
* Timestamp
* FPS

因此：

```cpp
stream->avg_frame_rate
```

可能返回：

```text
0/0
```

---

## 获取方式

优先：

```cpp
av_q2d(stream->avg_frame_rate)
```

其次：

```cpp
av_q2d(stream->r_frame_rate)
```

---

## 注意

如果 SPS/VUI 未写入 Timing Info：

```text
无法获得真实 FPS
```

属于正常现象。

---

## 工程建议

优先从业务侧获取：

```cpp
fps = 30
```

而不是依赖裸流推导。

---

# 问题六：H264/H265 裸流与 MP4 的区别

## MP4

包含：

* 时间戳
* FPS
* Duration
* Metadata

可以稳定获取：

```cpp
stream->avg_frame_rate
```

---

## H264/H265 裸流

仅包含：

```text
压缩视频数据
```

通常不包含：

```text
播放时间信息
```

因此：

```text
无法保证获取 FPS
```

---

# 问题七：NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE 与 WITH_DEFAULT

## NON_INTRUSIVE

缺失字段：

```cpp
j.at("field")
```

直接抛异常。

适用于：

* 网络协议
* RPC
* 数据交换格式

---

## NON_INTRUSIVE_WITH_DEFAULT

缺失字段：

```cpp
保持对象默认值
```

例如：

```cpp
struct Config
{
    int fps = 30;
};
```

JSON：

```json
{}
```

结果：

```cpp
cfg.fps == 30
```

---

## 注意

必须提供默认值：

```cpp
int fps = 30;
```

否则可能得到未初始化数据。

---

# 最终结论

本次开发完成了：

* FFmpeg VideoWriter 封装
* NV12 → H264 编码流程
* AVPacket 生命周期管理
* AVFrame Buffer 管理
* H264/H265 FPS 获取方案
* JSON 自动序列化配置方案

同时规避了：

* Deprecated API
* 内存泄漏
* Buffer 生命周期问题
* Stride 导致的图像错误
* 裸流 FPS 获取误区

后续建议继续研究：

1. H265 编码
2. NVENC/QSV 硬编码
3. SEI Metadata 注入
4. BGR → NV12 转换
5. RTSP 推流
6. 时间戳（PTS/DTS）管理
