# React 渲染机制详解

## 1. React 渲染基本概念

### 渲染的定义
渲染指的是：函数组件执行 + 虚拟DOM → 真实DOM更新

### 完整渲染流程图

```mermaid
graph TD
    A[state/props 改变] --> B[调度更新 schedule]
    B --> C[函数组件执行 render phase]
    C --> D[执行函数组件]
    D --> E[生成 Fiber Tree]
    E --> F[Diff]
    F --> G[计算变更]
    G --> H[生成新的 Virtual DOM Fiber Tree]
    H --> I[Diff/Reconciliation]
    I --> J[计算出 DOM 变更]
    J --> K[Commit Phase 提交阶段]
    K --> L[更新真实 DOM]
    L --> M[更新 ref]
    M --> N[useLayoutEffect cleanup]
    N --> O[useLayoutEffect]
    O --> P[真实 DOM 更新]
    P --> Q[useLayoutEffect 执行 同步]
    Q --> R[浏览器 Paint 绘制]
    R --> S[useEffect 执行 异步]
```

### 重要说明
- render 阶段不会操作真实 DOM，真正操作 DOM 的是：commit phase
- 任何地方调用 setState(useState定义的) 都会触发新一轮渲染
- render = 函数组件重新执行
- React 的渲染机制是：只要你调用了 setState，React 就会把这个组件标记为"需要更新"，然后在下一次调度时重新执行函数组件，生成新的虚拟 DOM，再对比并更新真实 DOM

## 2. useEffect vs useLayoutEffect

### 都会在组件第一次挂载时调用吗？
是的，两者在组件第一次挂载时都会调用，但它们的执行时机不同。

### 执行时机对比表

| 生命周期阶段 | useLayoutEffect | useEffect |
|-------------|----------------|-----------|
| 初次挂载 | 在 DOM 更新后，浏览器绘制前同步执行 | 在浏览器完成绘制后异步执行 |
| 依赖变化 | 先执行上一次清理函数 → 再执行新的回调(绘制前) | 先执行上一次清理函数 → 再执行新的回调(绘制后) |
| 阻塞绘制 | 会阻塞浏览器绘制 | 不会阻塞浏览器的绘制 |
| 卸载 | 执行清理函数 | 执行清理函数 |

### 主要区别对比表

| 特性 | useEffect | useLayoutEffect |
|------|-----------|-----------------|
| 执行时机 | 在浏览器完成渲染并绘制到屏幕之后异步执行 | 在 DOM 更新后，浏览器绘制前同步执行 |
| 阻塞绘制 | 不会阻塞浏览器绘制，适合大多数副作用逻辑 | 会阻塞绘制，保证在用户看到页面前完成 DOM 操作 |

### React 渲染流程时间线

```mermaid
graph TD
    A[React 开始渲染] --> B[更新虚拟 DOM]
    B --> C[更新真实 DOM]
    C --> D[执行 useLayoutEffect 回调]
    D --> E[浏览器绘制到屏幕]
    E --> F[执行 useEffect 回调]

    D --> D1[同步 阻塞绘制]
    D1 --> D2[适合 订阅消息 测量 DOM 同步修改样式]

    F --> F1[异步 不阻塞绘制]
    F1 --> F2[适合 数据请求 日志 事件绑定]
```

## 3. 何时使用 useLayoutEffect

### 必须使用 useLayoutEffect 的场景

```mermaid
graph TD
    A[需要使用 useLayoutEffect] --> B[订阅消息影响UI]
    A --> C[绘制前读取DOM尺寸位置]
    A --> D[同步修改DOM样式或滚动条]
    A --> E[动画精确控制初始状态]
    A --> F[副作用会影响UI]

    B --> B1[避免消息丢失]
    C --> C1[避免闪烁]
    D --> D1[保证一致性]
    E --> E1[避免抖动]
    F --> F1[保证正确渲染]
```

### 为什么在订阅 ROS1 消息时常用 useLayoutEffect

#### 需要同步建立订阅/广播
ROS 的 watch、advertise 本质上是和底层通信中间件建立连接。如果放在 useEffect，它会在绘制之后才执行，可能导致组件已经渲染出来，但订阅还没建立，出现"短暂空窗期"。

#### 避免竞态条件
有些场景下，组件渲染后马上就可能触发消息发布。如果订阅还没建立（因为 useEffect 是异步的），就会丢掉第一批消息。useLayoutEffect 保证在绘制前就完成订阅，避免消息丢失。

