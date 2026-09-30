# Thread Ripper 总结

来源文件：`Thread Ripper.txt`，主题为 C++ 多线程三件套（thread / future / condition_variable）及 std::thread、pthread、jthread 的对比。

## 一、三件套分工

| 工具                   | 主要用途         | 特点                                                 |
| ---------------------- | ---------------- | ---------------------------------------------------- |
| `<thread>`             | 创建与管理线程   | 最底层，直接操作线程；`join()` 等待、`detach()` 分离 |
| `<future>`             | 获取异步任务结果 | 结果导向，适合一次性通信（promise / future / async） |
| `<condition_variable>` | 条件等待与唤醒   | 事件驱动，可多次通知，常与 `std::mutex` 配合         |

一句话：`<thread>` 负责"跑起来"；`<future>` 负责"拿结果"；`<condition_variable>` 负责"等条件"。

## 二、std::thread / pthread / std::jthread 对比

| 项目       | std::thread                   | pthread.h                      | std::jthread                           |
| ---------- | ----------------------------- | ------------------------------ | -------------------------------------- |
| 标准       | C++11                         | C/POSIX                        | C++20                                  |
| 可移植性   | 跨平台                        | 主要 Unix/POSIX                | 跨平台                                 |
| 资源管理   | 手动 join/detach              | 手动 join/detach               | 析构自动 join（RAII）                  |
| 停止机制   | 无内置，需原子标志/future     | pthread_cancel（异步、有风险） | stop_token / stop_source（协作式停止） |
| 返回值     | 无，配合 future/async         | 返回错误码 int                 | 同 std::thread                         |
| 细粒度控制 | 经 native_handle 转发平台 API | 原生支持属性/调度/亲和性       | 同 std::thread                         |

选择建议：

- 跨平台、简单任务 → `std::thread`
- 需要安全生命周期 + 协作式停止（长期任务、轮询循环）→ `std::jthread`
- POSIX 底层调度优化、读写锁、亲和性 → `pthread`
- 需要返回值/异常传播 → `std::async`（简单）或 `promise/future`（复杂）
- 避免 `pthread_cancel`，优先协作式停止

## 三、std::promise 的传递方式

- promise **不可拷贝，只能移动**（持有与 future 绑定的共享状态）。
- `std::thread t(worker, std::move(prom))` 配合 `void worker(std::promise<int>&& prom)`，完成所有权转移（推荐）。
- 写左值引用 `promise<int>&` 必须用 `std::ref(prom)`，语义是"多线程共享同一 promise"，注意并发安全。
- 按值传参 `promise<int> prom` 不可行（构造时需拷贝，编译报错）。

## 四、future 的 get() 与 wait()

- `get()`：阻塞等待结果 → **取走结果并清空共享状态**，只能调用一次；再次调用会抛 `std::future_error`。
- `wait()`：只阻塞等待就绪，**不取走结果**，可调用多次（已就绪则立即返回）。
- `valid()` 可检查 future 是否还有共享状态。

## 五、fire-and-forget 的工程语义

- `std::async` 返回的 future 析构时会阻塞（fire-and-wait-a-bit），不是真正的 fire-and-forget。
- `std::thread(work).detach()` 是原始实现：真不等、真不管、**真不安全**（生命周期需自己 100% 保证）。
- C++20 的 `std::jthread` 是"受控的 fire-and-forget"：析构自动 join + 自动请求停止。
- 工程级方案 = 线程池/任务队列统一管理生命周期。

## 六、工程建议

- 要并行：显式写 `std::launch::async`，不要依赖默认策略。
- 要延迟执行：用 `launch::deferred`，但别当并发用。
- 不要用 `std::async` 做 fire-and-forget。

## 补充与纠正

1. **future 再次 get() 的行为**：标准规定再次调用 `get()` 时因"共享状态已被取走"而抛出 `std::future_error`（`no_state`）。这是常见实现行为，但严格来说 C++ 标准把"无共享状态时调用"定义为抛 `future_error`，合理。
2. **`std::async` future 析构阻塞的细节**：该行为仅对 `std::launch::async` 策略返回的 future 成立（C++14 起明确规定）；若使用 `deferred` 策略，析构不阻塞。
3. **jthread 停止是协作式的**：线程函数需主动检查 `stop_token` 才能退出，无法强制中断；适合轮询/循环任务。
4. **pthread_cancel 的风险**：依赖"取消点"，可能打断到不一致状态，必须配合清理处理程序（pthread_cleanup_push/pop），强烈不建议使用。
5. **异常跨线程**：三者都不支持异常自动跨线程传播，线程函数内未捕获异常会导致 `std::terminate`。
