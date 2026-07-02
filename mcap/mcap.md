# MCAP 单例写入导致 Chunk 结构破坏问题分析与解决方案

## 1. 问题描述

在**纯单线程**环境下，使用单例模式封装 MCAP Writer 进行数据写入时，出现以下现象：
- 写入完成后，使用工具打开 `.mcap` 文件时报错，提示 Chunk 结构被破坏或文件截断。
- 尝试加锁、改为单线程均无法解决（因为本身就是单线程）。
- 尝试手动调用 `close()` 依然失败。
- 使用 ROS2 默认的 BagWriter（底层 SQLite3）在相同逻辑下可以正常写入。

**初始推断**：原因是静态变量的生命周期问题导致写入失败；BagWriter 正常说明 Bag 和 MCAP 的底层结构不一致。

---

## 2. 初始推断验证

### 2.1 关于 "Bag 和 MCAP 结构不一致"
**✅ 推断正确。**
- **ROS2 Bag (SQLite3)**：底层是 SQLite3 数据库，天然支持流式追加写入，每条消息是独立的 `INSERT` 事务。具有 WAL（Write-Ahead Logging）和 Journal 机制，崩溃容错和恢复能力极强。
- **MCAP**：是一个严格的二进制容器格式，包含 `Header → Chunks → Index → Summary → Footer` 的精密结构。它不是流式追加就能直接读取的，对文件完整性要求极高。

### 2.2 关于 "静态变量生命周期问题"
**✅ 推断正确（在纯单线程前提下）。**
在排除了多线程并发导致的状态交错后，C++ 单例模式（静态变量）的生命周期管理缺陷，正是导致 MCAP 文件结构损坏的**核心根因**。

---

## 3. 核心根因分析（纯单线程环境）

### 3.1 MCAP 文件格式的致命特性：强依赖 `close()`
MCAP 文件在写入过程中，消息数据只是被暂存在 Chunk 中。**完整的文件结构必须在调用 `close()` 时才能最终封口**：

```text
┌─────────────────────────────┐
│  Magic (8 bytes)            │
│  Header                     │
│  Chunk 1...N (消息数据)      │  ← writeMessage() 阶段写入
│  DataEnd                    │
├─────────────────────────────┤
│  Summary (Chunk索引/统计)    │  ← 只有在 close() 时才写入！
│  SummaryOffset              │
├─────────────────────────────┤
│  Footer (指向Summary的偏移)  │  ← 只有在 close() 时才写入！
│  Magic (8 bytes)            │  ← 只有在 close() 时才写入！
└─────────────────────────────┘
```
**结论**：如果 `close()` 没有执行、执行失败或执行了两次，文件就会缺少 Footer 和 Summary，或者在尾部写入垃圾数据。任何 MCAP 读取工具都会将其判定为 "Chunk 结构被破坏" 或 "文件截断"。

### 3.2 单例模式踩坑的三大场景

#### ❌ 场景 A：静态指针单例 → `close()` 根本没被调用
```cpp
class McapWriterSingleton {
    static McapWriterSingleton* instance_; // 裸指针
    ~McapWriterSingleton() { writer_.close(); } // 永远不会执行！
};
```
**机制**：`new` 出来的对象赋值给静态裸指针，C++ 标准**不会在程序退出时自动 `delete` 它**。析构函数永远不执行，`close()` 永远不调用，文件停留在残缺状态。

#### ❌ 场景 B：Meyer 单例 → 析构时依赖已死（最可能的根因）
```cpp
class McapWriterSingleton {
public:
    static McapWriterSingleton& getInstance() {
        static McapWriterSingleton instance; // 局部静态变量
        return instance;
    }
    ~McapWriterSingleton() { writer_.close(); } // 此时底层资源可能已死！
};
```
**机制**：C++ 的 **Static Destruction Order Fiasco（静态析构顺序灾难）**。局部静态变量在 `main()` 返回后，按**与构造相反的顺序**析构。
如果 `IWriteCallback` 依赖的底层文件流（如全局的 `std::ofstream`、`FILE*` 或其他静态单例）**先于** MCAP Writer 被析构，`close()` 在写入 Summary/Footer 时就会写入到已释放的句柄中，导致静默失败，文件损坏。

#### ❌ 场景 C：手动 `close()` + 析构函数 Double Close
```cpp
// 在 main() 结束前手动调用
McapWriterSingleton::getInstance().close(); 

// 程序退出时，~McapWriterSingleton() 再次调用 writer_.close();
```
**机制**：MCAP 的 `close()` 通常**不是幂等的**。如果底层文件流已经关闭，第二次 `close()` 可能会写入错误的偏移量或垃圾数据，直接破坏已经写好的 Footer 结构。

