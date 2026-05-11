# RxCpp 是什么？🚀

RxCpp 是：

```txt id="rx0"
Reactive Programming（响应式编程）
```

在 C++ 里的实现。

它来自：

ReactiveX

同家族还有：

* RxJS（前端特别火）
* RxJava
* RxSwift
* Rx.NET

---

# 它解决什么问题？

核心思想：

```txt id="rx1"
“数据流” + “事件流”
```

不是：

```cpp id="rx2"
函数调用函数
```

而是：

```txt id="rx3"
“订阅某个流”
```

然后：

* 有新数据
* 自动触发
* 自动传播

---

# 你可以把它理解成：

## “超级版 signal-slot” 🔥

比 Qt signal-slot 强很多。

因为它支持：

✅ stream
✅ filter
✅ map
✅ debounce
✅ merge
✅ async
✅ scheduler
✅ 多线程
✅ backpressure（部分）
✅ chain pipeline

---

# 一个最简单例子 ✨

```cpp id="rx4"
#include <rxcpp/rx.hpp>
#include <iostream>

int main() {
    auto values =
        rxcpp::observable<>::just(1, 2, 3);

    values.subscribe(
        [](int v) {
            std::cout << v << std::endl;
        }
    );
}
```

输出：

```txt id="rx5"
1
2
3
```

---

# 这和普通 callback 有什么区别？

普通 callback：

```txt id="rx6"
事件来了 -> 调函数
```

Rx：

```txt id="rx7"
数据在“流”里流动
```

非常像：

```txt id="rx8"
Linux pipe
```

或者：

```txt id="rx9"
ROS topic chain
```

---

# 核心概念 🌊

---

# 1. Observable（数据流）

```cpp id="rx10"
observable
```

表示：

```txt id="rx11"
未来会不断产生数据
```

例如：

* 鼠标事件
* ROS消息
* websocket
* 摄像头帧
* timer
* UI事件

---

# 2. Observer（订阅者）

```cpp id="rx12"
subscribe(...)
```

谁来消费数据。

---

# 3. Operator（操作符）🔥

这是 Rx 最强的地方。

例如：

```cpp id="rx13"
map
filter
merge
debounce
zip
buffer
```

---

# 示例：filter + map

```cpp id="rx14"
#include <rxcpp/rx.hpp>
#include <iostream>

int main() {
    auto values =
        rxcpp::observable<>::range(1, 10)

        .filter([](int v) {
            return v % 2 == 0;
        })

        .map([](int v) {
            return v * 10;
        });

    values.subscribe(
        [](int v) {
            std::cout << v << std::endl;
        }
    );
}
```

输出：

```txt id="rx15"
20
40
60
80
100
```

---

# 这个像什么？✨

像：

```txt id="rx16"
Unix Pipe
```

```bash id="rx17"
cat xxx | grep xxx | awk xxx
```

或者：

```txt id="rx18"
Python generator pipeline
```

---

# UI 场景（特别强）🔥

---

# 按钮点击流

```txt id="rx19"
click stream
```

---

# 防抖 debounce

例如搜索框：

```txt id="rx20"
输入停止 300ms 才搜索
```

Rx 非常擅长。

---

## debounce 示例

```cpp id="rx21"
stream
    .debounce(std::chrono::milliseconds(300))
    .subscribe(...);
```

这在普通 callback 里很麻烦。

---

# ROS 场景 🚗

你应该会非常容易理解。

---

# 普通 ROS

```cpp id="rx22"
camera -> callback
lidar -> callback
imu -> callback
```

然后：

```txt id="rx23"
各种 mutex
各种 queue
各种状态同步
```

很乱。💥

---

# Rx 风格

```txt id="rx24"
camera_stream
    .zip(imu_stream)
    .map(fusion)
    .filter(valid)
    .subscribe(output)
```

非常清晰。

---

# 多线程 🔥

RxCpp 有 scheduler。

---

# 切换线程

```cpp id="rx25"
.observe_on(rxcpp::observe_on_new_thread())
```

---

# 后台线程

```cpp id="rx26"
.subscribe_on(...)
```

---

# 定时器 ⏱️

```cpp id="rx27"
auto ticks =
    rxcpp::observable<>
        ::interval(std::chrono::seconds(1));

ticks.subscribe([](long v){
    std::cout << v << std::endl;
});
```

---

# merge（合并流）

```cpp id="rx28"
camera_stream.merge(lidar_stream)
```

---

# combine_latest

```cpp id="rx29"
combine_latest(
    camera,
    imu,
    gps
)
```

非常适合：

* 机器人
* 自动驾驶
* 实时系统

---

# 热流 vs 冷流 ⚠️

Rx 最难理解的地方之一。

---

# 冷流（Cold Observable）

每个订阅者：

```txt id="rx30"
自己重新执行
```

类似：

```txt id="rx31"
函数调用
```

---

# 热流（Hot Observable）

所有人共享：

```txt id="rx32"
同一个数据源
```

类似：

```txt id="rx33"
ROS topic
```

---

# Subject（像 Qt signal）🔥

```cpp id="rx34"
rxcpp::subjects::subject<int> subject;
```

---

## 发消息

```cpp id="rx35"
subject.get_subscriber().on_next(123);
```

---

## 订阅

```cpp id="rx36"
subject
    .get_observable()
    .subscribe(...);
```

---

# 这就非常像：

```txt id="rx37"
signal-slot
pub/sub
ROS topic
```

了。

---

# 为什么 Rx 很强？

因为：

```txt id="rx38"
“异步”被数据流抽象掉了
```

你不用：

❌ mutex
❌ callback hell
❌ 状态机乱飞
❌ if嵌套

---

# 为什么很多人又讨厌 Rx 😅

因为：

```txt id="rx39"
太抽象
```

会导致：

```txt id="rx40"
代码像魔法
```

特别是：

* 新人
* 调试
* 生命周期

会很痛苦。

---

# RxCpp 的问题 ⚠️

老实说：

## RxCpp 已经不算活跃了

而且：

```txt id="rx41"
模板地狱非常严重
```

编译错误会：

```txt id="rx42"
几百行
```

---

# 现代替代方案

很多现代 C++ 开始用：

* coroutines
* async generators
* cppcoro
* folly
* asio

替代 Rx。

---

# 但 Rx 思想依然非常重要 🌟

因为：

* RxJS 超流行
* 前端响应式核心
* Reactive Programming 很经典

---

# 适合什么项目？

---

# 非常适合 ✅

* UI
* 实时流
* ROS
* 自动驾驶
* 传感器融合
* websocket
* async pipeline

---

# 不适合 ❌

* 简单 CRUD
* 普通业务逻辑
* 小项目

---

# 你可以把它理解成一句话 ✨

```txt id="rx43"
Qt signal-slot = 单次事件
Rx = 可变换的数据流系统
```
