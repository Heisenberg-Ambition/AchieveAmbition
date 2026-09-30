# ROS Salvation 总结

来源文件：`ROS Salvation.txt`，主题为 ROS1/ROS2 的 bag 写入、`ros::init`/`NodeHandle` 依赖、spin 模型与 Rate、ROS2 命令行与 DDS 发现机制。

## 一、rosbag 写入与 bag 文件特性

- `rosbag::Bag::write()` 写入时必须提供时间戳（`ros::Time`），用于标记消息发生时刻。
- **bag 是顺序日志流，不是数据库**：没有"唯一键"概念，相同话题+相同时间戳+不同内容的消息会作为独立记录依次追加，**不会覆盖**。
- 播放时按写入顺序发布；时间戳相同 → 逻辑上"同时发生"，订阅者同一时刻收到多条。
- 下游只取最新一条导致的"覆盖感"是应用层行为，不是 bag 文件行为。

## 二、ros::init 与 ros::NodeHandle 的依赖

| 场景                          | 需要 ros::init | 需要 ros::NodeHandle   |
| ----------------------------- | -------------- | ---------------------- |
| 真实时间环境（默认）          | ✅ 必须        | ❌ 不需要              |
| 仿真时间（use_sim_time=true） | ✅ 必须        | ✅ 需要（订阅 /clock） |
| rosbag 回放（带 /clock）      | ✅ 必须        | ✅ 需要                |
| 只用 rosbag::Bag 写文件       | ✅ 必须        | ❌ 不需要              |

- `ros::init` 初始化 ROS 客户端库：节点名、**时间源**、日志、参数服务器连接等。不调用 → `ros::Time::now()` 返回 0/无效值，bag 时间戳错误，TF/rviz 等报 OLD_DATA 或时间不同步。
- `ros::NodeHandle` 用于连接 ROS Master：话题、服务、参数服务器。**不影响 ros::Time 的初始化**（"必须创建句柄否则时间没初始化"这句话**只在仿真时间场景下成立**）。
- 仿真时间下 `ros::Time::now()` 依赖 `/clock` 话题，订阅 /clock 需要 NodeHandle，否则时间不推进。

## 三、spin 与回调模型

- `ros::spin()`：阻塞，进入循环逐个处理回调队列中的消息；**单线程顺序执行，不并行**。某个回调耗时会长会阻塞后续回调。
- 并行 → `ros::AsyncSpinner spinner(4); spinner.start();`（4 线程并行处理回调）。
- `ros::spinOnce()`：只处理一次回调队列后立即返回，非阻塞，适合放在自定义循环里。
- 典型主循环：

```cpp
while (ros::ok()) {
    ros::spinOnce();      // 处理订阅回调
    doSomething();        // 自定义逻辑：计算/发布/状态机/日志
    loop_rate.sleep();    // 控制频率
}
```

- `ros::Rate loop_rate(10)`：按**ROS 时间**睡眠，保证循环接近 10Hz。
    - 循环体快 → sleep 约 0.1s；循环体慢 → sleep 更少；循环体超过周期 → 不 sleep 并打印警告。
    - **若 use_sim_time 且仿真时间停滞（如 Gazebo 暂停），Rate::sleep() 会阻塞卡死**。
- `ros::WallRate`：按**系统墙钟时间**睡眠，不受 use_sim_time 影响，仿真停滞时仍按真实时间运行。

## 四、ROS2 命令行工具

`ros2 --help` 常用子命令：action、bag、component、daemon、doctor、interface、launch、lifecycle、multicast、node、param、pkg、run、security、service、test、topic、trace（仅 Linux）、wtf（doctor 别名）。

## 五、ROS2 DDS 自动发现机制

- **默认范围：同一局域网（LAN，同一子网）**。ROS2 节点默认通过 UDP 多播在子网内发现，自动发现节点名、话题、服务、动作接口。
- **跨子网/广域网**：路由器一般不转发多播，默认无法跨子网；需要 FastDDS Discovery Server、unicast discovery、手动指定对端 IP，或 VPN/隧道把不同子网虚拟成局域网。
- ROS1 用 master 集中管理；ROS2 完全分布式，靠 DDS 自动发现。

## 补充与纠正

1. **rosbag 时间戳的单位**：`ros::Time` 为 `{sec, nsec}`，所有消息时间戳同一时刻回放时"同一时刻收到多条"是正常行为；回放可用 `--clock` 选项发布 /clock 驱动仿真时间。
2. **仿真时间下时间不推进的另一种情况**：即使有 NodeHandle，若 `/clock` 发布频率为 0 或时钟暂停（Gazebo pause），`Rate::sleep()` 同样阻塞。工程上可用 `ros::WallRate` + 超时保护。
3. **ROS2 节点自动发现细节**：FastDDS（默认 RMW）多播域可通过 `ROS_DOMAIN_ID` 隔离不同网络域；`ROS_LOCALHOST_ONLY=1` 可限制仅本机发现。注意**多网卡**机器可能因默认网卡选择导致发现失败。
4. **ROS1 已停止维护（2025-05）**：新项目应直接使用 ROS2（Jazzy/K 系列）；本文件大量 ROS1 API（ros::init、spin）在 ROS2 中对应 `rclcpp::init`、`rclcpp::spin(node)`。
5. **AsyncSpinner 的线程安全**：并行回调需注意共享状态互斥；回调里调用 `ros::ServiceClient`/`Publisher` 是线程安全的，但自建数据结构需自行加锁。
