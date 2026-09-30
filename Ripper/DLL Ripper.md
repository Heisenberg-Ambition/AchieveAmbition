# DLL Ripper 总结

## 一、核心函数

| 函数                     | 功能                         |
| ------------------------ | ---------------------------- |
| `dlopen(filename, flag)` | 打开动态库，返回句柄         |
| `dlsym(handle, symbol)`  | 查找库里的符号（函数、变量） |
| `dlclose(handle)`        | 关闭动态库                   |
| `dlerror()`              | 获取最近一次动态链接错误     |

`dlopen` 的两个常用 flag：

- `RTLD_LAZY`：延迟解析符号，真正调用时才绑定
- `RTLD_NOW`：打开库时立即解析所有符号

典型用法：

```cpp
#include <dlfcn.h>

void* handle = dlopen("libm.so", RTLD_LAZY);

typedef double (*sin_func_t)(double);

sin_func_t mysin = (sin_func_t)dlsym(handle, "sin");

mysin(0.5);

dlclose(handle);
```

## 二、为什么用 dlfcn.h

- 写插件系统 / 模块化程序
- 运行时决定用哪个库
- 不想每次编译都静态链接
- 可动态更新库（替换 .so）而不重启程序

## 三、注意事项

- **类型安全靠自己**：`dlsym` 返回 `void*`，必须手动 cast，不匹配会 crash。
- **符号名必须正确**：C++ 编译有 name mangling，可用 `extern "C"` 避免。
- **`RTLD_GLOBAL` / `RTLD_LOCAL`** 控制符号可见性。
- **跨平台**：Windows 用 `LoadLibrary` / `GetProcAddress`，不是 dlfcn.h。

## 四、extern vs dlopen 对比

| 特性     | extern           | dlfcn.h / dlopen                        |
| -------- | ---------------- | --------------------------------------- |
| 绑定时机 | 编译期 / 链接期  | 运行期                                  |
| 使用对象 | 编译期可见的符号 | 动态库里的任意符号                      |
| 类型检查 | 编译期类型检查   | 手动 cast，类型不安全                   |
| 灵活性   | 固定             | 高，可插件化/热插拔                     |
| 跨平台   | 标准 C/C++       | Linux/Unix 专用，Windows 用 LoadLibrary |

比喻：extern = 提前写好通讯录，等链接器找到；dlopen = 运行时去电话簿找人，今天 Alice 明天 Bob。

## 补充与纠正

1. **`void*` 转函数指针的合法性**：C++ 标准不允许将 `void*` 直接 reinterpret_cast 为函数指针（实现定义），POSIX 允许；现代写法建议 `reinterpret_cast<FuncType>(dlsym(...))`（GCC/Clang 可用），或用 `memcpy` 拷贝指针以完全符合标准。多数生产代码直接用 cast 并注释平台假设。
2. **`dlsym` 返回值判空陷阱**：若符号本身是值为 NULL 的数据对象，`dlsym` 返回 NULL 会与"符号不存在"混淆，需先 `dlerror()` 清空错误再判断 `dlerror() != NULL`。
3. **句柄管理**：`dlopen` 是引用计数制，多次 dlopen 需对应多次 dlclose 才真正卸载；库卸载后指向其内部函数的指针立即失效。
4. **`extern "C"` 的作用范围**：只影响链接期符号名，不影响调用约定；`__attribute__((visibility))` 与链接脚本可进一步控制导出符号。
5. **声明与定义细节**：`extern int x;` 只是声明不产生存储；与 `dlopen` 的最大区别是 extern 的符号必须在链接期被解析到（否则链接错误），而 dlopen 的符号到运行期才解析。
