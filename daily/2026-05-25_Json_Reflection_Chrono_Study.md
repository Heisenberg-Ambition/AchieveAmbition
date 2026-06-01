# C++ JSON 序列化、伪反射与 Chrono 时间处理研究总结

## 研究日期

2026-06-01

---

# 一、nlohmann/json 基础使用

本次主要研究了如何在 C++ 中将结构体/Class 与 JSON 互相转换。

常用库：

```cpp
#include <nlohmann/json.hpp>

using json = nlohmann::json;
```

---

## Struct -> JSON

定义结构体：

```cpp
struct Person
{
    std::string name;
    int age;
};
```

注册序列化：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Person,
    name,
    age
)
```

使用：

```cpp
Person p{"Tom", 18};

json j = p;
```

生成：

```json
{
    "name": "Tom",
    "age": 18
}
```

---

## JSON -> Struct

```cpp
Person p = j.get<Person>();
```

内部自动调用：

```cpp
from_json(...)
```

---

## dump()

```cpp
j.dump();
```

输出紧凑格式：

```json
{"name":"Tom","age":18}
```

---

```cpp
j.dump(4);
```

输出格式化 JSON：

```json
{
    "name": "Tom",
    "age": 18
}
```

参数 4 表示：

```text
每层缩进4个空格
```

类似 Python：

```python
json.dumps(obj, indent=4)
```

---

# 二、NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE 本质

研究过程中发现：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE
```

并不是真正意义上的反射。

本质是：

```text
宏 + 模板 + ADL
```

自动生成：

```cpp
to_json(...)
from_json(...)
```

等价于手写：

```cpp
void to_json(json& j, const Person& p)
{
    j = json{
        {"name", p.name},
        {"age", p.age}
    };
}
```

---

## 为什么必须写 name、age？

因为 C++ 目前没有标准反射能力。

编译器无法自动获得：

```text
字段名
字段数量
字段类型
```

所以必须手动告诉库：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Person,
    name,
    age
)
```

---

# 三、嵌套结构体序列化

结构体嵌套：

```cpp
struct Address
{
    std::string city;
};

struct Person
{
    std::string name;
    Address addr;
};
```

必须先注册子结构体：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Address,
    city
)
```

再注册父结构体：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Person,
    name,
    addr
)
```

---

## 注册顺序要求

必须：

```text
子结构体
    ↓
父结构体
```

否则编译器找不到：

```cpp
to_json(...)
from_json(...)
```

---

# 四、No viable overloaded '=' 问题分析

遇到错误：

```text
No viable overloaded '='

Candidate function not viable:
no known conversion from
'const DecodeInnerAdasInnerVersion'
to 'basic_json<>'
```

---

## 原因

JSON 不知道如何处理：

```cpp
DecodeInnerAdasInnerVersion
```

因为该结构体没有注册：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(...)
```

---

## 解决方案

对子结构体注册：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    DecodeInnerAdasInnerVersion,
    major,
    minor
)
```

然后父结构体再引用。

---

## 排查清单

### 1. 子结构体是否注册

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(...)
```

---

### 2. 注册顺序是否正确

```text
子 -> 父
```

---

### 3. namespace 是否一致

宏最好放在对应 namespace 中。

---

### 4. 成员是否为 private

如果是：

```cpp
NLOHMANN_DEFINE_TYPE_INTRUSIVE
```

或者 friend。

---

# 五、NON_INTRUSIVE 与 INTRUSIVE

## NON_INTRUSIVE

写在类外：

```cpp
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(...)
```

要求：

```text
成员为 public
```

适合：

* POD结构体
* 第三方类型
* SDK对象

---

## INTRUSIVE

写在类内部：

```cpp
NLOHMANN_DEFINE_TYPE_INTRUSIVE(...)
```

可以访问：

```cpp
private
protected
```

成员。

适合：

```cpp
class Person
{
private:
    std::string name;

public:
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        Person,
        name
    )
};
```

