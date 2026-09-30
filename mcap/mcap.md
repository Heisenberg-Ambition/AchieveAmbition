MCAP（**MCAP: Message Capture**）是一种专门用于**机器人、自动驾驶、传感器数据记录**的高性能日志文件格式，由 Foxglove 提出，目前已经成为机器人领域比较流行的记录格式之一。它最大的特点是**支持随机访问、索引、压缩、多 Schema、多 Topic、多编码**，比 ROS1 的 bag 和 ROS2 的 db3 更通用。

结合你之前一直在写 **Foxglove + MCAP C++**（`mcap::McapWriter`、protobuf Schema、Channel、Message 等），下面我按**文件结构、Chunk、索引、读写流程、源码实现**几个层次详细介绍。

---

# 一、MCAP 整体结构

一个 MCAP 文件可以看成下面这样：

```text
┌─────────────────────────────────────────────┐
│ Header                                      │
├─────────────────────────────────────────────┤
│ Schema                                      │
│ Schema                                      │
│ Schema                                      │
├─────────────────────────────────────────────┤
│ Channel                                     │
│ Channel                                     │
│ Channel                                     │
├─────────────────────────────────────────────┤
│ Chunk                                       │
│   ├── Message                               │
│   ├── Message                               │
│   ├── Message                               │
│   └── ...                                   │
├─────────────────────────────────────────────┤
│ Chunk                                       │
│   ├── Message                               │
│   └── ...                                   │
├─────────────────────────────────────────────┤
│ Attachment (可选)                           │
├─────────────────────────────────────────────┤
│ Metadata (可选)                             │
├─────────────────────────────────────────────┤
│ Summary                                     │
│   ├── Schema Index                          │
│   ├── Channel Index                         │
│   ├── Chunk Index                           │
│   ├── Attachment Index                      │
│   ├── Metadata Index                        │
│   └── Statistics                            │
├─────────────────────────────────────────────┤
│ Footer                                      │
└─────────────────────────────────────────────┘
```

MCAP 不是简单的消息顺序写入，它更像数据库：

- 前面存数据
- 后面存索引

因此打开文件时，不需要扫描整个文件。

---

# 二、文件头（Header）

文件开始：

```
Magic
Header
```

Magic：

```text
89 4D 43 41 50 30 0D 0A
```

ASCII：

```
MCAP0
```

Header：

```cpp
struct Header
{
    string profile;
    string library;
};
```

例如：

```
profile = "ros2"
library = "mcap cpp 2.1.3"
```

Foxglove：

```
profile = ""
```

也是允许的。

---

# 三、Record（MCAP 的基本单位）

MCAP 整个文件都是由 Record 组成。

每个 Record：

```
+----------------+
| Opcode (1B)    |
+----------------+
| Length (8B)    |
+----------------+
| Payload        |
+----------------+
```

因此：

```
Header

Schema

Schema

Channel

Chunk

Footer
```

全部都是 Record。

---

常见 Opcode：

| Opcode | Record           |
| ------ | ---------------- |
| 0x01   | Header           |
| 0x02   | Footer           |
| 0x03   | Schema           |
| 0x04   | Channel          |
| 0x05   | Message          |
| 0x06   | Chunk            |
| 0x07   | Message Index    |
| 0x08   | Chunk Index      |
| 0x09   | Attachment       |
| 0x0A   | Attachment Index |
| 0x0B   | Statistics       |
| 0x0C   | Metadata         |
| 0x0D   | Metadata Index   |
| 0x0E   | Summary Offset   |
| 0x0F   | Data End         |

---

# 四、Schema

Schema 描述：

> Message 长什么样。

例如：

protobuf：

```
message Pose
{
    float x;
    float y;
}
```

Schema：

```cpp
struct Schema
{
    uint16 id;
    string name;
    string encoding;
    bytes data;
}
```

例如：

```
id = 1

name = "foxglove.Pose"

encoding = "protobuf"

data = descriptor set
```

你之前生成：

```
FileDescriptorSet
```

然后：

```
SerializeToString()
```

写入：

```
schema.data
```

就是这里。

---

# 五、Channel

Schema 描述：

```
数据格式
```

Channel 描述：

```
数据来自哪里
```

例如：

```
topic:

/camera/front

/lane

/imu

/can
```

Channel：

```cpp
struct Channel
{
    uint16 id;

    uint16 schemaId;

    string topic;

    string messageEncoding;

    map<string,string> metadata;
}
```

例如：

```
channel id = 2

schema id = 1

topic

/front/image

encoding

protobuf
```

以后：

所有 Message：

```
channelId=2
```

都知道：

应该按照：

```
Schema 1
```

解析。

---

# 六、Message

真正的数据。

```
Message
```

结构：

```cpp
struct Message
{
    uint16 channelId;

    uint32 sequence;

    uint64 logTime;

    uint64 publishTime;

    bytes data;
}
```

例如：

```
channel

/front/image

timestamp

100

protobuf bytes
```

data：

就是：

```
SerializeToArray()
```

后的二进制。

---

# 七、Chunk（最重要）

Chunk：