#### 和 DOM 无关，但和"同步初始化"有关
虽然这里不是直接操作 DOM，但 React 的生命周期钩子只有这两个选择。既然需要"同步初始化"，就只能用 useLayoutEffect。

### useLayoutEffect 和 useEffect 总结

- **useEffect**：适合异步副作用(请求、日志、事件绑定)，不影响首屏渲染
- **useLayoutEffect**：适合必须在绘制前完成的初始化逻辑
- 订阅 ROS1 消息属于"必须立即建立连接"的场景，所以一般写在 useLayoutEffect 里，避免丢消息或出现初始化延迟

## 4. 虚拟DOM机制

### 什么是虚拟DOM
虚拟 DOM(Virtual DOM)是对真实 DOM 的一种轻量级 JavaScript 对象表示。它描述了页面结构(标签、属性、子节点等)，框架(如 React,Vue)通过比较新旧虚拟 DOM 的差异(Diff)，再有选择地更新真实 DOM，从而提升性能和开发体验。

### 什么是DOM
DOM(Document Object Model) 是浏览器对 HTML 的对象化表示。每个标签、属性、文本节点都会对应一个 DOM 节点。操作 DOM(如 document.getElementById,innerText)会触发浏览器的重排和重绘，代价较高。

### 为什么要用虚拟DOM

```mermaid
graph TD
    A[虚拟DOM优势] --> B[性能优化]
    A --> C[跨平台]
    A --> D[开发体验]

    B --> B1[减少频繁的真实DOM操作]
    B --> B2[先在虚拟DOM上计算差异]
    B --> B3[一次性更新真实DOM]

    C --> C1[不依赖浏览器API]
    C --> C2[可渲染到Web Native 小程序]

    D --> D1[声明UI长什么样]
    D --> D2[框架负责高效更新]
```

### DOM和虚拟DOM的工作流程

```mermaid
graph LR
    A[初始渲染] --> B[模板]
    B --> C[虚拟DOM]
    C --> D[真实DOM]

    E[状态更新] --> F[生成新的虚拟DOM]
    F --> G[Diff算法]
    G --> H[比较新旧虚拟DOM]
    H --> I[找出差异]
    I --> J[Patch]
    J --> K[只更新有变化的真实DOM节点]
```

### DOM和虚拟DOM总结
- 虚拟 DOM = 真实 DOM 的 JS 抽象
- 核心价值：通过 Diff 算法减少不必要的真实 DOM 操作，提升性能和开发效率
- 应用场景：React、Vue 等现代前端框架的核心机制
- 在 React 中，渲染应该是纯粹的计算 JSX，不应该包含任何像修改 DOM 这样的副作用
- 使用 useEffect 包裹副作用，把它分离到渲染逻辑的计算过程之外

## 5. React 中能引起 UI 更新的来源

### UI 更新来源表

| 来源 | 会不会 render | 会不会更新 UI |
|------|--------------|---------------|
| setState/useState | ✅ | 可能 |
| props 变化 | ✅ | 可能 |
| context 变化 | ✅ | 可能 |
| 父组件 render | ✅ | 可能 |
| forceUpdate | ✅ | 可能 |
| useReducer dispatch | ✅ | 可能 |
| 外部 store（Redux/Zustand） | ✅ | 可能 |

### 最终分类（非常重要）

#### 会触发 render
```mermaid
graph TD
    A[会触发 render] --> B[setState]
    A --> C[useReducer dispatch]
    A --> D[props 变化]
    A --> E[context 变化]
    A --> F[父组件 render]
    A --> G[Redux store 更新]
    A --> H[Zustand store 更新]
```

#### 不会触发 render
```mermaid
graph TD
    A[不会触发 render] --> B[useRef.current = xxx]
    A --> C[普通变量修改]
    A --> D[DOM 操作]
    A --> E[localStorage 修改]
```

## 6. React UI 更新本质

一句话总结：React 的 UI 更新本质是：组件重新 render 后，新的 Virtual DOM 与旧 DOM diff，最后决定是否更新真实 DOM。setState 只是触发 render 的一种方式。

### 性能问题分析

