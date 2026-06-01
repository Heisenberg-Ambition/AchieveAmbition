# 今日 Windows/C++/Python 工程问题总结 🚀

日期：2026-05-21

---

# 1. sccache 导致 MSVC 输出乱码 ⭐⭐⭐⭐⭐

## 现象

开启：

```cmake
set(CMAKE_CXX_COMPILER_LAUNCHER sccache)
```

后：

```text
注锟?
ע�:
```

等乱码大量出现。

并且：

* 每个 cpp 编译时都会出现
* `/showIncludes` 输出乱码
* 关闭 sccache 后恢复正常

---

## 根因

Windows 下：

```text
MSVC + Ninja + sccache + 中文系统
```

组合存在编码问题。

尤其：

```text
/showIncludes
```

输出会被：

* Ninja
* sccache

解析。

中文：

```text
注意: 包含文件:
```

容易编码炸裂。

---

## 结论

不是：

* cmd
* chcp
* UTF8

的问题。

而是：

# sccache 的 Windows 输出流编码问题。

---

## 解决方案

### 方案1（最终采用）

关闭：

```cmake
set(CMAKE_CXX_COMPILER_LAUNCHER sccache)
```

---

### 方案2（可选）

使用：

```cmake
target_precompile_headers(...)
```

替代 sccache 提升编译速度。

---

# 2. C2059 `syntax error ')'` 实际是假错误 ⭐⭐⭐⭐⭐

## 错误

```text
error C2059: 语法错误:“)”
```

位置：

```text
AppConfig.cpp(35)
```

---

## 实际根因

真正错误是：

```text
fatal error C1034: string: 不包括路径集
```

即：

```cpp
#include <string>
```

失败。

---

## 原因

执行：

```bat
cl /P AppConfig.cpp
```

时：

没有初始化 VS 编译环境。

---

## 正确方式

先：

```bat
call vcvars64.bat
```

再：

```bat
cl /P AppConfig.cpp
```

---

# 3. `warning C4819` 文件编码问题 ⭐⭐⭐⭐

## 错误

```text
warning C4819:
该文件包含不能在当前代码页(936)中表示的字符
```

---

## 根因

文件实际编码：

```text
GBK
```

但编译器：

```cmake
/utf-8
```

按 UTF8 解析。

---

## 解决

VSCode：

```text
Save with Encoding
→ UTF-8
```

推荐：

```text
UTF-8 without BOM
```

---

# 4. `/FS` `/Zi` `/Z7` 区别 ⭐⭐⭐⭐

---

## `/FS`

强制：

```text
PDB 写入串行化
```

解决：

```text
fatal error C1041
```

多个 cl.exe 同时写 pdb。

---

## `/Zi`

生成：

```text
独立 PDB
```

调试信息。

适合：

* IDE
* 调试

---

## `/Z7`

调试信息直接写入：

```text
.obj
```

不依赖统一 pdb。

适合：

* Ninja
* 并行编译
* CI

---

## 当前推荐

```cmake
/Z7 /FS
```

---

# 5. `LOG_INFO` 在 `CLI::App` 附近异常 ⭐⭐⭐⭐⭐

## 现象

```cpp
std::cout
```

正常。

但：

```cpp
LOG_INFO(...)
```

在：

```cpp
CLI::App
```

附近失效。

---

## 推测

高概率：

# 宏展开污染。

---

## 原因

`LOG_INFO` 大概率是：

```cpp
#define LOG_INFO(...)
```

而：

```cpp
CLI11.hpp
```

内部：

* 模板
* 宏
* formatter
* operator<<

非常复杂。

---

## 结论

问题不像：

* 编码
* stdout

更像：

# 宏冲突 / 宏展开异常。

---

## 建议

避免：

```cpp
LOG_INFO
```

这种通用宏名。

改为：

```cpp
APP_LOG_INFO
ARC_LOG_INFO
```

---

# 6. PowerShell Tee 输出日志 ⭐⭐⭐⭐⭐

## 目标

运行 exe：