是 MCAP 性能的核心。

不是：

```
Message

Message

Message
```

直接写磁盘。

而是：

```
Chunk
    Message

    Message

    Message
```

例如：

```
Chunk

100MB
```

里面：

```
5000 个 Message
```

Chunk：

```cpp
struct Chunk
{
    compression;

    uncompressedSize;

    compressedSize;

    data;
}
```

支持：

```
none

lz4

zstd
```

---

为什么？

例如：

```
1000000 Message
```

如果没有 Chunk：

```
1000000 次 fwrite()
```

效率很差。

Chunk：

```
先写内存

↓

100MB

↓

压缩

↓

一次写盘
```

速度快很多。

---

# 八、Chunk Index

Chunk：

结束以后：

生成：

```
ChunkIndex
```

里面记录：

```
Chunk 在哪

Chunk 时间范围

Chunk Offset

Chunk Length
```

例如：

```
Chunk1

100~200

offset

10MB
```

以后：

如果：

```
跳到

timestamp=150
```

不用扫描。

直接：

```
ChunkIndex

↓

Chunk1
```

---

# 九、Summary

Summary：

相当于：

数据库索引。

里面包括：

```
Schema Index

Channel Index

Chunk Index

Statistics

Metadata Index
```

例如：

Statistics：

```
消息数

Topic 数

Schema 数
```

---

# 十、Footer

Footer：

告诉 Reader：

```
Summary 在哪
```

Reader：

打开文件：

```
Magic

↓

Footer

↓

Summary

↓

Chunk

↓

Message
```

几乎不用扫描整个文件。

---

# 十一、为什么写文件时必须先写 Schema？

例如：

```
Message
```

里面只有：

```
channelId=3
```

没有：

```
protobuf 类型
```

Reader：

看到：

```
channel=3
```

↓

```
Channel
```

↓

```
schemaId=1
```

↓

```
Schema
```

↓

```
protobuf descriptor
```

↓

解析 Message。

因此：

Schema 必须先注册。

这也是你之前一直调用：

```cpp
writer.addSchema()

writer.addChannel()
```

的原因。

---

# 十二、Writer 写入流程（与你之前的代码一致）

```cpp
mcap::McapWriter writer;

writer.open();

writer.addSchema();

writer.addChannel();

for(...)
{
    writer.write(message);
}

writer.close();
```

close() 时：

自动：

```
Flush Chunk

↓

Chunk Index

↓

Statistics

↓

Summary

↓

Footer
```

所以：

千万不要：

```
exit()

return
```

而不：

```
close()
```

否则：

Footer

Summary

都没有。

Foxglove：

打不开。

---

# 十三、Reader 读取流程

Reader：

```
Open

↓

Header

↓

Footer

↓

Summary

↓

Chunk Index

↓

定位 Chunk

↓

解压 Chunk

↓

读取 Message

↓

protobuf Parse
```

---

# 十四、与你目前项目的对应关系

结合你之前一直写的 **Foxglove Visualization** 项目，可以对应起来：

| 你的代码                        | MCAP Record                    |
| ------------------------------- | ------------------------------ |
| `addSchema()`                   | Schema                         |
| `addChannel()`                  | Channel                        |
| `write(message)`                | Message（写入 Chunk）          |
| `McapWriterOptions("protobuf")` | Header/Profile                 |
| `CompressedImage` Protobuf      | Message Data                   |
| `SceneUpdate` Protobuf          | Message Data                   |
| `writer.close()`                | Flush Chunk + Summary + Footer |

你的 `Common::addFileDescriptor()` 函数实际上就是把 **Protobuf DescriptorSet** 生成后填充到 **Schema.data** 中；而 `DescriptorPool::generated_pool()->FindMessageTypeByName(...)` 则是为了找到对应消息的描述信息，从而注册 Schema。

---

# 十五、MCAP 文件布局（完整示意）

```text
Magic
│
├── Header
│
├── Schema (Pose)
├── Schema (CompressedImage)
├── Schema (SceneUpdate)
│
├── Channel (/camera/front)
├── Channel (/imu)
├── Channel (/lane)
│
├── Chunk #1
│     ├── Message
│     ├── Message
│     ├── Message
│     └── ...
│
├── Chunk #2
│     ├── Message
│     └── ...
│
├── Attachment（可选）
├── Metadata（可选）
│
├── DataEnd
│
├── Summary
│     ├── SchemaIndex
│     ├── ChannelIndex
│     ├── ChunkIndex
│     ├── Statistics
│     └── MetadataIndex
│
├── Footer
│
└── Magic（结束标记）
```

---

如果你想**深入到源码层面**（结合你一直使用的 **mcap C++ 2.1.3**），还可以进一步分析：

1. **`McapWriter::write()` 内部是如何把 Message 放入 Chunk 的。**
2. **`ChunkBuilder` 如何组织内存并进行 LZ4/Zstd 压缩。**
3. **`writer.close()` 是如何一步步生成 Summary、ChunkIndex、Footer 的。**

这些内容能帮助你彻底理解 MCAP Writer 的底层实现，而不仅仅是文件格式。