```mermaid
graph TD
    A[大量 useEffect] --> B[修改 DOM]
    B --> C[修改布局]
    C --> D[调用 setState]
    D --> E[导致重新 render]
    E --> F[导致 repaint/layout]

    F --> G[闪烁 flicker]
    F --> H[抖动 jank]
    F --> I[掉帧]
    F --> J[白屏瞬闪]
```

### 频繁 useEffect + setState 的危险

```mermaid
graph TD
    A[频繁 useEffect + setState] --> B[多次 paint]
    B --> C[layout thrashing]
    C --> D[UI 抖动]
```

## 7. useEffect 的依赖和清理

### useEffect 依赖数组的不同

#### 每次渲染后运行
```javascript
useEffect(() => {
    // 这里的代码会在每次渲染后运行
});
```

#### 只在组件挂载时运行
```javascript
useEffect(() => {
    // 这里的代码只会在组件挂载(首次出现)时运行
}, []);
```

#### 挂载时运行，依赖变化时也运行
```javascript
useEffect(() => {
    // 这里的代码不但会在组件挂载时运行，而且当 a 或 b 的值自上次渲染后发生变化后也会运行
}, [a, b]);
```

### useEffect 的返回值（清理函数）

#### 基本规则
useEffect 的回调函数可以返回一个清理函数(cleanup function)。这个清理函数不会在 useEffect 本身执行时立即运行，而是在特定时机触发。

#### 执行时机

```mermaid
graph TD
    A[useEffect 清理函数执行时机] --> B[组件卸载时]
    A --> C[依赖变化时]

    B --> B1[React 调用上一次清理函数]
    B --> B2[常用于 移除事件监听 清理定时器 取消订阅]

    C --> C1[React 先执行上一次清理函数]
    C --> C2[再执行新的 useEffect 回调]
```

#### 不会在初次渲染前执行
初次渲染时，useEffect 的回调会执行，但清理函数不会执行。清理函数只会在下一次 effect 运行前或组件卸载时运行。

### useEffect 的 return 执行顺序

```mermaid
graph TD
    A[组件挂载] --> B[执行 useEffect 回调]

    C[依赖变化] --> D[执行上一次清理函数]
    D --> E[执行新的 useEffect 回调]

    F[组件卸载] --> G[执行最后一次清理函数]
```

### 定时器的创建和清理

```javascript
useEffect(() => {
  const id = setInterval(() => {
    console.log("定时器运行中...");
  }, 1000);

  // 返回清理函数
  return () => {
    clearInterval(id);
    console.log("定时器已清理");
  };
}, []);
```

## 8. 经典死循环示例

这是新手最容易踩坑的：

```javascript
useEffect(() => {
  setCount(count + 1)
})
```

### 死循环流程图

```mermaid
graph TD
    A[useEffect 运行] --> B[更新 state]
    B --> C[触发重新渲染]
    C --> D[又触发 useEffect 运行]
    D --> E[再次更新 state]
    E --> F[继而再次触发重新渲染]
    F --> D
```

### 问题分析
useEffect 在渲染结束后运行。更新 state 会触发重新渲染。在 useEffect 中直接更新条件里的 useState 就像是把电源插座的插头插回自身：useEffect 运行，更新 state，触发重新渲染，于是又触发 useEffect 运行，再次更新 state，继而再次触发重新渲染。如此反复，从而陷入死循环。

默认情况下，useEffect 会在每次渲染后运行。

## 9. Concurrent Mode

### 为什么引入 Concurrent Mode
如果我们用「重编译时还是运行时」区分前端框架。那么Vue就是「重编译时」的杰出代表。而React由于使用JSX（而非模版语法）描述视图，走的是「重运行时」的路线。

### 「重编译时」的框架 —— Vue

```mermaid
graph TD
    A[Vue 重编译时] --> B[使用模板语法 template]
    B --> C[编译阶段转成高效渲染函数]
    C --> D[静态节点标记]
    D --> E[依赖收集]
    E --> F[编译时优化]
    F --> G[运行时轻量]

    G --> G1[上场之前就把战术排好]
    G --> G2[运行时只要执行既定策略]
```

### 「重运行时」的框架 —— React

```mermaid
graph TD
    A[React 重运行时] --> B[使用 JSX]
    B --> C[本质是 JavaScript 表达式]
    C --> D[编译阶段转成 createElement]
    D --> E[不做太多优化]
    E --> F[运行时遍历 Fiber 树]
    F --> G[计算差异]
    G --> H[调度更新]
    H --> I[运行时优化]

    I --> I1[比赛过程中不断调整战术]
    I --> I2[依赖运行时的灵活性]
```

