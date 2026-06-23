# 本次对话总结（C++ / nlohmann::json / Protobuf / Foxglove / MCAP）

## 1. C++ 获取字符串后缀

需求：

```cpp
dafda
dafdazxc
```

希望得到：

```cpp
zxc
```

讨论了通过判断前缀、截取后缀：

```cpp
std::string prefix = "dafda";
std::string str = "dafdazxc";

if (str.starts_with(prefix))
{
    std::string suffix = str.substr(prefix.size());
}
```

C++17：

```cpp
if (str.rfind(prefix, 0) == 0)
{
    std::string suffix = str.substr(prefix.size());
}
```

---

## 2. 读取 TXT 文件，每行一个 JSON

需求：

```text
{"id":1}
{"id":2}
...
```

读取到：

```cpp
std::vector<nlohmann::json>
```

实现：

```cpp
std::ifstream ifs("data.txt");

std::vector<nlohmann::json> data;

std::string line;

while (std::getline(ifs, line))
{
    if (line.empty())
    {
        continue;
    }

    data.push_back(
        nlohmann::json::parse(line));
}
```

### push_back 与 emplace_back

讨论：

```cpp
data.push_back(json::parse(line));
```

是否会发生额外拷贝。

结论：

```cpp
json::parse(line)
```

返回临时对象。

因此：

```cpp
push_back(temp)
```

会调用移动构造。

实际等价于：

```cpp
push_back(std::move(temp))
```

不会发生额外深拷贝。

因此：

```cpp
push_back(json::parse(line));
```

与

```cpp
emplace_back(json::parse(line));
```

收益几乎一致。

---

## 3. 读取整个 JSON 文件

读取：

```json
{
    "name": "test"
}
```

到：

```cpp
nlohmann::json
```

实现：

```cpp
std::ifstream ifs("test.json");

nlohmann::json j;

ifs >> j;
```

或者：

```cpp
std::ifstream ifs("test.json");

std::string content(
    std::istreambuf_iterator<char>(ifs),
    std::istreambuf_iterator<char>());

auto j = nlohmann::json::parse(content);
```

---

## 4. 空 JSON 处理

文件中可能存在：

```json
{}
```

或者：

```json
[]
```

需求：

统一视为空对象。

实现：

```cpp
json j = json::parse(line);

if (j.is_array() && j.empty())
{
    j = json::object();
}
```

---

## 5. Foxglove Proto Schema 注册

涉及：

- Point3
- LinePrimitive
- TextPrimitive
- SceneEntity
- SceneUpdate

注册流程：

```cpp
Schema
    ↓
addSchema()
    ↓
Channel
    ↓
addChannel()
```

核心：

```cpp
DescriptorPool
    ↓
Descriptor
    ↓
FileDescriptorSet
    ↓
SerializeToString()
```

最终生成：

```cpp
m_schema_.data
```

作为 MCAP Schema Data。

---

## 6. MCAP Schema 构造流程

代码：

```cpp
auto descriptor =
    DescriptorPool::generated_pool()
        ->FindMessageTypeByName(schema_name);
```

得到：

```cpp
Descriptor*
```

然后：

```cpp
descriptor->file()
```

得到：

```cpp
FileDescriptor*
```

再构造：

```cpp
FileDescriptorSet
```

最终：

```cpp
fds.SerializeToString(&descriptorBytes);
```

保存到：

```cpp
Schema.data
```

---

## 7. addFileDescriptor 的问题

原实现：

```cpp
void addFileDescriptor(
    const FileDescriptor* file,
    FileDescriptorSet& fds)
{
    file->CopyTo(fds.add_file());

    for (...)
    {
        addFileDescriptor(...);
    }
}
```

问题：

依赖文件可能重复加入。

例如：

```text
SceneUpdate
 ├─ SceneEntity
 │   └─ Color
 ├─ TextPrimitive
 │   └─ Color
 └─ LinePrimitive
     └─ Color
```

最终：

```text
Color.proto
Color.proto
Color.proto
```

Foxglove 可能报：

```text
duplicate symbol
duplicate type
```

---

## 8. 去重实现

增加：

```cpp
std::unordered_set<std::string> visited;
```

实现：

```cpp
if (!visited.insert(
        std::string(file->name())).second)
{
    return;
}
```

作用：

同一个 proto 文件只加入一次。