* 保留控制台输出
* 同时保存日志

---

## 最终方案

```powershell
$ts = Get-Date -Format "yyyyMMdd_HHmmss"

.\AdasLogDump.exe 2>&1 |
    Tee-Object "../log/output_$ts.log"
```

---

## 效果

生成：

```text
log/output_20260521_145830.log
```

---

# 7. OpenCV `cv2.polylines` 报错 ⭐⭐⭐⭐

## 错误

```text
(-215:Assertion failed)
p.checkVector(2, CV_32S) >= 0
```

---

## 根因

传入点格式错误。

---

## 正确格式

```python
points = np.array(points, dtype=np.int32).reshape(-1, 1, 2)
```

---

# 8. NumPy shape 问题 ⭐⭐⭐⭐

## 错误

```text
ValueError:
setting an array element with a sequence
```

---

## 根因

数组：

```python
shape 不一致
```

例如：

```python
[[1,2], [3,4,5]]
```

---

## 正确构造三维点

```python
gp_points = np.column_stack((x, y, z))
```

---

# 9. `IndexError: index 2 is out of bounds` ⭐⭐⭐⭐

## 错误

```python
gd_point[i][2]
```

但：

```python
shape=(N,2)
```

---

## 根因

代码假设：

```python
[x, y, z]
```

实际只有：

```python
[x, y]
```

---

## 解决

补：

```python
z = np.zeros_like(x)
```

---

# 10. Windows 长路径问题 ⭐⭐⭐⭐

## 问题

```python
shutil.move()
```

报：

```text
No such file or directory
```

但文件存在。

---

## 根因

UNC 路径 + 超长路径。

Windows 默认：

```text
MAX_PATH = 260
```

---

## 建议

开启：

```text
LongPathsEnabled
```

或：

```python
\\?\
```

前缀。

---

# 11. Ninja 与 CMake Generator 冲突 ⭐⭐⭐

## 错误

```text
generator : NMake Makefiles
Does not match previously used: Ninja
```

---

## 根因

build 目录缓存了旧 generator。

---

## 解决

删除：

```text
build/
CMakeCache.txt
CMakeFiles/
```

重新 cmake。

---

# 12. `sys/time.h` Windows 不存在 ⭐⭐⭐⭐

## Linux 代码

```cpp
gettimeofday()
```

---

## Windows 解决

改：

```cpp
std::chrono
```

---

## 推荐

```cpp
std::chrono::system_clock
```

---

# 13. `std::vector<const std::string>` 非法 ⭐⭐⭐⭐

## 错误

```text
std::vector must have a non-const value_type
```

---

## 原因

```cpp
std::vector<const std::string>
```

非法。

vector 元素必须可赋值。

---

## 正确

```cpp
std::vector<std::string>
```

---

# 14. `Non-const lvalue reference cannot bind to temporary` ⭐⭐⭐⭐

## 错误

```text
cannot bind to temporary
```

---

## 原因

```cpp
foo(std::string&)
```

传入：

```cpp
foo("abc")
```

产生临时对象。

---

## 正确

```cpp
const std::string&
```

或者：

```cpp
std::string
```

---

# 今日结论 ⭐⭐⭐⭐⭐

Windows C++ 工程最容易踩坑的几个方向：

---

## 1. 编码

* UTF8
* GBK
* MSVC
* Console

---

## 2. 宏污染

* Windows.h
* logger
* 第三方库

---

## 3. CMake/Ninja/MSVC 配合

* generator
* pdb
* parallel build

---

## 4. Python/OpenCV/Numpy shape

* reshape
* dtype
* ndarray structure

---

## 5. Windows 历史兼容问题 😂

* MAX_PATH
* code page
* showIncludes
* pdb locking

---

# 当前工程推荐配置 ⭐⭐⭐⭐⭐

```cmake
add_compile_options(
    /utf-8
    /Z7
    /FS
)

set(CMAKE_UNITY_BUILD ON)
```

配合：

```cmake
target_precompile_headers(...)
```

替代 sccache。