---

## 4. 为什么 BagWriter 正常工作？

| 对比维度 | MCAP Writer | ROS2 BagWriter (SQLite3) |
| :--- | :--- | :--- |
| **文件结尾要求** | 必须有 Footer + Summary 才能被解析 | SQLite3 的 WAL/日志机制，崩溃恢复能力强 |
| **`close()` 失败后果** | 文件完全不可读（缺少索引和魔数） | 下次打开时 SQLite3 自动执行 Recovery |
| **写入模型** | 一次性写入 Summary/Footer（在 close 时） | 每条消息都是独立的 `INSERT` 事务 |
| **对析构顺序敏感度**| **极高**（close 依赖底层 IO 句柄存活） | **低**（SQLite3 有独立的文件锁和 journal 机制） |

**总结**：BagWriter 能正常工作，不是因为代码逻辑没问题，而是 SQLite3 强大的容错机制在底层帮你兜底了生命周期和异常退出的问题。

---

## 5. 解决方案

### 方案一：显式生命周期管理（⭐ 强烈推荐）
**放弃静态单例，改为显式创建和销毁，确保在所有依赖资源存活时完成 `close()`。**

```cpp
// 使用 unique_ptr 持有，不使用 static
std::unique_ptr g_writer;

void init_writer() {
    g_writer = std::make_unique();
    g_writer->open("output.mcap");
}

void shutdown_writer() {
    if (g_writer) {
        g_writer->close();  // 确保在 main() 结束前、其他资源析构前 close
        g_writer.reset();   // 立即销毁
    }
}

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    init_writer();
    
    // ... 运行节点 ...
    
    shutdown_writer();      // 1. 先关 writer
    rclcpp::shutdown();     // 2. 再关 ROS2
    return 0;
}
```

### 方案二：如果必须用单例，确保 `close()` 幂等且 IO 资源作为类成员
**将底层文件流作为单例类的成员变量，确保其生命周期与 Writer 严格绑定，并增加防重入保护。**

```cpp
class McapWriterSingleton {
public:
    static McapWriterSingleton& getInstance() {
        static McapWriterSingleton instance;
        return instance;
    }

    void close() {
        std::lock_guard lock(mutex_);
        if (!closed_) {
            writer_.close();
            closed_ = true;
        }
    }

    ~McapWriterSingleton() {
        close(); // 幂等保护，防止 double close
    }

private:
    McapWriterSingleton() = default;
    std::mutex mutex_;
    mcap::McapWriter writer_;
    bool closed_ = false;

    // 关键：将 ofstream 作为类成员，确保它和 writer_ 同生共死，
    // 避免依赖外部全局/静态变量导致析构顺序错乱
    std::ofstream file_stream_; 
};
```

### 方案三：使用 `atexit()` 强制控制析构顺序
**如果受限于历史代码无法重构，可以使用 `atexit` 确保 Writer 在其他静态变量之前被清理。**

```cpp
static McapWriterSingleton* writer_ptr = nullptr;

void cleanup_writer() {
    if (writer_ptr) {
        writer_ptr->close();
        delete writer_ptr;
        writer_ptr = nullptr;
    }
}

// 在创建 writer 时注册
writer_ptr = new McapWriterSingleton();
atexit(cleanup_writer); // atexit 注册的函数在静态变量析构之前执行
```

---

## 6. 问题排查与验证清单

如果修改代码后仍有疑虑，可通过以下步骤进行物理验证：

1. **Hex 编辑器检查文件尾部**：
   - 用十六进制编辑器（如 HxD, bless）打开损坏的 `.mcap` 文件。
   - 检查文件末尾是否存在 MCAP 的尾部 Magic：`89 4D 43 41 50 30 65 6E 64 00` (`\x89MCAP0end\0`)。
   - **如果没有**：说明 `close()` 根本没执行完（命中场景 A 或 B）。
   - **如果有但文件仍报错**：说明 `close()` 执行了两次或写入了错误偏移（命中场景 C）。

2. **底层日志追踪**：
   - 在 `close()` 前后使用 `fprintf(stderr, ...)` 或 `std::cout` 打印日志（**不要使用 ROS2 的 RCLCPP_INFO**，因为 ROS2 日志系统可能已经 shutdown）。
   - 确认 `close()` 是否被调用、调用了几次、是否执行到了最后一行。

3. **检查返回值**：
   - 检查 `mcap::McapWriter::close()` 的返回状态（Status），确认是否有底层 IO 写入失败的错误码。
