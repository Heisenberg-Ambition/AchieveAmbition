`ROS1 Bag` 和 `ROS2 Bag` 都是 **ROS（Robot Operating System）** 用于记录和回放数据的日志文件，但它们的设计差异非常大。

一句话概括：

* **ROS1 Bag (`.bag`)**：为 ROS1 定制的**单文件二进制格式**。
* **ROS2 Bag (`rosbag2`)**：是一个**框架**，默认使用 SQLite3（早期）或 MCAP（现在越来越推荐）作为底层存储。

---

# 整体对比

| 特性    | ROS1 Bag   | ROS2 Bag         |
| ----- | ---------- | ---------------- |
| ROS版本 | ROS1       | ROS2             |
| 文件格式  | `.bag`     | 一个目录（默认）         |
| 默认存储  | 自定义 Bag 格式 | SQLite3 (`.db3`) |
| 推荐存储  | Bag        | MCAP             |
| 可插拔存储 | ❌          | ✅ Storage Plugin |
| 多文件   | ❌          | ✅                |
| QoS   | ❌          | ✅                |
| DDS支持 | ❌          | ✅                |

---

# ROS1 Bag

## 一个文件

例如：

```text
test.bag
```

里面包含：

```text
Header

↓

Connection

↓

Chunk

↓

ChunkInfo

↓

Index
```

基本结构：

```text
+----------------+
| Header         |
+----------------+
| Connection     |
+----------------+
| Chunk          |
+----------------+
| Chunk          |
+----------------+
| ChunkInfo      |
+----------------+
| Index          |
+----------------+
```

---

## Connection

Connection 保存：

```text
Topic

MessageType

MD5

Caller
```

例如：

```text
/topic/image

sensor_msgs/Image
```

---

## Chunk

真正的数据：

```text
Message1

Message2

Message3

...
```

为了提高压缩效率：

```text
Chunk

↓

LZ4

或者

BZ2
```

---

# ROS2 Bag

ROS2 最大变化：

> **Bag 不再是一种固定格式，而是一套接口。**

即：

```text
Recorder

↓

Storage Plugin

↓

SQLite

或者

MCAP

或者

其他
```

官方架构：

```text
Recorder

↓

Storage API

↓

SQLite Plugin

MCAP Plugin

Custom Plugin
```

所以：

```bash
ros2 bag record
```

实际上：

调用的是：

```text
Storage Plugin
```

---

# 默认 SQLite

例如：

```bash
ros2 bag record /camera
```

生成：

```text
my_bag/

    metadata.yaml

    my_bag_0.db3
```

SQLite：

里面就是：

```sql
topics

messages

metadata
```

例如：

```sql
messages

id

topic_id

timestamp

data(BLOB)
```

---

# MCAP

近几年官方越来越推荐：

```text
MCAP
```

例如：

```bash
ros2 bag record \
--storage mcap
```

得到：

```text
mybag/

metadata.yaml

mybag_0.mcap
```

MCAP：

相比 SQLite：

优势：

* 顺序写更快
* 压缩更好
* 更容易随机访问
* 更适合大数据

---

# 为什么 ROS2 放弃 ROS1 Bag？

主要原因：

ROS1 Bag：

```text
只能ROS用
```

而：

MCAP：

```text
ROS

Foxglove

Autoware

Apollo

CyberRT

自定义日志

都能用
```

更通用。

---

# 数据组织

ROS1：

```text
Connection

↓

Chunk

↓

Message
```

ROS2 SQLite：

```text
Topic

↓

Message Table

↓

Blob
```

ROS2 MCAP：

```text
Schema

↓

Channel

↓

Chunk

↓

Message
```

---

# Schema

MCAP：

新增：

```text
Schema
```

例如：

```text
sensor_msgs/Image
```

保存：

```text
IDL

Proto

Flatbuffer

JSON
```

---

# Channel

Channel：

对应：

```text
/camera
```

记录：

```text
Topic

Encoding

SchemaID
```

---

# Message

真正的数据：

```text
timestamp

↓

channel

↓

payload
```

---

# Chunk

多个 Message：

组成：

```text
Chunk
```

压缩：

```text
LZ4

ZSTD
```

---

# 性能比较

| 指标   | ROS1 Bag | SQLite | MCAP  |
| ---- | -------- | ------ | ----- |
| 顺序写  | ⭐⭐⭐      | ⭐⭐     | ⭐⭐⭐⭐⭐ |
| 顺序读  | ⭐⭐⭐      | ⭐⭐     | ⭐⭐⭐⭐⭐ |
| 随机读  | ⭐⭐       | ⭐⭐⭐⭐   | ⭐⭐⭐⭐⭐ |
| 压缩   | ⭐⭐⭐      | ⭐⭐     | ⭐⭐⭐⭐⭐ |
| 超大文件 | 一般       | 一般     | 最好    |

---

# 为什么现在很多自动驾驶公司都用 MCAP？

因为：

自动驾驶：

```text
Camera

Radar

Lidar

CAN

Localization

Planning
```

每天：

```text
TB
```

级数据。

SQLite：

写入：

```text
INSERT
```

很多。

性能一般。

MCAP：

就是：

```text
Append

Append

Append
```

非常快。

---

# 与你现在的项目

结合我们之前的交流，你目前在做的是：

* FFmpeg 解码 H264/H265
* Protobuf
* MCAP 写入
* Foxglove 可视化

实际上，你现在使用的 **MCAP** 与 **ROS2 的 MCAP Storage Plugin** 使用的是**同一种 MCAP 文件格式**（遵循 MCAP 规范）。也就是说，你自己生成的 `.mcap` 文件，只要符合规范，就可以被 ROS2（配置 MCAP 存储插件时）、Foxglove 等工具读取。

而 **ROS1 的 `.bag`** 是一种完全不同的文件格式，与 MCAP 没有兼容关系，不能直接互相读取或写入，需要借助转换工具进行格式转换。