---

## 9. FileDescriptor 详解

类型：

```cpp
google::protobuf::FileDescriptor
```

不是文件内容。

而是：

```text
.proto 文件的运行时元信息
```

例如：

```proto
SceneUpdate.proto
```

对应：

```cpp
FileDescriptor*
```

可以获得：

```cpp
file->name()
```

```cpp
file->dependency_count()
```

```cpp
file->dependency(i)
```

等。

---

## 10. file->name()

新版 protobuf：

```cpp
absl::string_view
FileDescriptor::name() const;
```

返回：

```text
foxglove/SceneUpdate.proto
```

而不是：

```cpp
std::string
```

因此：

```cpp
visited.insert(file->name());
```

可能出现类型匹配问题。

解决：

```cpp
visited.insert(
    std::string(file->name()));
```

---

## 11. insert(...).second 是什么

对于：

```cpp
std::unordered_set<std::string> visited;
```

执行：

```cpp
auto result =
    visited.insert("Color.proto");
```

返回：

```cpp
std::pair<iterator, bool>
```

其中：

```cpp
result.first
```

表示迭代器。

```cpp
result.second
```

表示是否插入成功。

### 第一次

```cpp
visited.insert("Color.proto");
```

返回：

```cpp
.second == true
```

### 第二次

```cpp
visited.insert("Color.proto");
```

返回：

```cpp
.second == false
```

因此：

```cpp
if (!result.second)
{
    return;
}
```

表示：

```text
已经处理过该 proto 文件
```

---

## 12. FileDescriptorSet 构造顺序

推荐：

```cpp
for (...)
{
    addFileDescriptorImpl(...);
}

file->CopyTo(...);
```

即：

```text
Dependency
    ↓
Owner
```

顺序。

例如：

```text
Color.proto
Pose.proto
SceneEntity.proto
SceneUpdate.proto
```

更符合 protobuf 依赖关系。

---

## 13. nlohmann::json 异常

出现：

```text
[json.exception.type_error.305]
cannot use operator[] with a string argument with array
```

含义：

当前 JSON 实际是：

```json
[]
```

或者：

```json
[
    {}
]
```

但代码写了：

```cpp
j["points"]
```

而：

```cpp
operator[](string)
```

只能用于：

```json
{}
```

即 Object。

不能用于 Array。

---

## 14. 如何定位 JSON 类型问题

打印：

```cpp
LOG_INFO(
    "type = {}",
    j.type_name());
```

可能输出：

```text
object
array
string
number
boolean
null
```

或者：

```cpp
LOG_INFO("{}", j.dump());
```

查看原始 JSON。

---

## 15. 稳定访问 JSON

### 不推荐

```cpp
j["id"]
```

原因：

- key 不存在异常
- 类型错误异常
- array 上访问异常

---

### 推荐 1

```cpp
if (j.is_object() &&
    j.contains("id"))
{
    auto id =
        j.at("id").get<int>();
}
```

---

### 推荐 2

```cpp
int id =
    j.value("id", 0);
```

```cpp
std::string name =
    j.value("name", "");
```

优点：

key 不存在时返回默认值。

---

### 注意

```json
{
    "id": "abc"
}
```

执行：

```cpp
j.value("id", 0);
```

仍会抛异常。

因为：

```text
类型不匹配
```

---

### 最稳方案

```cpp
auto it = j.find("id");

if (it != j.end() &&
    it->is_number_integer())
{
    int id =
        it->get<int>();
}
```

或者封装：

```cpp
GetInt(...)
GetString(...)
GetArray(...)
```

统一检查：

- key 是否存在
- 类型是否正确

避免项目中到处出现 JSON 异常。

---

# 最终结论

本次主要解决了三个方向的问题：

1. C++ 字符串与 JSON 文件读取。
2. nlohmann::json 的安全访问与异常分析。
3. Foxglove / MCAP / Protobuf Descriptor 的 Schema 构造、依赖递归与去重。

尤其明确了：

- FileDescriptor 是 proto 文件元信息。
- file->name() 返回 absl::string_view。
- unordered_set::insert().second 用于判断是否首次插入。
- addFileDescriptor 必须去重，否则会产生重复 Descriptor。
- JSON 访问尽量避免直接使用 operator[]。
- 推荐使用 contains / at / value / find + 类型检查。