---

# 六、C++ 反射研究

目标：

```cpp
foo(Person p, json j);
```

希望：

```text
自动遍历所有成员变量
```

无需手写：

```cpp
name
age
addr
```

---

## 结论

传统 C++ 做不到。

原因：

```text
没有标准反射
```

编译后：

```cpp
name
age
```

这些字段名通常不存在。

---

# 七、Boost.Hana

研究了：

```cpp
BOOST_HANA_DEFINE_STRUCT
```

例如：

```cpp
struct Person
{
    BOOST_HANA_DEFINE_STRUCT(
        Person,
        (std::string, name),
        (int, age)
    );
};
```

---

## Hana 可以获得

* 字段名
* 字段值
* 字段类型

例如：

```cpp
hana::accessors<Person>()
```

能够遍历结构体元数据。

---

## 但仍然需要注册

```cpp
BOOST_HANA_DEFINE_STRUCT
```

原因依然是：

```text
C++没有真正反射
```

---

# 八、Reflection TS 研究

研究了：

```cpp
#include <experimental/reflect>
```

以及：

```cpp
reflect<Point>()
```

的未来反射方案。

---

## 理想中的反射能力

```cpp
auto meta = reflect<Person>();
```

能够获得：

```text
字段名
字段数量
字段类型
方法信息
```

---

## 当前现状

主流编译器：

* GCC
* Clang
* MSVC

基本不支持。

仍属于：

```text
Reflection TS
实验提案
```

阶段。

---

# 九、Chrono 时间处理研究

---

## high_resolution_clock

用于：

```text
性能计时
耗时统计
```

例如：

```cpp
auto start =
    std::chrono::high_resolution_clock::now();
```

---

## system_clock

用于：

```text
现实时间
日志时间
时间戳
```

例如：

```cpp
auto now =
    std::chrono::system_clock::now();
```

---

## 不能直接混用

错误示例：

```cpp
auto start =
    std::chrono::high_resolution_clock::now();

std::chrono::system_clock::to_time_t(start);
```

原因：

```text
clock类型不同
```

---

# 十、时间转字符串

推荐方式：

```cpp
std::string current_time_string()
{
    auto now =
        std::chrono::system_clock::now();

    auto t =
        std::chrono::system_clock::to_time_t(now);

    std::tm tm{};

    localtime_s(&tm, &t);

    std::ostringstream oss;

    oss << std::put_time(
        &tm,
        "%Y-%m-%d %H:%M:%S");

    return oss.str();
}
```

输出：

```text
2026-06-01 10:30:45
```

---

# 十一、localtime 警告问题

遇到：

```text
'localtime' is deprecated
```

---

## 原因

```cpp
std::localtime()
```

返回：

```cpp
static tm*
```

属于：

```text
线程不安全
```

API。

---

## Windows 推荐

```cpp
localtime_s(&tm, &t);
```

---

## Linux 推荐

```cpp
localtime_r(&t, &tm);
```

---

## 跨平台封装

```cpp
std::tm safe_localtime(std::time_t t)
{
    std::tm tm{};

#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    return tm;
}
```

---

# 十二、本次研究核心结论

## JSON

```text
nlohmann/json
=
宏 + 模板 + ADL
```

不是真反射。

---

## 嵌套结构体

```text
子结构体必须先注册
```

否则：

```text
No viable overloaded '='
```

---

## C++反射

目前：

```text
没有标准反射
```

所以：

```cpp
name
age
addr
```

必须注册。

---

## Boost.Hana

属于：

```text
高级元编程
伪反射
```

比 nlohmann/json 更接近真正反射。

---

## Chrono

工程实践：

```text
system_clock
    ↓
打印时间

high_resolution_clock
    ↓
统计耗时
```

不要混用。

---

## 时间格式化

推荐：

```cpp
localtime_s
+
std::put_time
```

而不是：

```cpp
asctime
gmtime
```

这样的老式接口。
