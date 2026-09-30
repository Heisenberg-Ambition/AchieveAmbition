# From Effective C++ . More Effective C++ And Effective Modern C++

# Skill of Rule for C++ Code

###

## 1. Modern C++

### 1. 减少new/delete的使用, 优先考虑unique_ptr/shared_ptr/weak_ptr智能指针

### 2. 优先考虑使用std::make_unique和std::make_shared而非new

### 3. 先考虑栈对象（值语义）-> 再考虑 std::unique_ptr -> 最后才考虑 std::shared_ptr

### 4. 优先考虑C++风格的类型转换, 尽量不要进行C语言形式的强制类型转换. 如果必须进行, 则先考虑使用static_cast<>, (dynamic_cast<>、reinterpret_cast<>、const_cast<>减少使用)

### 5. 使用++i(循环语句/一般情况) 和 i = i + n

### 6. 减少C语言的方式和头文件, 优先考虑C++风格的类型/接口: std::string/vector/map/set/algorithm/numeric/iterator/functional/exception/chrono/regex.

### 7. 当使用临时组合的数据时(数据间没有关联关系), 使用std::pair<T1, T2>, 否则考虑定义struct

### 8. 使用enum class而不是enum, 因为enum class可以防止隐式转换, 而enum不能防止

### 9. 不要使用 typedef, 统一使用using alias声明, typedef属于C语言, 而using属于C++, using可以使用模板参数, typedef不能.

### 10. 减少include全局头文件, 优先考虑导入你所需的头文件, 减少使用forward declaration(前向声明).

### 11. 不要在头文件里写实现, 除了inline/template.

### 12. 能auto就使用auto推导类型, 但是要确定是否为&/&&.

## 资源拷贝和移动

### 1. 尝试在循环内定义变量, 离开循环前std::move(), 而不是在循环开始前定义, 在循环内赋值.

### 2. 内置数字类型, 不要使用引用传参, 尽量避免基础类型不初始化.

### 3. 对于内建类型(数值/指针/char\*)及 STL 中的迭代器和函数对象类型传值通常更合适.

### 4. 不要滥用引用成员, 指针有时候更合适, 比如在函数返回时使用智能指针.

### 5. std::move() 不移动对象，它只是将表达式转换为右值引用, std::forward() 不转发对象，它只是根据模板参数决定保持左值还是右值。真正的移动或转发发生在后续函数调用中.

## RAII相关

### 1. 使用{}初始化, 可以防止窄化转换和确保initializer_list<T>优先, 使用std::string name {} 初始化, 数值类型初始化使用 = 0.

### 2. 类里使用 {} 初始化和构造函数列表初始化(std::move()), 避免使用 this.a = a 初始化

### 3. 不要return std::move(w), 打断ROV.

### 4. 能放栈上，就不要放堆上, 减少Person\* person = new Person()这样的代码

### 5. 小的局部对象, 栈上 > 堆上.

### 5. 让对象拥有对象, 而不是对象拥有指针

### 6. std::shared_ptr不表示生命周期, 只表示共享所有权

## class 和 const

### 1. const/const & 能加就加

### 2 .尽量在编译器完成不用推到运行期, 尽量constexpr/consteval(必须在编译期确定)/constinit(初始化时机的约束)

### 3. 为多态基类声明virtual析构函数

### 4. 成员变量几乎都应该是private

### 5. 绝不在构造和析构过程中调用virtual函数

### 6. 一定标注 override

### 7. 若不想使用编译器自动生成的函数，就该明确拒绝

### 8. 组合优于继承, 减少多层继承, 减少菱形继承(非要的话, 使用虚继承), 非要拉高性能可以考虑CRTP(奇异递归模板模式)

### 9. 将All成员变量声明为private, 可以提供get, 但不一定提供set

### 10. 暴露行为(Behavior)，而不是暴露数据(Data)

### 11. Rule of Zero, 如果不写特殊成员函数, 就不要写特殊成员函数, 如果写了, 就要写全

### 12. 假设声明/定义了析构函数, 要手动实现移动构造函数和移动赋值运算符

### 13. 析构函数应该 noexcept

### 14. 为成员函数添加const/override/final(禁止继续override)/=0/volatile(很少用)/&(仅限左值对象)/&&(仅限右值对象)/noexcept(承诺不抛异常否则直接std::terminate())

### 15. 使用=delete而非未定义的private声明

### 16. 绝不重新定义继承而来的non-virtual函数

### 17. 明智而审慎地使用多重继承

### 18. 优先考虑使用delete来禁用函数而不是声明成private却又不实现

### 19. 确定你的public继承塑模出is-a关系

### 20. 在命名空间里使用non-member. non-friend替换member函数

### 21. 右值可以绑定到 const 左值引用, 此时MC(std::move(mc))会调用拷贝构造函数.

### 22. Name Hiding(名字隐藏) 只看函数名 派生类同名函数隐藏基类所有同名函数

### 23. Overload(重载) 同一作用域中函数名相同、参数不同(包括 const、&、&& 等) 编译器根据参数选择调用哪个函数

### 24. Override(覆盖) 基类虚函数 + 完整签名匹配(包括 const、&、&& 等) 运行时多态，替换基类虚函数实现

### 25. 对于默认构造、拷贝构造、移动构造，不需要使用 explicit；对于可能发生隐式转换的构造函数（尤其是单参数构造函数），原则上都应考虑加上 explicit.

## 并发相关

### 1. 不要写std::thread/裸线程, 而是使用std::async/std::future, 如果非要使用std::thread, 改为std::jthread

### 2. 使用atomic/lock_guard

### 3. 从各个方面使得std::threads unjoinable

### 4. 在线程间一次性结果传递时，优先使用 std::promise<T>(不需要std::mutex) / std::future<T>，而不是使用全局变量, 共享状态配合轮询

### 5. 在线程间多次传递数据时，可以使用std::queue<T> 和 std::condition_variable, 构建生产者-消费者模型.

## Lambda STL

### 1. 不要要避免返回裸指针, 而是unique_ptr<T>/shared_ptr<T>

### 2. 使用Lambda而不是std::bind, lambda > std::bind

### 3. 优先考虑const_iterator而非iterator

### 4. 使用emplace_back() 而不是 push_back(), 考虑就地创建而非插入

### 5. 使用迭代器来遍历容器, 而不是索引

### 6. Lambda表达式明确捕获模式/捕获的对象, 避免使用默认捕获模式

### 7. 对于通用引用使用std::forward<T>

### Other

### 1. Don't premature optimization, 不要提前优化.

### 2. 提早返回, 短链Retrun, Early Return.

### 3. 前置条件检查用 Early Return（不带 else），对等的二选一逻辑用对称 if/else.

### 4. 当存在错误逻辑时: if(!open()) return; 明确不需要else{}直接开始成功的逻辑, 减少{}嵌套.

### 5. 当是两者均为正常逻辑时, if/else需要对称使用抱持可读性: if(isDaytime) {useLightTheme();} else {useDarkTheme();}

### 6. "当条件块以 return、break、continue 或 throw 结尾时，不要在其后跟上 else 语句。"

### 7. 一般情况不写this->, 除非出现名称遮蔽问题

### 8. 使用C++20的[this] 捕获指针，[*this] 拷贝捕获对象

### 9. switch 表达"是什么"，if/else 表达"满足什么条件", 大量判断时使用switch, 少量判断时使用if/else. 或者尽可能使用static constexpr std::map<C, F>.

### 10. 对短小且影响性能的函数使用内联展开
