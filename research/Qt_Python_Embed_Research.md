下面是一份可以直接保存为 `Qt_Python_Embed_Research.md` 的总结。

````md
# Qt + Python 混合项目研究总结

## 研究目标

探索一种适合当前项目架构的方案：

```text
Qt(C++)
    ↓
Python
    ↓
NumPy / OpenCV / PyTorch
````

其中：

* Qt负责GUI
* C++负责主程序框架
* Python负责算法实现
* 后续支持OpenCV、NumPy、AI模型推理

---

# 第一阶段：Qt环境搭建

## Qt版本

最终确认：

```text
Qt 5.15.2
```

---

## CMake接入Qt

```cmake
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/qt")

find_package(
    Qt5 REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets
)
```

---

## 遇到的问题

### stdext找不到

报错：

```text
qvector.h
qlist.h

error C3861: stdext
```

原因：

```text
Qt5.15 + 新版MSVC
+
C++20
```

兼容性问题。

---

### 解决方案

修改：

```cmake
set(CMAKE_CXX_STANDARD 17)
```

成功编译。

---

# 第二阶段：Qt运行时部署

## 遇到的问题

程序启动时报错：

```text
This application failed to start because no Qt platform plugin
could be initialized.
```

---

## 原因

缺少平台插件：

```text
platforms/qwindows.dll
```

---

## 错误部署方式

之前将所有dll直接复制到：

```text
bin/
```

例如：

```text
bin/
├── Qt5Core.dll
├── Qt5Gui.dll
├── Qt5Widgets.dll
├── qwindows.dll
```

Qt无法找到平台插件。

---

## 正确结构

```text
bin/
├── App.exe
├── Qt5Core.dll
├── Qt5Gui.dll
├── Qt5Widgets.dll
│
└── platforms/
    └── qwindows.dll
```

---

## 推荐方案

使用：

```text
windeployqt.exe
```

自动部署。

---

# 第三阶段：嵌入Python解释器

## 目标

实现：

```text
C++
    ↓
Python
    ↓
调用Python函数
    ↓
获取返回值
```

---

## CMake配置

```cmake
find_package(
    Python3 REQUIRED
    COMPONENTS
        Interpreter
        Development
)

target_link_libraries(
    ${PROJECT_NAME}
    Python3::Python
)
```

---

## 最简单验证

```cpp
#include <Python.h>

int main()
{
    Py_Initialize();

    PyRun_SimpleString(
        "print('Hello Python')"
    );

    Py_Finalize();
}
```

输出：

```text
Hello Python
```

---

# 第四阶段：导入Python模块

## 目录结构

```text
Project
│
├── python
│   └── analysis.py
│
└── bin
```

---

## Python代码

```python
# analysis.py

print("analysis imported")

def add(a, b):
    print("add called")
    return a + b
```

---

## 导入模块

```cpp
PyRun_SimpleString(
    "import sys\n"
    "sys.path.append('./python')"
);

PyObject* moduleName =
    PyUnicode_FromString("analysis");

PyObject* module =
    PyImport_Import(moduleName);
```

---

# 第五阶段：调用Python函数

## 获取函数

```cpp
PyObject* func =
    PyObject_GetAttrString(
        module,
        "add");
```

---

## 构造参数

```cpp
PyObject* args =
    PyTuple_Pack(
        2,
        PyLong_FromLong(1),
        PyLong_FromLong(2));
```

---

## 调用函数

```cpp
PyObject* result =
    PyObject_CallObject(
        func,
        args);
```

---

## 获取返回值

```cpp
long value =
    PyLong_AsLong(result);

std::cout
    << "Result = "
    << value
    << std::endl;
```

输出：

```text
Result = 3
```

---

# 调试经验

## Python异常必须打印

推荐：

```cpp
if (result == nullptr)
{
    PyErr_Print();
}
```

否则Python异常无法看到。

---

## 验证模块是否导入

在Python中增加：

```python
print("analysis imported")
```

---

## 验证函数是否执行

```python
print("add called")
```

---

## 输出顺序问题

观察到：

```text
Hello Python
Result = 3
analysis imported
add called
```

或：

```text
Hello Python
analysis imported
add called
Result = 3
```

顺序可能不稳定。

原因：

```text
C++ stdout
Python stdout
```

使用不同缓冲机制。

属于正常现象。

---

# Python C API评估

当前方案：

```text
PyObject*
PyTuple*
PyImport_Import
PyObject_CallObject
```

优点：

* 理解底层机制
* 无第三方依赖

缺点：

* 代码冗长
* 引用计数复杂
* NumPy集成困难
* 后期维护成本高

---

# 最终架构决策

决定放弃继续扩展：

```text
Python C API
```

转向：

```text
pybind11::embed
```

---

# 为什么选择pybind11

例如：

Python：

```python
def add(a, b):
    return a + b
```

---

Python C API：

```cpp
PyTuple_Pack(...)
PyObject_CallObject(...)
PyLong_AsLong(...)
```

大量样板代码。

---

pybind11：

```cpp
int result =
    analysis.attr("add")(1, 2)
        .cast<int>();
```

更加现代化。

---

# pybind11安装方案

推荐：

```text
项目内集成源码
```

目录：

```text
TheGreatTry
│
├── pybind11
│   └── include
│
├── qt
├── ffmpeg
└── src
```

---

CMake：

```cmake
include_directories(
    ${PROJECT_SOURCE_DIR}/pybind11/include
)
```

---

仍需链接：

```cmake
find_package(
    Python3 REQUIRED
    COMPONENTS
        Interpreter
        Development
)

target_link_libraries(
    ${PROJECT_NAME}
    Python3::Python
)
```

---

# 下一步规划

## 第一阶段

引入：

```text
pybind11::embed
```

实现：

```cpp
class PythonManager
{
public:

    bool Initialize();

    template<typename T>
    T Call(
        const std::string& module,
        const std::string& func,
        ...);
};
```

---

## 第二阶段

支持：

```text
Python String
Python Dict
Python List
```

---

## 第三阶段

支持：

```text
NumPy ndarray
```

---

## 第四阶段

实现：

```text
cv::Mat
    ↔
numpy.ndarray
```

---

## 第五阶段

支持：

```text
OpenCV
PyTorch
YOLO
```

---

# 目标架构

```text
Qt GUI
    ↓
PythonManager
    ↓
pybind11::embed
    ↓
Python
    ↓
NumPy
    ↓
OpenCV
    ↓
PyTorch
```

最终形成：

```text
Qt(C++)负责界面
Python负责算法
FFmpeg负责视频
OpenCV负责图像
```

的混合项目架构。

```

这份总结基本覆盖了本次从 **Qt接入 → Python解释器嵌入 → Python函数调用 → pybind11技术选型** 的完整过程。
```
