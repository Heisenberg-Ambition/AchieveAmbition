# TypeScript 相关

## 1. => 和 : 的区别

在 TypeScript/JavaScript 里，=> 和 : 用在完全不同的语境里：

### 1. => 箭头函数
用来定义函数的实现。

**语法：** (参数) => { 函数体 }

**例子：**
```typescript
const add = (a: number, b: number): number => {
  return a + b;
};
```

这里的 => 表示"函数体的开始"，相当于传统的 function 关键字。

### 2. : 类型注解
用来声明变量、参数或函数的类型。

**语法：** 变量名: 类型

**例子：**
```typescript
const add: (a: number, b: number) => number = (a, b) => a + b;
```

这里的 : 表示"这个变量的类型是一个函数，接受两个 number，返回一个 number"。

### 3. 两者结合
常见的写法是：用 : 声明函数返回类型，用 => 定义函数体。

```typescript
// 函数声明
function foo(a: number): Promise<string> {
  return Promise.resolve("ok");
}

// 箭头函数写法
const foo = (a: number): Promise<string> => {
  return Promise.resolve("ok");
};
```

### 总结
- : → 类型注解，告诉编译器"这个东西是什么类型"
- => → 箭头函数，定义函数的实现
- 两者经常一起出现：const fn = (x: number): string => "hello"

## 2. TypeScript 导入时加 {} 和不加 {} 的区别

### 不加 {} → 导入默认导出 (export default)
```typescript
import MyComponent from './MyComponent'
```

### 加 {} → 导入具名导出 (export)
```typescript
import { MyComponent } from './MyComponent'
```

### 可以同时导入默认和具名
```typescript
import MyComponent, { helper } from './MyComponent'
```

### 总结
- 不加 {}：你拿的是文件里唯一的"默认出口"
- 加 {}：你拿的是文件里明确命名的"多个出口"之一

## 3. TypeScript 运算符详解

### 1. .(点运算符)
**作用：** 最基本的属性访问运算符。

**用法：** 直接访问对象的属性或方法。

### 2. ?.(可选链 Optional Chaining)
**作用：** 在访问对象属性或方法时，先检查前面的对象是否为 null 或 undefined。如果是，则返回 undefined，而不是抛出错误。

**用法：**
```typescript
const obj: any = null;
console.log(obj?.name); // 输出 undefined，而不是报错
```

**常见场景：** 深层嵌套对象访问。

### 3. ??(空值合并 Nullish Coalescing)
**作用：** 当左边的值是 null 或 undefined 时，返回右边的值；否则返回左边的值。

**区别于 ||：** || 会把假值（如 0、false、""）也当成需要替换的情况，而 ?? 只针对 null 和 undefined。

**用法：**
```typescript
const a = null ?? "default"; // "default"
const b = undefined ?? "default"; // "default"
const c = 0 ?? 42; // 0（不会替换）
const d = "" ?? "hello"; // ""（不会替换）
```

### 4. === 严格相等 (Strict equality)
比较时类型和值都必须相等。

### 5. == 宽松相等 (Loose equality)
比较时会进行类型转换。容易出现一些"奇怪"的结果：
```javascript
console.log(1 == "1"); // true（字符串"1"会被转换成数字1）
console.log(0 == false); // true
console.log(null == undefined); // true
```

### 6. ! 非空断言和确定赋值断言

#### 1. 非空断言
```typescript
console.log(value!.length);
```
告诉编译器 "我保证这里不是 null 或 undefined"。编译器就不会报错，但如果运行时真的为 null 或 undefined，还是会抛异常。

**常见场景：** 当你非常确定某个值一定存在，但类型上是可空的。

#### 2. 确定赋值断言
```typescript
let name!: string;
```
告诉编译器 "这个变量我一定会在使用前赋值"。编译器就不会报 "变量可能未初始化" 的错误。

**常见场景：** 类的属性在构造函数或某个初始化方法里赋值，但不是在声明时赋值。

