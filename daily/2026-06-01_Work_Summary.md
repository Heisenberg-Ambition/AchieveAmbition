# 工作总结 - 2026-06-01

## 一、三方库版本回退适配问题

### 背景

项目原本基于较新的三方库版本开发，代码中已经增加了对 31 版本接口的支持。

旧版本头文件中仅存在：

```cpp
#ifndef _30_H_20160503__
#define _30_H_20160503__
```

新版本头文件中新增：

```cpp
#ifndef _31_H_20160503__
#define _31_H_20160503__
```

项目代码已经分别实现了 30 版本和 31 版本对应逻辑。

### 问题现象

回退到旧版本三方库后：

* 31 版本相关头文件不存在
* 31 版本结构体不存在
* 31 版本接口不存在
* 编译阶段出现大量类型和接口找不到的问题

### 原因分析

代码中存在类似：

```cpp
#ifdef _31_H_20160503__
    // Version 31 implementation
#endif
```

当旧版本头文件中不存在该宏时：

* 31 版本实现不会参与编译
* 依赖 31 版本结构体和接口的代码失效

### 解决方案

建立统一版本适配头文件：

```cpp
#ifdef _30_H_20160503__
#define SDK_VER_30
#endif

#ifdef _31_H_20160503__
#define SDK_VER_31
#endif
```

业务代码统一使用：

```cpp
#if defined(SDK_VER_31)
    ...
#elif defined(SDK_VER_30)
    ...
#endif
```

避免直接依赖第三方头文件中的版本宏。

---

## 二、Clang-Format extern "C" 格式化问题

### 需求

希望格式化结果如下：

```cpp
extern "C"
{
    #include <xxx.h>
}
```

### 当前配置

```yaml
IndentExternBlock: AfterExternBlock

BraceWrapping:
  AfterExternBlock: true
```

### 问题现象

格式化后结果与预期不一致：

```cpp
extern "C" {
#include <xxx.h>
}
```

### 原因分析

`BraceWrapping` 配置只有在：

```yaml
BreakBeforeBraces: Custom
```

时才会生效。

否则：

```yaml
BraceWrapping:
```

中的所有配置都会被忽略。

### 修正方案

```yaml
BreakBeforeBraces: Custom

IndentExternBlock: AfterExternBlock

BraceWrapping:
  AfterExternBlock: true
```

### 进一步发现

对于：

```cpp
#include <xxx.h>
```

clang-format 不会稳定地按照 `IndentExternBlock` 进行缩进。

这是 clang-format 长期存在的行为限制。

---

## 三、spdlog/fmt 指针格式化编译错误

### 问题现象

编译时报错：

```text
static assertion failed:
'formatting of non-void pointers is disallowed'
```

### 原因分析

新版 fmt 不允许直接格式化非 void 指针：

```cpp
LOG_INFO("{}", frame);
```

其中：

```cpp
AVFrame* frame;
```

属于非 void 指针。

### 解决方案

打印地址时使用：

```cpp
LOG_INFO("{}", fmt::ptr(frame));
```

或者：

```cpp
LOG_INFO("{}", static_cast<void*>(frame));
```

推荐使用：

```cpp
fmt::ptr(...)
```

因为这是 fmt 官方推荐写法。

---

## 四、LNK2019 链接错误排查

### 错误信息

```text
LNK2019:
无法解析的外部符号

FormatResult::format2json(...)
```

### 现象

编译通过：

```text
Compile Success
```

链接失败：

```text
LNK2019
LNK1120
```

### 原因分析

头文件中存在声明：

```cpp
int format2json(...);
```

但链接阶段找不到对应实现。

可能原因：

#### 原因1

实现被版本宏屏蔽：

```cpp
#ifdef SDK_VER_31

int FormatResult::format2json(...)
{
}

#endif
```

回退到 30 版本后：

```cpp
SDK_VER_31
```

不存在。

导致实现未参与编译。

#### 原因2

函数签名发生变化：

例如：

```cpp
ArcDrvAdasData*
```

与：

```cpp
const ArcDrvAdasData*
```

不一致。

导致名字修饰（Name Mangling）不同。

#### 原因3

对应 cpp 文件未加入编译目标。

### 排查方向

搜索：

```cpp
FormatResult::format2json
```

确认：

* 是否存在实现
* 是否被宏包裹
* 是否签名一致
* 是否参与编译

---

## 五、Git Push Upstream 问题

### 问题现象

执行：

```bash
git push
```

提示：

```text
To have this happen automatically for branches without a tracking upstream
```

### 原因分析

当前分支没有关联远程分支。

Git 无法确定：

```bash
git push
```

应该推送到哪个远程分支。

### 临时解决方案

```bash
git push -u origin <branch_name>
```

例如：

```bash
git push -u origin dev
```

### 永久解决方案

开启自动关联：

```bash
git config --global push.autoSetupRemote true
```

以后首次推送分支时自动建立 upstream。

---

# 今日收获

1. 熟悉了三方库版本兼容适配方案。
2. 理解了 clang-format 中 `BreakBeforeBraces` 与 `BraceWrapping` 的关系。
3. 掌握了 fmt 对非 void 指针格式化的限制及解决方式。
4. 学会从 LNK2019 角度分析声明与实现不匹配问题。
5. 了解 Git Upstream 分支关联机制及自动配置方式。

# 待处理事项

* 完成 30/31 版本兼容层整理。
* 修复 `FormatResult::format2json()` 链接问题。
* 清理项目中的版本宏散布问题。
* 统一版本检测入口，避免后续新增版本造成维护困难。
