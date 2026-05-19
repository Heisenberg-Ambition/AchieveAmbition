# React 状态管理

## 1. React 单向数据流

### 为什么 React 选择单向数据流

```mermaid
graph TD
    A[React 单向数据流优势] --> B[数据流向清晰 易于理解和调试]
    A --> C[性能更好]
    A --> D[逻辑可控 减少错误]
    A --> E[更容易做服务端渲染 SSR]

    B --> B1[数据总是自顶向下流动]
    B --> B2[父组件 → 子组件]
    B --> B3[追踪谁把数据传给了谁]
    B --> B4[调试更直观]

    C --> C1[单向数据流结合虚拟DOM diff]
    C --> C2[只更新必要的部分]
    C --> C3[避免不必要的渲染]

    D --> D1[数据修改只能通过 setState/useState]
    D --> D2[或父组件传递的回调]
    D --> D3[数据来源和修改点可控]
    D --> D4[减少状态混乱和副作用]

    E --> E1[数据变化可预测]
    E --> E2[便于在服务端生成一致的 UI]
```

### 单向数据流工作流程

```mermaid
graph LR
    A[父组件] -->|传递 props| B[子组件]
    B -->|不能直接修改父组件数据| C[通过回调函数更新]
    C -->|调用父组件传递的函数| A

    style A fill:#e1f5ff
    style B fill:#fff4e1
    style C fill:#e8f5e9
```

## 2. React 单向数据流 vs Vue 双向绑定

### 对比表

| 对比维度 | React（单向数据流） | Vue（双向绑定） |
|---------|-------------------|----------------|
| 数据流向 | 数据只能从父组件 → 子组件(props)，子组件不能直接修改父组件数据 | 数据可以在视图和数据模型之间双向流动(v-model) |
| 状态更新 | 必须通过 setState / useState / 状态管理库来更新，触发重新渲染 | 模型数据变化会自动更新视图，视图输入也会自动更新模型 |
| 可控性 | 数据修改点集中，逻辑清晰，可预测性强 | 修改来源可能分散，调试时需要追踪数据的双向流动 |
| 性能 | 结合虚拟 DOM diff，减少不必要的更新，性能更稳定 | 双向绑定需要依赖响应式系统(getter/setter 或 Proxy)，小项目方便，大项目需注意性能优化 |
| 调试难度 | 单向数据流，数据来源明确，调试相对简单 | 双向绑定可能导致"谁改了数据"不清晰，调试复杂度更高 |
| 适用场景 | 大型复杂应用，状态管理严格，可预测性要求高 | 中小型项目，表单交互频繁，开发效率优先 |
| 典型写法 | `<Child count={count} onChange={setCount} />` | `<input v-model="message" />` |

### 数据流对比图

```mermaid
graph TD
    subgraph React 单向数据流
        A1[父组件] -->|props| B1[子组件]
        B1 -->|onChange| C1[父组件更新]
        C1 -->|新 props| B1
    end

    subgraph Vue 双向绑定
        A2[数据模型] <-->|v-model| B2[视图]
        B2 -->|输入变化| A2
        A2 -->|数据变化| B2
    end

    style A1 fill:#e1f5ff
    style B1 fill:#fff4e1
    style C1 fill:#e8f5e9
    style A2 fill:#fce4ec
    style B2 fill:#f3e5f5
```

## 3. React 状态管理方案

### 状态管理层次

```mermaid
graph TD
    A[React 状态管理] --> B[组件内部状态]
    A --> C[状态提升]
    A --> D[Context API]
    A --> E[第三方状态管理库]

    B --> B1[useState]
    B --> B2[useReducer]

    C --> C1[父组件管理状态]
    C --> C2[通过 props 传递]
    C --> C3[通过回调更新]

    D --> D1[useContext]
    D --> D2[跨组件共享状态]

    E --> E1[Redux]
    E --> E2[Zustand]
    E --> E3[Recoil]
    E --> E4[Jotai]
```

### 不同状态管理方案对比

| 方案 | 适用场景 | 优点 | 缺点 |
|------|---------|------|------|
| useState | 组件内部简单状态 | 简单直接，React 内置 | 不适合跨组件共享 |
| useReducer | 复杂状态逻辑 | 状态逻辑集中，易于测试 | 代码量相对较多 |
| Context API | 跨组件共享状态 | 无需额外依赖，React 内置 | 性能问题，容易导致不必要的渲染 |
| Redux | 大型应用复杂状态 | 生态完善，中间件丰富，调试工具强大 | 样板代码多，学习曲线陡峭 |
| Zustand | 中小型项目 | 简单轻量，API 简洁，无 Provider 包裹 | 生态相对较小 |
| Recoil | 原子化状态管理 | 细粒度更新，与 React 紧密集成 | 相对较新，生态不够成熟 |

## 4. Redux 状态管理

### Redux 核心概念

```mermaid
graph TD
    A[Redux 核心概念] --> B[Action]
    A --> C[Reducer]
    A --> D[Store]
    A --> E[Middleware]

    B --> B1[描述发生了什么]
    B --> B2[纯对象 type 字段必需]

    C --> C1[根据 action 更新状态]
    C --> C2[纯函数 不修改原状态]

    D --> D1[保存应用状态]
    D --> D2[dispatch action]
    D --> D3[subscribe 监听变化]

    E --> E1[处理副作用]
    E --> E2[日志 异步请求]
```

### Redux 数据流