**注意：** ! 只是编译器层面的保证，不会在运行时做检查。滥用可能导致隐藏 bug，因为运行时如果真的为空，还是会报错。更安全的方式是用可选链 ?. 或空值合并 ?? 来处理。

### 总结对比表

| 符号 | 名称 | 作用 | 示例 |
|------|------|------|------|
| . | 点运算符 | 普通属性访问 | obj.name |
| ?. | 可选链 | 遇到 null/undefined 时返回 undefined 而不是报错 | obj?.name |
| ?? | 空值合并 | 左边是 null/undefined 时用右边的值 | value ?? "default" |

### TypeScript/JavaScript 运算符对比表

| 运算符 | 名称 | 用途 | 示例 | 结果 |
|--------|------|------|------|------|
| ?. | 可选链 (Optional chaining) | 安全访问对象属性，避免 null/undefined 报错 | obj?.a?.b | 如果 obj.a 存在→返回 b；否则→undefined |
| ?? | 空值合并 (Nullish coalescing) | 给变量设置默认值，仅在 null/undefined 时生效 | null ?? "default" | "default" |
| \|\| | 逻辑或 (Logical OR) | 给变量设置默认值，在"假值"（包括 0、false、""）时生效 | 0 \|\| 42 | 42 |
| === | 严格相等 (Strict equality) | 比较值和类型是否完全相等 | 1 === "1" | false |
| == | 宽松相等 (Loose equality) | 比较时会自动类型转换 | 1 == "1" | true |

## 4. Record<string, any> 类型

Record<string, any> 是 TypeScript 提供的一个工具类型 (Utility Type)，用来快速定义键值对对象。

### 定义
```typescript
type Record<K extends keyof any, T> = {
  [P in K]: T;
};
```

Record<K, T> 表示一个对象类型：
- 键（Key）类型是 K
- 值（Value）类型是 T

### 举例说明

#### 1. Record<string, any>
```typescript
const obj: Record<string, any> = {
  name: "Ori",
  age: 25,
  active: true,
};
```
- 键是 string 类型（比如 "name", "age"）
- 值可以是任意类型（any）
- 相当于一个任意键值对对象

## 5. tsconfig.json 里的 ESNext 和 Node

### module 的常见取值

#### CommonJS
- 输出 require() 和 module.exports
- 传统 Node.js 模块系统
- 不支持 tree-shaking，和纯 ESM 包（如 lodash-es）不兼容

#### ESNext
- 输出标准的 import/export
- 最适合现代前端（React + Webpack/Rollup）
- 不依赖 package.json 的 "type" 字段，比较宽松

#### NodeNext
- 模拟 Node.js 的最新 ESM 行为
- 编译器会根据 package.json 的 "type" 来决定某个文件是 ESM 还是 CJS
- 要求和 moduleResolution: "NodeNext" 搭配使用
- 更严格，适合纯 Node.js 项目，但在 Webpack 前端项目里容易冲突

### moduleResolution 的常见取值

#### Classic
- 老版本的解析方式，几乎不用了

#### Node
- 模拟 Node.js 的 require 解析规则
- 默认适合大多数前端项目
- 和 module: "ESNext" 搭配最常见

#### NodeNext
- 新增的解析模式，支持 Node.js 的 ESM 规则
- 会区分 .js/.cjs/.mjs，并依赖 package.json 的 "type"
- 如果项目不是纯 Node.js 环境，容易导致 TSX 报错

### CommonJS vs ESM 对比

| 特性 | CommonJS (CJS) | ESM (ECMAScript Modules) |
|------|----------------|--------------------------|
| 导入语法 | const x = require("x") | import x from "x" |
| 导出语法 | module.exports = ... | export default ... / export { ... } |
| 加载方式 | 动态（运行时） | 静态（编译时） |
| Tree-shaking 支持 | ❌ 不支持 | ✅ 支持 |
| 适用场景 | Node.js 老项目 | 前端、现代 Node.js |

### 总结
ESM 就是 JavaScript 官方的模块化标准，用 import/export 来组织代码。它比 CommonJS 更现代，更高效，尤其适合前端项目，因为打包工具能自动优化未使用的代码。
