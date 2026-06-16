# 2026-06-11 开发问题总结

## 一、Python 调用 AdasLogDump.exe 时路径包含空格导致失败

### 问题现象

原始代码：

```python
input_video_path = os.path.dirname(video_path)

cmd = os.path.join(
    script_directory,
    f"AdasLogDump",
    f'{AdasLogDump}/AdasLogDump.exe "{input_video_path}" "output" -b'
)

os.system(cmd)
```

日志：

```text
'C:\ppp' 不是内部或外部命令，也不是可运行的程序或批处理文件。
```

或者：

```text
WinError 2
No such file or directory
```

---

### 问题原因

当路径中包含空格：

```text
C:\Program Files (x86)\...
```

时：

```python
os.system()
```

依赖 shell 解析命令。

命令字符串中的引号、空格、转义很容易出问题。

例如：

```text
C:\Program Files (x86)\Test\
```

会被 shell 解析成：

```text
C:\Program
Files
(x86)\Test
```

导致执行失败。

---

### 解决方案

改用：

```python
subprocess.run()
```

不要手工拼接命令字符串。

```python
result = subprocess.run(
    [
        exe_path,
        input_video_path,
        "output",
        "-b"
    ],
    cwd=work_dir,
    capture_output=True,
    text=True,
    encoding="utf-8",
    errors="ignore",
    check=True
)
```

优点：

* 自动处理空格
* 自动处理转义
* 可获取 stdout/stderr
* 更稳定
* 跨平台

---

## 二、Python 文件移动时报 WinError

### 问题现象

```python
shutil.move(src_bin_file, dst_bin_file)
```

日志：

```text
[Errno 2] No such file or directory
```

---

### 原因

不是路径中的空格导致。

实际原因：

```text
AdasLogDump.exe 执行失败
↓
output目录没有生成bin文件
↓
shutil.move找不到源文件
```

因此：

```python
src_bin_file
```

根本不存在。

---

### 排查方法

增加：

```python
logger.info(f"src={src_bin_file}")
logger.info(f"exist={os.path.exists(src_bin_file)}")
```

确认文件是否真的生成。

---

## 三、MCAP Schema 自动获取 Proto Descriptor

### 原始写法

```cpp
Common::addFileDescriptor(
    foxglove::CompressedImage::descriptor()->file(),
    fds
);
```

问题：

只能处理：

```cpp
foxglove::CompressedImage
```

不具备通用性。

---

### 优化方案

通过：

```cpp
DescriptorPool::generated_pool()
```

动态查找。

```cpp
auto descriptor =
    google::protobuf::DescriptorPool::generated_pool()
        ->FindMessageTypeByName(this->m_schema_.name);
```

然后：

```cpp
Common::addFileDescriptor(
    descriptor->file(),
    fds
);
```

---

### 优点

新增 Proto 时：

```cpp
custom.PCPTTimestampInfo
custom.VehicleInfo
custom.LaneInfo
```

无需修改代码。

自动生成 Schema。

---

## 四、std::string 初始化方式

### 写法1

```cpp
std::string descriptorBytes = "";
```

### 写法2

```cpp
std::string descriptorBytes;
```

---

### 区别

几乎没有区别。

两者最终都会得到：

```cpp
descriptorBytes.empty() == true
```

---

### 推荐

```cpp
std::string descriptorBytes;
```

更现代。

避免无意义构造。

---

## 五、std::span 学习

### 作用

非拥有型数组视图。

例如：

```cpp
std::span<uint8_t> data;
```

类似：

```text
pointer + size
```

---

### 优点

代替：

```cpp
uint8_t* data
size_t size
```

写法。

更安全。

支持：

```cpp
vector
array
C数组
```

统一接口。

---

## 六、PolynomialFit 算法分析

### 功能

对离散点进行：

```text
最小二乘多项式拟合
```

支持：

```cpp
CONSTANT
LINEAR
QUADRATIC
CUBIC
```

最高三次曲线。

---

### 输入

```text
x[]
y[]
```

例如：

```text
x: 0 10 20 30 40
y: 0 0.1 0.5 1.4 3.0
```

---

### 内部流程

构建：

```text
Normal Equation
```

即：

```text
A·x=b
```

然后：

```cpp
Gaussian Elimination
```

求解系数。

---

### 输出

得到：

