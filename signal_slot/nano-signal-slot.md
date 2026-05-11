# [nano-signal-slot GitHub](https://github.com/NoAvailableAlias/nano-signal-slot?utm_source=chatgpt.com) 是什么？🚀

`nano-signal-slot` 是一个：

```txt id="nano0"
超轻量 C++ signal-slot 库
```

核心目标：

```txt id="nano1"
像 Qt signal-slot
但不要 Qt 那套巨大的 MOC 系统
```

---

# 它特别适合：

✅ 游戏引擎
✅ GUI
✅ 工具链
✅ C++ 后端事件系统
✅ ROS风格本地回调
✅ ECS/event bus

---

# 最大特点 ✨

## 1. 只有头文件

你只需要：

```cpp id="nano2"
#include <nano_signal_slot.hpp>
```

---

# 2. 不需要 moc

Qt：

```txt id="nano3"
signals:
slots:
Q_OBJECT
```

背后依赖：

```txt id="nano4"
MOC 元对象编译器
```

而 nano-signal-slot：

```txt id="nano5"
纯标准 C++
```

---

# 3. API 非常像 Qt

---

# 安装 📦

---

# 方法1：直接复制头文件（推荐）

下载：

```txt id="nano6"
nano_signal_slot.hpp
```

放进项目。

---

# 方法2：vcpkg

```bash id="nano7"
vcpkg install nano-signal-slot
```

---

# 方法3：CMake FetchContent

```cmake id="nano8"
include(FetchContent)

FetchContent_Declare(
    nano_signal_slot
    GIT_REPOSITORY https://github.com/NoAvailableAlias/nano-signal-slot.git
)

FetchContent_MakeAvailable(nano_signal_slot)
```

---

# 最简单例子 ✨

```cpp id="nano9"
#include <nano_signal_slot.hpp>
#include <iostream>

Nano::Signal<void(int)> signal;

int main() {

    signal.connect([](int value) {
        std::cout << "slot1: "
                  << value
                  << std::endl;
    });

    signal.connect([](int value) {
        std::cout << "slot2: "
                  << value
                  << std::endl;
    });

    signal.fire(123);
}
```

输出：

```txt id="nano10"
slot1: 123
slot2: 123
```

---

# 这就已经非常像：

```cpp id="nano11"
emit signal(123)
```

了。

---

# connect 本质

```cpp id="nano12"
signal.connect(...)
```

相当于：

```txt id="nano13"
注册回调函数
```

---

# fire 本质

```cpp id="nano14"
signal.fire(...)
```

相当于：

```txt id="nano15"
广播事件
```

---

# 成员函数绑定 🔥

这个非常常用。

---

## 示例

```cpp id="nano16"
#include <nano_signal_slot.hpp>
#include <iostream>

class Receiver {
public:
    void onEvent(int v) {
        std::cout << "receive: "
                  << v
                  << std::endl;
    }
};

int main() {

    Nano::Signal<void(int)> signal;

    Receiver r;

    signal.connect<Receiver,
                   &Receiver::onEvent>(&r);

    signal.fire(666);
}
```

输出：

```txt id="nano17"
receive: 666
```

---

# 这非常像 Qt

Qt：

```cpp id="nano18"
connect(sender,
        &Sender::sig,
        receiver,
        &Receiver::slot);
```

---

# 多参数

```cpp id="nano19"
Nano::Signal<void(int, float, std::string)>
```

---

# 返回值？

一般：

```txt id="nano20"
signal-slot 不建议依赖返回值
```

因为：

```txt id="nano21"
一个 signal 可能连接多个 slot
```

不知道该返回谁。

---

# disconnect 🔥

---

## disconnect 某对象

```cpp id="nano22"
signal.disconnect(&receiver);
```

---

# 生命周期问题 ⚠️

这是所有 signal-slot 的重点。

---

# Qt 为什么强？

因为 Qt：

```txt id="nano23"
QObject 析构时自动 disconnect
```

---

# nano-signal-slot 呢？

它也支持：

```txt id="nano24"
自动生命周期跟踪
```

但：

```txt id="nano25"
你需要按它的模式写
```

---

# Observer 模式

---

## 继承 Observer

```cpp id="nano26"
class Receiver : public Nano::Observer
```

---

## 示例

```cpp id="nano27"
#include <nano_signal_slot.hpp>
#include <iostream>

class Receiver
    : public Nano::Observer {

public:

    void onEvent(int v) {
        std::cout << v << std::endl;
    }
};

int main() {

    Nano::Signal<void(int)> signal;

    {
        Receiver r;

        signal.connect<
            Receiver,
            &Receiver::onEvent
        >(&r);

        signal.fire(1);
    }

    // r 已析构
    // 自动 disconnect

    signal.fire(2);
}
```

不会崩。✨

---

# 线程安全 ⚠️

默认：

```txt id="nano28"
不是完全线程安全
```

但它支持：

```cpp id="nano29"
Nano::TS_Policy_Safe
```

---

# 示例

```cpp id="nano30"
Nano::Signal<
    void(int),
    Nano::TS_Policy_Safe
> signal;
```

---

# 性能 🚀

nano-signal-slot 的目标之一：

```txt id="nano31"
比 Qt 更轻
比 Boost.Signals2 更快
```

因为：

* 无 RTTI 依赖
* 无 moc
* 模板优化强
* header-only

---

# 和 Boost.Signals2 对比

|             | nano | Boost.Signals2 |
| ----------- | ---- | -------------- |
| 重量          | 极轻   | 重              |
| header-only | ✅    | ❌              |
| 编译速度        | 快    | 慢              |
| 线程安全        | 可选   | 默认强            |
| Qt风格        | 很像   | 一般             |
| 性能          | 高    | 中              |
| 生命周期        | 支持   | 支持             |

---

# 和 RxCpp 对比 🌊

|                 | nano-signal-slot | RxCpp  |
| --------------- | ---------------- | ------ |
| 模型              | signal-slot      | stream |
| 难度              | 简单               | 高      |
| 数据流变换           | ❌                | ✅      |
| debounce/filter | ❌                | ✅      |
| 学习成本            | 低                | 很高     |
| 性能              | 极高               | 中      |
| GUI             | 很适合              | 一般     |
| 游戏引擎            | 非常适合             | 少      |

---

# 你可以这样理解 ✨

---

# nano-signal-slot

```txt id="nano32"
“本地事件广播”
```

---

# RxCpp

```txt id="nano33"
“异步数据流编程”
```

---

# Qt signal-slot

```txt id="nano34"
“带对象系统的工业级事件框架”
```

---

# ROS topic

```txt id="nano35"
“分布式网络 signal-slot”
```

---

# 我其实很推荐你现在用 nano-signal-slot 🚀

因为你最近的问题明显偏：

* 工具链
* UI
* 视频播放器
* 自动化
* 本地事件系统

这种：

```txt id="nano36"
轻量 signal-slot
```

会非常舒服。
