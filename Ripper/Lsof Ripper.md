# Lsof Ripper 总结

## 一、lsof 是什么

lsof = LiSt Open Files。能查看"哪些文件被哪个进程打开了"。在 Linux 里"一切皆文件"，lsof 覆盖：常规文件、目录、网络套接字（TCP/UDP）、管道、设备、内存映射文件。

## 二、基础用法

| 命令                          | 作用                            |
| ----------------------------- | ------------------------------- |
| `lsof`                        | 列出所有打开的文件              |
| `lsof /tmp/myfile`            | 谁在用这个文件                  |
| `lsof -p 1234`                | 按 PID 过滤                     |
| `lsof -i :8080`               | 谁占用 8080 端口                |
| `lsof -i tcp` / `lsof -i udp` | 按协议过滤                      |
| `lsof -i @127.0.0.1:22`       | 特定 host:port                  |
| `lsof +D /var/log`            | 递归列出目录下被打开的文件      |
| `lsof -i -nP`                 | 不解析 hostname/port 名，输出快 |

输出字段：COMMAND、PID、USER、FD、TYPE、DEVICE、SIZE/OFF、NODE、NAME。

## 三、文件描述符 FD

**数字 FD**：

- 0 = stdin、1 = stdout、2 = stderr、3+ = 其他文件/socket/pipe
- `open()`/`socket()`/`dup()` 返回数字 FD；可重定向（dup2）、可共享（同一文件多个 FD）

**特殊 FD**：

| FD  | 含义                               |
| --- | ---------------------------------- |
| cwd | current working directory          |
| rtd | root directory（进程看到的根目录） |
| txt | text，程序执行文件                 |
| mem | memory-mapped file                 |
| DEL | deleted file，已删除但仍被占用     |
| PD  | parent directory                   |

**权限标记**：数字 FD 后的 r/w/u/m → read / write / read-write / mmap。例如 `3u IPv4 TCP 127.0.0.1:8080->...`。

## 四、实战拆解示例

```
roslaunch 21610 shihao rtd DIR 8,48 4096 2 /
```

- COMMAND=roslaunch、PID=21610、USER=shihao
- FD=**rtd**：Root Directory（进程根目录，内核结构中的"锚点"而非 open() 得到的 FD）
- TYPE=DIR、DEVICE=8,48（主,次设备号）、SIZE/OFF=4096、NODE=2、NAME=/

含义：该进程**未被 chroot/容器隔离**，看到的是系统真正的 `/`。

## 五、inode 是什么

inode = Index Node（索引节点）：每个文件/目录在文件系统里的唯一标识。

- 存：文件类型、权限、拥有者、大小、时间戳、数据块指针、链接计数
- **不存文件名**：文件名在目录里（目录是"文件名 → inode 的映射表"）
- 传统约定：inode 1 保留；inode 2 = 根目录 `/`；3+ 为普通文件/目录
- 硬链接指向同一 inode；链接计数 > 0 时删除文件名文件仍存在（DEL FD 场景）

三层关系：**FD = 进程角度，inode = 文件系统角度，文件名 = 用户角度**。

## 补充与纠正

1. **inode 2 约定并非绝对**：inode 2 是根目录在 ext 系列文件系统的传统约定，但**并非所有文件系统都如此**（如某些网络/特殊文件系统）。"看到 NODE=2 表示健康"是经验判断，不是普适定律。
2. **DEVICE 的 `8,48`**：主设备号 8 通常是 SCSI/SATA 盘，48 为次设备号（分区号），可经 `ls -l /dev/` 或 `stat -c %D` 核对。
3. **`rtd` 与安全隔离**：容器中（如 Docker 默认命名空间）进程的 rtd 指向容器自己的根，但很多情况 NAME 仍显示为 `/`，需结合 `ls -l /proc/<pid>/root` 判断真正的根文件系统。
4. **lsof 的权限限制**：普通用户只能看到自己进程的 FD；看全系统需要 root。部分信息来源是 `/proc/<pid>/fd`、`/proc/<pid>/maps`。
5. **配合技巧**：`lsof +L1` 列出所有 deleted 但仍被占用的文件（排查磁盘空间未释放）；`fuser`、`ss`/`netstat` 可补充端口与 socket 排查。