```text
y = a + bx + cx² + dx³
```

---

### 支持

```cpp
eval()
```

任意位置求值。

```cpp
derivative()
```

求一阶导数。

```cpp
secondDerivative()
```

求二阶导数。

```cpp
rmse()
```

计算拟合误差。

---

## 七、使用 x 和 z 进行拟合

### 输入

```python
x = [pt["x"] for pt in laneWorldPts]
z = [-pt["z"] for pt in laneWorldPts]
```

---

### Python实现

```python
import numpy as np

def fit_z(x, z, x_new=None):

    if len(x) != len(z):
        raise ValueError()

    if len(x) < 2:
        return z.copy()

    order = min(3, len(x) - 1)

    coeff = np.polyfit(x, z, order)

    if x_new is None:
        x_new = x

    z_new = np.polyval(coeff, x_new)

    return z_new.tolist()
```

---

### 特点

默认：

```python
x_new = x
```

因此：

```text
输出点数量
=
输入点数量
```

---

### np.polyfit 与 C++ 版本区别

C++：

```text
Normal Equation
+
Gaussian Elimination
```

NumPy：

```text
Least Squares
+
SVD / QR
```

数值稳定性更好。

在车道线场景：

```text
误差通常 < 1e-5
```

几乎可以认为一致。

---

## 八、Tkinter Canvas 事件绑定

常见绑定：

```python
<ButtonPress-1>
```

左键按下。

```python
<B1-Motion>
```

左键拖动。

```python
<ButtonRelease-1>
```

左键释放。

```python
<MouseWheel>
```

滚轮缩放。

```python
<ButtonPress-3>
```

右键开始平移。

```python
<B3-Motion>
```

右键拖动画布。

---

### Linux注意事项

Windows：

```python
<MouseWheel>
```

Linux：

```python
<Button-4>
<Button-5>
```

建议同时支持。

---

## 九、NV12 文件保存

### 原始需求

保存：

```cpp
ASVLOFFSCREEN
```

到：

```text
指定目录
```

---

### 最终方案

```cpp
std::filesystem::create_directories(folder);

std::filesystem::path filepath =
    std::filesystem::path(folder) / filename;

std::ofstream file(
    filepath,
    std::ios::binary
);
```

---

### 优点

支持：

```text
Windows
Linux
路径包含空格
```

---

## 十、NV12 文件为空排查

重点检查：

```cpp
asvl_offscreen->ppu8Plane[0]
asvl_offscreen->ppu8Plane[1]

asvl_offscreen->pi32Pitch[0]
asvl_offscreen->pi32Pitch[1]

asvl_offscreen->u32PixelArrayFormat
```

打印：

```cpp
LOG_INFO("plane0={}", ...);
LOG_INFO("plane1={}", ...);

LOG_INFO("pitch0={}", ...);
LOG_INFO("pitch1={}", ...);

LOG_INFO("format={}", ...);
```

重点判断：

### 情况1

```text
plane为空
```

数据已释放。

---

### 情况2

```text
pitch为0
```

写入长度为0。

---

### 情况3

```text
不是NV12格式
```

例如：

```text
NV21
I420
RGB
```

---

## 十一、Git Stash 单个文件

### 推荐写法

```bash
git stash push src/main.cpp
```

---

### 多文件

```bash
git stash push file1 file2
```

---

### 带备注

```bash
git stash push -m "stash main.cpp" src/main.cpp
```

---

### 恢复

查看：

```bash
git stash list
```

恢复：

```bash
git stash pop stash@{0}
```

或者：

```bash
git stash apply stash@{0}
```

区别：

```text
apply 保留stash
pop 删除stash
```

---

# 本次收获总结

1. 使用 subprocess.run 替代 os.system。
2. Windows 路径带空格必须避免字符串拼接命令。
3. MCAP Schema 可通过 DescriptorPool 自动获取。
4. PolynomialFit 本质是三次最小二乘拟合。
5. np.polyfit 可直接替代当前 Python 侧拟合需求。
6. std::filesystem 可彻底解决路径拼接问题。
7. NV12 空文件优先检查 plane、pitch、format。
8. Git 支持直接 stash 单个文件。
9. Linux 与 Windows Tkinter 鼠标滚轮事件不同。
10. C++17 filesystem 已成为跨平台文件操作首选方案。
