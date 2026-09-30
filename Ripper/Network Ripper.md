# Network Ripper 总结

来源文件：`Network Ripper.txt`，主题为 C++ 网络库选型。

## 一、C++ 网络库三大派系

1. **底层抽象派**：接近 socket，但安全、可移植
2. **现代异步派**：future / coroutine / io_context
3. **高层协议派**：HTTP / RPC / WebSocket

## 二、主流库一览

| 库                  | 定位                      | 特点                                                    | 适用                                  |
| ------------------- | ------------------------- | ------------------------------------------------------- | ------------------------------------- |
| Boost.Asio / Asio   | 事实上的 C++ 网络基础设施 | 异步事件驱动，TCP/UDP/定时器/串口/SSL，C++20 协程支持好 | 服务器、中间件、ROS，重视性能与可控性 |
| cpp-httplib         | 极简主义 HTTP             | 单头文件、同步 API                                      | 工具、小服务、调试                    |
| Boost.Beast         | 严肃工程 HTTP/WS          | 构建在 Asio 之上，完整但偏重                            | 高性能 HTTP/WS，掌控连接生命周期      |
| libuv / uWebSockets | 高性能异步                | 极快，偏 C 风格（libuv）                                | 游戏服务器、高频实时通信              |
| gRPC (C++)          | RPC/分布式                | HTTP/2，强类型 IDL（proto），工业级、跨语言             | 分布式 RPC；代价是重、编译慢          |

## 三、选型建议

| 需求                    | 推荐        |
| ----------------------- | ----------- |
| 学"正宗 C++ 网络"       | Boost.Asio  |
| 快速写 HTTP 工具        | cpp-httplib |
| 高性能 HTTP/WS          | Boost.Beast |
| 协程 / async / ROS 风格 | Asio        |
| 分布式 RPC              | gRPC        |

## 四、重要认知

- Asio **不是 HTTP 库**，是"网络版的 RAII + 事件循环"，很多高层库底层就是它。
- C++ 网络库不是"帮你隐藏复杂性"，而是"帮你正确管理复杂性"：RAII、生命周期、线程模型迟早要理解。

## 补充与纠正

1. **现代选型补充**：C++23 之后可关注 `std::net`（P2300 发送/接收模型）、`libcurl`（成熟 HTTP 客户端）、`POCO`（全功能跨平台网络/服务框架）、`Drogon`（C++17 异步 Web 框架）、`Boost.JSON + Beast` 组合。
2. **Asio 版本注意**：独立 Asio（非 Boost 版）需注意头文件为 `<asio.hpp>`，API 与 boost::asio 基本一致；C++20 协程支持依赖 `co_await` 与 `awaitable`，建议用较新版本。
3. **cpp-httplib 的局限**：单线程处理 HTTP/1.1，性能与并发能力有限，不适合高并发生产服务；可启用 keep-alive 缓解。
4. **性能提示**：uWebSockets 基于 libuv + 零拷贝设计，适合高频场景；Boost.Beast 的异步模型需要开发者自己管理 buffer 生命周期，易踩坑。
5. **gRPC 注意事项**：需要 protobuf 生成代码，异步 API 有 sync/async/callback 三套；对 C++ 版本和构建系统要求较高。
