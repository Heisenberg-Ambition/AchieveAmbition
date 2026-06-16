# Windows 11 + Visual Studio 2026 使用 vcpkg 安装 Protobuf（完整实践记录）

## 一、背景

本次目标：

- Windows 11
- Visual Studio 2026
- CMake
- Foxglove Proto
- MCAP项目

原计划：

```text
下载 protobuf-35.0 源码
自行编译 libprotobuf.lib
自行编译 protoc.exe
```

但是在实际编译过程中遇到了大量 Abseil 链接错误：

```text
absl::Cord::AppendPrecise
absl::Base64Unescape
absl::numbers_internal::safe_strto64_base
...
```

最终发现：

```text
protobuf 35.0
+
abseil 20250512
+
MSVC 14.51 (VS2026)
```

存在兼容性问题。

因此最终方案调整为：

```text
使用 vcpkg 安装 protobuf
```

优点：

- 不需要自己编译 protobuf
- 自动处理 Abseil 依赖
- 自动处理 utf8_range 依赖
- 自动生成 protoc
- 与 CMake 无缝集成
- 后续升级方便

---

# 二、安装 vcpkg

## 1. 准备目录

例如：

```text
F:
└── Tools
```

进入：

```powershell
cd F:\Tools
```

---

## 2. 下载 vcpkg

执行：

```powershell
git clone https://github.com/microsoft/vcpkg.git
```

得到：

```text
F:\Tools\vcpkg
```

---

## 3. 编译 vcpkg

进入目录：

```powershell
cd F:\Tools\vcpkg
```

执行：

```powershell
.\bootstrap-vcpkg.bat
```

成功后出现：

```text
F:\Tools\vcpkg\vcpkg.exe
```

验证：

```powershell
.\vcpkg.exe version
```

例如：

```text
vcpkg package management program version ...
```

说明安装成功。

---

# 三、安装 Protobuf

推荐：

```text
静态库版本
```

安装命令：

```powershell
.\vcpkg.exe install protobuf:x64-windows-static
```

等待完成。

---

## 为什么选择 static？

相比：

```text
protobuf:x64-windows
```

动态库版本会产生：

```text
libprotobuf.dll
abseil_dll.dll
```

运行时需要复制 DLL。

而：

```text
protobuf:x64-windows-static
```

优点：

- 不需要 DLL
- 发布简单
- 不容易出现链接问题
- 更适合单 EXE 项目

因此推荐：

```powershell
.\vcpkg.exe install protobuf:x64-windows-static
```

---

# 四、安装完成后的目录结构

安装完成后：

```text
F:\Tools\vcpkg
└── installed
    └── x64-windows-static
        ├── include
        │   └── google
        │       └── protobuf
        │
        ├── lib
        │   ├── libprotobuf.lib
        │   ├── libprotobuf-lite.lib
        │   └── libprotoc.lib
        │
        └── tools
            └── protobuf
                └── protoc.exe
```

---

# 五、验证安装

## 查看 protoc 版本

执行：

```powershell
F:\Tools\vcpkg\installed\x64-windows-static\tools\protobuf\protoc.exe --version
```

输出：

```text
libprotoc 35.0
```

说明安装成功。

---

# 六、protoc 的作用

protoc 是：

```text
Proto 编译器
```

负责：

```text
xxx.proto
    ↓
protoc
    ↓
xxx.pb.h
xxx.pb.cc
```

例如：

```proto
syntax = "proto3";

message Person {
    string name = 1;
    int32 age = 2;
}
```

执行：

```powershell
protoc --cpp_out=. Person.proto
```

生成：

```text
Person.pb.h
Person.pb.cc
```

---

# 七、生成 Foxglove 的 pb 文件

目录：

```text
message
└── foxglove
    ├── SceneUpdate.proto
    ├── TextPrimitive.proto
    ├── FrameTransform.proto
    └── ...
```

---

## 使用 protoc

定义：

```powershell
$PROTOC="F:\Tools\vcpkg\installed\x64-windows-static\tools\protobuf\protoc.exe"
```

执行：

```powershell
& $PROTOC `
    -I=message `
    --cpp_out=message `
    message\foxglove\*.proto
```

生成：

```text
message
└── foxglove
    ├── SceneUpdate.pb.h
    ├── SceneUpdate.pb.cc
    ├── TextPrimitive.pb.h
    ├── TextPrimitive.pb.cc
    └── ...
```

---

# 八、推荐的自动生成脚本

## generate_proto.ps1

```powershell
$PROTOC="F:\Tools\vcpkg\installed\x64-windows-static\tools\protobuf\protoc.exe"

Get-ChildItem `
    message\foxglove `
    -Filter *.proto |
ForEach-Object {

    & $PROTOC `
        -I=message `
        --cpp_out=message `
        $_.FullName
}
```

执行：

```powershell
.\generate_proto.ps1
```

自动生成全部 pb 文件。

---

# 九、CMake 集成

## 方式一（推荐）

配置阶段指定 Toolchain：

```powershell
cmake .. ^
    -G "Visual Studio 18 2026" ^
    -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=F:/Tools/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## CMakeLists.txt

```cmake
find_package(Protobuf CONFIG REQUIRED)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    protobuf::libprotobuf
)
```

即可。

---

# 十、删除旧的 protobuf 配置

以前可能有：

```cmake
include_directories(
    ${PROJECT_SOURCE_DIR}/protobuf/include
)
```

删除。

---

以前可能有：

```cmake
link_directories(
    ${PROJECT_SOURCE_DIR}/protobuf/lib
)
```

删除。

---

以前可能有：

```cmake
set(CMAKE_PREFIX_PATH
    "F:/Southlake/Mcap_Cpp/protobuf"
)
```

删除。

---

全部交给 vcpkg 管理。

---

# 十一、验证是否链接成功

测试代码：

```cpp
#include <google/protobuf/message.h>

int main()
{
    return 0;
}
```

编译通过说明：

```text
libprotobuf.lib
```

已经正确链接。

---

# 十二、常见问题

## 1. 找不到 Protobuf

错误：

```text
Could not find Protobuf
```

检查：

```powershell
-DCMAKE_TOOLCHAIN_FILE=...
```

是否传入。

---

## 2. 找不到 protoc

检查：

```text
F:\Tools\vcpkg\installed\x64-windows-static\tools\protobuf
```

是否存在：

```text
protoc.exe
```

---

## 3. 找不到 .proto

确认：

```powershell
-I=message
```

路径正确。

---

## 4. 生成 pb 文件失败

确认：

```powershell
protoc --version
```

正常输出：

```text
libprotoc 35.0
```

---

# 十三、最终推荐目录结构

```text
F:
│
├── Tools
│   └── vcpkg
│
├── Southlake
│   └── Mcap_Cpp
│       ├── CMakeLists.txt
│       ├── message
│       │   └── foxglove
│       │       ├── *.proto
│       │       ├── *.pb.h
│       │       └── *.pb.cc
│       │
│       └── src
│
└── ...
```

---

# 十四、最终结论

本次实践最终采用：

```text
vcpkg + protobuf:x64-windows-static
```

放弃：

```text
protobuf 35.0 源码自行编译
```

原因：

- VS2026 + protobuf35 + Abseil 存在兼容性问题
- 编译过程复杂
- 容易出现大量链接错误

而 vcpkg 方案：

- 自动处理依赖
- 自动安装 protoc
- 自动安装 libprotobuf
- 自动与 CMake 集成
- 基本无需修改业务代码

对于：

```text
Foxglove
MCAP
FFmpeg
CMake
Visual Studio
```

项目，是目前最省时、最稳定的方案。