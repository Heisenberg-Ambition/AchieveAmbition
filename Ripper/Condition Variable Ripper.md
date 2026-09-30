# Condition Variable

### 1. 多次发送

### 2. 多个线程

### 3. 阻塞机制

### 4. 需要加锁

###

**不是"先通知，再等待条件"。而是"先检查条件，不满足才进入等待；被通知后，再重新检查条件。"**

真正的执行流程如下：

```cpp
cv.wait(lock, [] {
    return !queue.empty();
});
```

实际上等价于（伪代码）：

```cpp
while (!(!queue.empty()))  // 即 while(queue.empty())
{
    // ① 释放 mutex
    lock.unlock();

    // ② 线程进入睡眠，等待 notify
    sleep();

    // ③ 被 notify_one()/notify_all() 唤醒
    wakeup();

    // ④ 重新获得 mutex
    lock.lock();

    // ⑤ 再检查 queue 是否为空
}

// queue 不为空，继续执行
```

注意最后一步：

> **被唤醒以后，并不会直接继续执行，而是重新检查条件！**

---

## 举个完整例子

初始状态：

```
queue 为空
```

消费者：

```cpp
cv.wait(lock, [] {
    return !queue.empty();
});
```

流程：

```
queue.empty() == true

↓

条件不满足

↓

释放 mutex

↓

进入睡眠
```

此时消费者完全不占 CPU。

---

生产者：

```cpp
queue.push(10);

cv.notify_one();
```

发生：

```
queue.push(10)

↓

queue 已经有数据

↓

notify_one()

↓

消费者被唤醒
```

消费者：

```
重新获得 mutex

↓

再次检查：

!queue.empty()

↓

true

↓

wait 返回
```

然后：

```cpp
int value = queue.front();
```

开始消费。

---

# 为什么一定要「先 push，再 notify」？

正确写法：

```cpp
{
    std::lock_guard<std::mutex> lock(mutex_);

    queue.push(data);
}

cv.notify_one();
```

顺序：

```
push

↓

notify
```

这样消费者醒来时：

```
queue 已经有数据
```

---

如果反过来：

```cpp
cv.notify_one();

queue.push(data);
```

可能发生：

```
notify

↓

消费者醒来

↓

queue 还是空

↓

继续睡觉

↓

Producer push

↓

没有新的 notify
```

结果：

```
queue 有数据

Consumer 睡着了
```

程序卡住。

所以：

> **共享数据状态（例如 `queue.push()`）应该先更新，再发出通知。**

---

# 为什么还要再次检查？

因为有一种现象叫：

## 虚假唤醒（Spurious Wakeup）

操作系统可能：

```
没有 notify

↓

线程自己醒了
```

例如：

```
queue 还是空

↓

wait 返回

↓

如果不检查

↓

queue.front()

↓

崩溃
```

所以标准要求：

永远写：

```cpp
cv.wait(lock, predicate);
```

或者：

```cpp
while (queue.empty())
{
    cv.wait(lock);
}
```

**不要写：**

```cpp
if (queue.empty())
{
    cv.wait(lock);
}
```

因为：

```
queue 空

↓

wait

↓

虚假唤醒

↓

if 不会再判断

↓

front()

↓

崩溃
```

---

# 为什么要重新获得 mutex？

假设：

有两个消费者：

```
Consumer A

Consumer B
```

Producer：

```
push(1)

↓

notify_all()
```

两个线程都醒了。

但是：

```
mutex

只能一个线程拥有
```

例如：

```
A 抢到 mutex

↓

取走数据

↓

queue 空了

↓

释放 mutex
```

然后：

```
B 获得 mutex

↓

再次检查

↓

queue.empty() == true

↓

继续睡觉
```

如果没有这一步检查：

B 就会：

```
front()

↓

访问空队列
```

---

## 整个流程图

```text
Consumer

检查 queue.empty()
        │
        ├── 否（有数据）
        │      ↓
        │   直接继续执行
        │
        └── 是（没数据）
               ↓
        释放 mutex
               ↓
           进入睡眠
               ↓
       Producer push(data)
               ↓
      Producer notify_one()
               ↓
        Consumer 被唤醒
               ↓
       重新获得 mutex
               ↓
      再次检查 queue.empty()
               │
        ├── 有数据 → 返回 wait()
        │
        └── 没数据 → 继续睡眠
```

所以，你的问题：

> **是先被通知还是先等待队列就绪？**

准确地说是：