```mermaid
graph LR
    A[UI Component] -->|dispatch action| B[Store]
    B -->|action| C[Reducers]
    C -->|new state| B
    B -->|subscribe| A

    D[Middleware] -->|intercept| B
    D -->|async actions| E[API Calls]
    E --> D

    style A fill:#e1f5ff
    style B fill:#fff4e1
    style C fill:#e8f5e9
    style D fill:#fce4ec
    style E fill:#f3e5f5
```

## 5. Zustand 状态管理

### Zustand 特点

```mermaid
graph TD
    A[Zustand 特点] --> B[简单轻量]
    A --> C[无 Provider 包裹]
    A --> D[支持中间件]
    A --> E[TypeScript 友好]

    B --> B1[API 简洁]
    B --> B2[学习成本低]

    C --> C1[直接使用 store]
    C --> C2[无需 Context Provider]

    D --> D1[devtools]
    D --> D2[persist 中间件]
    D --> D3[immer 中间件]

    E --> E1[完整类型推导]
    E --> E2[开发体验好]
```

### Zustand 使用示例

```javascript
import create from 'zustand';

// 创建 store
const useStore = create((set) => ({
  count: 0,
  increment: () => set((state) => ({ count: state.count + 1 })),
  decrement: () => set((state) => ({ count: state.count - 1 })),
}));

// 在组件中使用
function Counter() {
  const { count, increment, decrement } = useStore();

  return (
    <div>
      <p>Count: {count}</p>
      <button onClick={increment}>+</button>
      <button onClick={decrement}>-</button>
    </div>
  );
}
```

## 6. Context API 状态管理

### Context API 使用场景

```mermaid
graph TD
    A[Context API 适用场景] --> B[主题切换]
    A --> C[用户认证]
    A --> D[语言国际化]
    A --> E[全局配置]

    B --> B1[暗色/亮色模式]
    B --> B2[主题颜色配置]

    C --> C1[登录状态]
    C --> C2[用户信息]

    D --> D1[多语言支持]
    D --> D2[文本翻译]

    E --> E1[应用配置]
    E --> E2[全局设置]
```

### Context API 使用示例

```javascript
// 创建 Context
const ThemeContext = React.createContext();

// 创建 Provider
function ThemeProvider({ children }) {
  const [theme, setTheme] = useState('light');

  return (
    <ThemeContext.Provider value={{ theme, setTheme }}>
      {children}
    </ThemeContext.Provider>
  );
}

// 使用 Context
function ThemedButton() {
  const { theme, setTheme } = useContext(ThemeContext);

  return (
    <button
      style={{ backgroundColor: theme === 'light' ? '#fff' : '#333' }}
      onClick={() => setTheme(theme === 'light' ? 'dark' : 'light')}
    >
      Toggle Theme
    </button>
  );
}
```

## 7. 状态管理最佳实践

### 状态分类

```mermaid
graph TD
    A[React 状态分类] --> B[UI 状态]
    A --> C[服务器状态]
    A --> D[表单状态]
    A --> E[全局状态]

    B --> B1[模态框开关]
    B --> B2[选中项]
    B --> B3[输入框焦点]

    C --> C1[API 数据]
    C --> C2[缓存数据]
    C --> C3[加载状态]

    D --> D1[表单字段]
    D --> D2[验证状态]
    D --> D3[提交状态]

    E --> E1[用户信息]
    E --> E2[应用配置]
    E --> E3[主题设置]
```

### 状态管理选择指南

```mermaid
graph TD
    A[选择状态管理方案] --> B{状态范围}

    B -->|单个组件内部| C[useState/useReducer]
    B -->|多个组件共享| D{状态复杂度}

    D -->|简单状态| E[Context API]
    D -->|复杂状态| F{应用规模}

    F -->|中小型应用| G[Zustand/Jotai]
    F -->|大型应用| H[Redux]

    C --> C1[组件内部简单状态]
    E --> E1[主题 用户信息]
    G --> G1[快速开发 简单配置]
    H --> H1[复杂业务 生态完善]

    style C fill:#e8f5e9
    style E fill:#fff4e1
    style G fill:#fce4ec
    style H fill:#e1f5ff
```

## 8. 性能优化技巧

### 避免不必要的渲染

```mermaid
graph TD
    A[避免不必要的渲染] --> B[React.memo]
    A --> C[useMemo]
    A --> D[useCallback]
    A --> E[状态拆分]

    B --> B1[记忆组件渲染结果]
    B --> B2[props 不变时跳过渲染]

    C --> C1[缓存计算结果]
    C --> C2[依赖不变时返回缓存值]

    D --> D1[缓存函数引用]
    D --> D2[避免子组件不必要的渲染]

    E --> E1[将频繁变化的状态拆分]
    E --> E2[减少组件重新渲染范围]
```

### 状态管理性能优化示例

```javascript
// 使用 React.memo 优化子组件
const ExpensiveChild = React.memo(function ExpensiveChild({ data, onUpdate }) {
  // 组件逻辑
});

// 使用 useMemo 缓存计算结果
const processedData = useMemo(() => {
  return expensiveCalculation(data);
}, [data]);

// 使用 useCallback 缓存函数
const handleUpdate = useCallback((newValue) => {
  onUpdate(newValue);
}, [onUpdate]);

// 状态拆分
const [name, setName] = useState('');
const [age, setAge] = useState(0);
// 而不是
const [user, setUser] = useState({ name: '', age: 0 });
```
