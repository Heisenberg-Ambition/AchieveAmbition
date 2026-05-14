
C++有没有类似于QT的信号槽的标准库/三方库, 通过函数来触发另一个函数, 或者像ROS的订阅函数一样触发

有，而且非常多。🚀

你说的这种：

```txt id="u2c9fx"
函数 -> 触发另一个函数
```

本质上就是：

* 观察者模式（Observer Pattern）
* Event Bus
* Publish/Subscribe
* Signal/Slot
* Callback

Qt、ROS、本质上都属于这一类。

---

# C++ 标准库里有没有？

## 有一半 😅

标准库提供了：

* `std::function`
* `std::bind`
* lambda
* `std::invoke`

但：

```txt id="xjlwm8"
没有完整 signal-slot 系统
```

你得自己拼。

---

# 最简单的“信号槽”实现 ✨

## 单回调

```cpp id="6jlwm9"
#include <functional>
#include <iostream>

class Button {
public:
    std::function<void()> onClick;

    void click() {
        if (onClick)
            onClick();
    }
};

int main() {
    Button btn;

    btn.onClick = []() {
        std::cout << "clicked\n";
    };

    btn.click();
}
```

这已经是：

```txt id="n2j2pk"
最小版 signal-slot
```

了。

---

# 多订阅（真正 pub/sub）🔥

## 一个 signal 多个 slot

```cpp id="dtxr8r"
#include <functional>
#include <iostream>
#include <vector>

class Signal {
public:
    using Slot = std::function<void(int)>;

    void connect(Slot slot) {
        slots.push_back(slot);
    }

    void emit(int value) {
        for (auto& slot : slots) {
            slot(value);
        }
    }

private:
    std::vector<Slot> slots;
};

int main() {
    Signal sig;

    sig.connect([](int x) {
        std::cout << "A: " << x << '\n';
    });

    sig.connect([](int x) {
        std::cout << "B: " << x << '\n';
    });

    sig.emit(123);
}
```

输出：

```txt id="jlwm1a"
A: 123
B: 123
```

这已经很像：

* Qt signal
* ROS topic
* EventBus

了。✨

---

# 现成第三方库（推荐）🚀

---

# 1. Boost.Signals2（最经典）

Boost.Signals2

非常接近 Qt signal-slot。

---

## 示例

```cpp id="jlwm1b"
#include <boost/signals2.hpp>
#include <iostream>

int main() {
    boost::signals2::signal<void(int)> sig;

    sig.connect([](int x) {
        std::cout << "slot1: " << x << '\n';
    });

    sig.connect([](int x) {
        std::cout << "slot2: " << x << '\n';
    });

    sig(42);
}
```

---

## 特点

✅ 线程安全
✅ 自动 disconnect
✅ 很稳定
✅ 工业级

缺点：

```txt id="jlwm1c"
Boost 比较重
```

---

# 2. nano-signal-slot（超轻量）🔥

[nano-signal-slot GitHub](https://github.com/NoAvailableAlias/nano-signal-slot?utm_source=chatgpt.com)

这是现代 C++ 很多人喜欢的。

---

## 特点

✅ 单头文件
✅ 非常轻
✅ API 像 Qt
✅ 无 Boost

---

## 示例

```cpp id="jlwm1d"
Nano::Signal<void(int)> signal;

signal.connect([](int value){
    std::cout << value << std::endl;
});

signal.fire(123);
```

---

# 3. sigslot（非常老牌）

[sigslot GitHub](https://github.com/palacaze/sigslot?utm_source=chatgpt.com)

经典中的经典。

很多游戏引擎爱用。

---

# 4. entt dispatcher（游戏开发超常见）🎮

[EnTT GitHub](https://github.com/skypjack/entt?utm_source=chatgpt.com)

不仅 ECS。

它的 event dispatcher 非常强。

---

## 示例

```cpp id="jlwm1e"
dispatcher.trigger<MyEvent>();
```

---

# 5. RxCpp（响应式）🔥

[RxCpp GitHub](https://github.com/ReactiveX/RxCpp?utm_source=chatgpt.com)

如果你喜欢：

* ROS
* async
* stream
* observable

这个会很爽。

---

# ROS 为什么像 signal-slot？

ROS topic 本质：

```txt id="jlwm1f"
Publisher -> Subscriber callback
```

例如：

```cpp id="jlwm1g"
sub = nh.subscribe(
    "/topic",
    10,
    callback
);
```

底层其实也是：

```txt id="jlwm1h"
收到消息 -> 调 callback
```

只是：

* 多线程
* 网络
* 序列化
* queue

更复杂。

---

# Qt Signal/Slot 的特殊之处 ⚠️

Qt 不只是 callback。

它还有：

✅ 跨线程
✅ queued connection
✅ 自动对象生命周期
✅ 元对象系统（MOC）
✅ 反射

所以：

```txt id="jlwm1i"
Qt signal-slot 比普通 callback 强很多
```

---

# 现代 C++ 最推荐方案 ✨

## 小项目

直接：

```cpp id="jlwm1j"
std::function + vector
```

---

## 中大型项目

推荐：

* Boost.Signals2
* nano-signal-slot

---

## 游戏/引擎

推荐：

* entt dispatcher

---

## 响应式/流处理

推荐：

* RxCpp

---

# 你现在这个需求，我其实最推荐：

## nano-signal-slot 🚀

因为：

```txt id="jlwm1k"
最像 Qt
但没有 Qt 那么重
```

而且：

```txt id="jlwm1l"
纯现代 C++
```