1. **先检查队列是否就绪（predicate）。**
2. **如果未就绪，则进入等待。**
3. **收到 `notify` 后被唤醒。**
4. **重新获取互斥锁。**
5. **再次检查队列是否就绪。**
6. **只有条件真正满足时，`wait()` 才会返回。**

这也是 `condition_variable` 能够正确处理**竞争条件（race condition）**和**虚假唤醒**的关键机制。

**是的。** ✅

`std::condition_variable` 最大的作用就是：

> **让消费者线程阻塞（睡眠），直到满足某个条件，而不是一直循环检查。**

这是它相比 `while + sleep` 最大的优势。

---

# 看一个完整的消费者

```cpp
std::mutex mutex_;
std::condition_variable cv;
std::queue<int> queue;

void Consumer()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv.wait(lock, [] {
            return !queue.empty();
        });

        int value = queue.front();
        queue.pop();

        lock.unlock();

        std::cout << value << std::endl;
    }
}
```

执行流程如下：

```text
开始
  │
  ▼
获得 mutex
  │
  ▼
queue 是否为空？
  │
 ┌┴───────────────┐
 │                │
否               是
 │                │
 ▼                ▼
继续执行      wait()
                 │
                 ▼
        释放 mutex
                 │
                 ▼
         线程进入阻塞（睡眠）
```

所以：

> **`wait()` 会阻塞当前线程。**

---

# 阻塞的时候 CPU 在干什么？

假设：

```cpp
cv.wait(lock, predicate);
```

如果队列为空：

消费者线程：

```text
Sleeping...
```

CPU：

```text
0%
```

线程不会一直运行：

```cpp
while (queue.empty())
{
}
```

这种才叫：

**Busy Waiting（忙等待）**

CPU：

```text
100%
```

---

# Producer 来了数据

Producer：

```cpp
{
    std::lock_guard<std::mutex> lock(mutex_);

    queue.push(10);
}

cv.notify_one();
```

发生：

```text
Producer

↓

push(10)

↓

notify_one()

↓

Consumer 被唤醒

↓

重新获得 mutex

↓

wait() 返回

↓

继续执行
```

所以：

消费者只有：

```cpp
cv.wait(...)
```

这一行是阻塞的。

后面的：

```cpp
queue.front();
queue.pop();
```

都不是。

---

# 阻塞期间 mutex 是什么状态？

很多人第一次学都会有这个疑问。

假设：

```cpp
std::unique_lock lock(mutex_);

cv.wait(lock);
```

阻塞以后：

**mutex 会自动释放！**

否则：

Producer 永远拿不到锁。

整个过程：

```text
Consumer:

获得 mutex
      │
      ▼
wait()
      │
      ▼
自动释放 mutex
      │
      ▼
线程睡眠
```

Producer：

```text
获得 mutex

↓

push

↓

notify

↓

释放 mutex
```

Consumer：

```text
醒来

↓

重新获得 mutex

↓

继续执行
```

这就是为什么：

`wait()` 必须使用：

```cpp
std::unique_lock
```

而不是：

```cpp
std::lock_guard
```

因为：

`wait()` 内部需要：

```text
unlock()

↓

sleep()

↓

lock()
```

---

# 它和 `future.get()` 很像吗？

非常像。

例如：

```cpp
std::future<int> future;

int value = future.get();
```

如果结果没准备好：

```text
阻塞
```

准备好了：

```text
返回
```

---

而：

```cpp
cv.wait(lock, predicate);
```

如果：

```text
predicate == false
```

阻塞。

如果：

```text
predicate == true
```

立即返回。

区别在于：

| `future.get()`                       | `condition_variable::wait()` |
| ------------------------------------ | ---------------------------- |
| 一次性结果                           | 可无限次等待                 |
| 自动同步                             | 需要 `mutex`                 |
| 不需要 `notify`（由 `promise` 完成） | 需要 `notify_one/all`        |
| 只能 `get()` 一次                    | 可以循环 `wait()`            |

---

# 在 ROS2 中举个例子

假设：

**感知线程**不断收到点云：

```text
PointCloud

↓

queue.push()

↓

notify_one()
```

**规划线程**：

```cpp
while (rclcpp::ok())
{
    std::unique_lock lock(mutex_);

    cv.wait(lock, [] {
        return !point_cloud_queue.empty();
    });

    auto cloud = point_cloud_queue.front();
    point_cloud_queue.pop();

    lock.unlock();

    Plan(cloud);
}
```

此时：

- 没有点云 → **规划线程阻塞，不占 CPU** 😴
- 有点云 → **立即唤醒，开始规划** 🚀

这就是生产者-消费者模型，也是 `std::condition_variable` 最经典、最推荐的使用方式。