### 为什么要引入 Concurrent Mode

```mermaid
graph TD
    A[React 运行时问题] --> B[状态更新到视图变化计算开销大]
    B --> C[没有调度机制]
    C --> D[所有更新同步执行]
    D --> E[容易造成卡顿]

    E --> F[Concurrent Mode 解决方案]
    F --> G[可中断渲染]
    F --> H[任务优先级调度]

    G --> G1[把渲染拆成小片段]
    G --> G2[遇到高优先级任务可暂停低优先级任务]

    H --> H1[用户输入事件优先于后台数据刷新]
    H --> H2[智能分配计算资源]
    H --> H3[提升交互流畅度]
```

### Vue vs React 的优化方式对比

| 特性/维度 | Vue（重编译时） | React（重运行时） |
|---------|----------------|------------------|
| 视图描述方式 | 模板语法 (<template>) | JSX（JavaScript 表达式） |
| 优化时机 | 编译阶段：在构建时分析模板，生成高效渲染函数 | 运行时：在渲染过程中遍历 Fiber 树，动态调度更新 |
| 典型优化手段 | 静态节点标记、依赖收集、预编译指令 | React.memo、PureComponent、shouldComponentUpdate 等运行时优化 |
| 渲染架构 | Virtual DOM + 编译时优化 | Fiber 架构（支持可中断渲染） |
| 性能瓶颈 | 编译时已优化，运行时压力较小 | 状态更新到视图变化之间的计算量大，容易卡顿 |
| 解决方案 | 编译器提前生成最优渲染逻辑 | 引入 Concurrent Mode：任务拆分 + 优先级调度 |
| 用户体验 | 流畅度依赖编译器优化结果 | 流畅度依赖运行时调度，能动态响应高优先级任务 |

### 总结
- Vue：靠编译器提前优化，运行时更轻量
- React：靠运行时调度（Fiber + Concurrent Mode）来保证流畅交互
- Concurrent Mode 的引入，就是为了让 React 在"重运行时"的路线下，也能像 Vue 一样流畅，但通过任务优先级 + 可中断渲染来实现

## 10. 浏览器渲染机制

### JS线程和UI线程为什么互斥

```mermaid
graph TD
    A[浏览器内部线程] --> B[JS 线程]
    A --> C[UI 渲染线程]

    B --> B1[负责执行 JavaScript 代码]
    C --> C1[负责布局 reflow]
    C --> C2[负责绘制 repaint]

    B1 --> D[可以直接操作 DOM]
    C1 --> E[互斥关系]
    C2 --> E

    E --> F[JS 修改 DOM 时 UI 线程必须暂停]
    E --> G[保证 DOM 操作和渲染的一致性]
```

### 当 JS 影响 UI 时会发生什么

```mermaid
graph TD
    A[JS 影响 UI] --> B[重排 Reflow]
    A --> C[重绘 Repaint]

    B --> B1[JS 改变 DOM 结构]
    B --> B2[JS 改变元素几何属性]
    B --> B3[浏览器重新计算布局]

    C --> C1[JS 改变元素样式]
    C --> C2[JS 改变颜色背景]
    C --> C3[浏览器重新绘制像素]

    B3 --> D[触发 UI 线程工作]
    C3 --> D
    D --> E[JS 线程必须让出执行权]
```

### 性能影响

```mermaid
graph TD
    A[JS 执行时间过长] --> B[UI 渲染线程挂起]
    B --> C[页面卡顿]

    D[频繁触发重排/重绘] --> E[循环里不断修改 DOM]
    E --> F[掉帧]

    C --> G[React 引入 Concurrent Mode]
    F --> G

    G --> H[任务拆分]
    G --> I[优先级调度]
    G --> J[避免长时间阻塞 UI]
```

### 总结
- JS 线程和 UI 线程互斥，是为了保证 DOM 操作和渲染的一致性
- 当 JS 修改 DOM 或样式时，会触发重排（Reflow）和重绘（Repaint）
- 如果 JS 执行过久或频繁触发这些操作，就会造成页面卡顿
- React 引入 Concurrent Mode 通过任务拆分和优先级调度，避免长时间阻塞 UI
