# React 基础概念

## 1. React 内联样式

在 React 中直接在元素内使用 CSS 样式是通过内联样式来实现的。

内联样式是将 CSS 样式直接作为一个对象传递给元素的style属性，每个样式属性都以驼峰命名法表示，而不是传统的 CSS 属性名称。

**示例：**
```jsx
<div style={{ color: 'red', fontSize: '16px', backgroundColor: '#f0f0f0' }}>
  Hello World
</div>
```

## 2. React Hooks 规则

### 仅在顶层使用 Hooks
不要在循环、条件或嵌套函数中调用 Hook，确保 Hooks 在每次渲染时都以相同的顺序被调用。

**错误示例：**
```jsx
// ❌ 错误：在条件语句中使用 Hook
if (condition) {
  useEffect(() => {
    // ...
  }, []);
}

// ❌ 错误：在循环中使用 Hook
for (let i = 0; i < 10; i++) {
  useState(i);
}
```

**正确示例：**
```jsx
// ✅ 正确：在顶层使用 Hook
useEffect(() => {
  if (condition) {
    // 在 Hook 内部使用条件判断
  }
}, []);
```

### 使用 ESLint 插件
React 官方提供了 eslint-plugin-react-hooks 插件来帮助你检查 Hook 的使用是否正确。非命令行，需要在配置文件里配置。

**配置示例：**
```json
{
  "eslintConfig": {
    "extends": ["react-app"],
    "plugins": ["react-hooks"],
    "rules": {
      "react-hooks/rules-of-hooks": "error",
      "react-hooks/exhaustive-deps": "warn"
    }
  }
}
```

## 3. useMemo 和 React.memo 的区别

| 特性 | useMemo | React.memo |
|------|---------|------------|
| 类型 | Hook | 高阶组件 (HOC) |
| 缓存对象 | 计算结果 | 整个组件渲染结果 |
| 触发条件 | 依赖项数组变化 | props 变化 |
| 使用位置 | 组件内部 | 组件外部定义时包裹 |
| 主要目的 | 避免重复计算 | 避免重复渲染 |

### useMemo 示例
```jsx
const expensiveValue = useMemo(() => {
  return computeExpensiveValue(a, b);
}, [a, b]);
```

### React.memo 示例
```jsx
const MyComponent = React.memo(function MyComponent(props) {
  // 组件逻辑
});
```

## 4. useMemo 的应用场景

### 4.1 复杂计算结果的缓存
当你的组件需要根据输入进行复杂的计算，而这些计算结果不会频繁变化时，使用useMemo可以避免在每次渲染时重复计算。

**示例：**
```jsx
const sortedList = useMemo(() => {
  return items.sort((a, b) => a.value - b.value);
}, [items]);
```

### 4.2 外部库的集成
如果你在使用React组件时集成了第三方库，而这些库的API调用结果依赖于组件的props或state，你可以使用useMemo来缓存这些API调用的结果。

**示例：**
```jsx
const chartData = useMemo(() => {
  return generateChartData(props.data, props.options);
}, [props.data, props.options]);
```

### 4.3 DOM节点的引用
当你需要获取DOM节点的引用，并且这个引用依赖于某些状态或属性时，可以使用useMemo来确保引用只在必要时更新。

**示例：**
```jsx
const elementRef = useMemo(() => {
  return document.getElementById(props.id);
}, [props.id]);
```

## 5. useCallback 的应用场景

### 5.1 事件处理器的传递
当你需要将事件处理器函数传递给子组件，并且希望这些处理器在父组件的依赖项不变时保持稳定。

**示例：**
```jsx
const handleClick = useCallback(() => {
  console.log('Button clicked');
}, []);

return <ChildComponent onClick={handleClick} />;
```

### 5.2 优化性能依赖项
当你的组件依赖于某些性能敏感的props，并且这些props可能会导致组件频繁重渲染时。

**示例：**
```jsx
const handleSearch = useCallback((query) => {
  setSearchResults(filterData(query));
}, []);
```

### 5.3 与React之外的库协同工作
如果你在使用React时需要与非React库（例如，图表库、动画库等）协同工作。

**示例：**
```jsx
const chartCallback = useCallback((data) => {
  externalChart.update(data);
}, [data]);
```

## 6. useState 和 useRef 的区别

| 特性 | useState | useRef |
|------|----------|--------|
| 更新触发渲染 | ✅ 更新会触发组件重新渲染 | ❌ 更新 .current 不会触发渲染 |
| 适用场景 | 适合管理需要影响 UI 的状态 | 适合保存不影响 UI 的数据或 DOM 引用 |

### useState 示例
```jsx
const [count, setCount] = useState(0);

// 更新状态会触发重新渲染
setCount(count + 1);
```

### useRef 示例
```jsx
const inputRef = useRef(null);

// 更新 ref 不会触发重新渲染
inputRef.current = document.getElementById('input');
```

## 7. React 热重载特性

- React 热重载不会刷新 ts 文件，只会刷新 tsx 文件
- React 使用 JSX/TSX，它本质上是 JavaScript 表达式，不是模板
- JSX 在编译阶段只是转成 React.createElement(...) 调用

## 8. Hooks 使用注意事项

### 不能在不确定是否稳定渲染生成的嵌套组件里使用 React 钩子

当 React 的组件是嵌套的，且被嵌套的子组件通过 if/else 判断，不确定能否稳定会被渲染生成，且子组件里使用了钩子函数。

**问题场景：**
```jsx
// ❌ 错误示例
function Parent() {
  const [showChild, setShowChild] = useState(true);

  return (
    <div>
      {showChild && <Child />}
    </div>
  );
}

function Child() {
  const [count, setCount] = useState(0); // 可能导致 hooks 数量不一致
  // ...
}
```

**正确做法：**
```jsx
// ✅ 正确示例
function Parent() {
  const [showChild, setShowChild] = useState(true);

  return (
    <div>
      <Child visible={showChild} />
    </div>
  );
}

function Child({ visible }) {
  const [count, setCount] = useState(0);

  if (!visible) {
    return null;
  }

  // ...
}
```

## 9. MUI Stack 组件

在 React + MUI 里，`<Stack>` 是一个非常常用的布局组件。它的作用就是帮你快速实现 flexbox 布局，而不用自己写一堆 display:flex, flexDirection, gap 等 CSS。

**示例：**
```jsx
<Stack direction="row" spacing={2}>
  <Button>Button 1</Button>
  <Button>Button 2</Button>
  <Button>Button 3</Button>
</Stack>
```

## 10. MUI Tooltip 问题

在使用 MUI 的 Tooltip 时，如果在一个很小的面板里，当点击一个边缘区域时，Tooltip 可能直接会脱离面板，显示在了整个页面的左上角。

### 问题原因
Tooltip 默认是基于它的锚点元素（children）来计算位置的。如果你的 `<Tooltip>` 包裹的子元素在点击时没有实际 DOM 尺寸，MUI 就会退回到 (0,0)，也就是页面左上角。

### 解决方案

#### 1. 确保 Tooltip 有有效的锚点元素
```jsx
// ✅ 正确：包裹实际的 DOM 节点
<Tooltip title="Help">
  <span>Hover me</span>
</Tooltip>
```

#### 2. 使用 PopperProps 限制 Tooltip 的容器
```jsx
<Tooltip
  title="Help"
  PopperProps={{
    disablePortal: true,   // 限制在当前容器内
  }}
>
  <span>Hover me</span>
</Tooltip>
```

## 11. MUI 版本升级

### 升级步骤
```bash
yarn add @mui/material@latest @emotion/react @emotion/styled
```

### 版本信息
当前版本：`"@mui/material": "6.4.12"`

## 12. 获取屏幕位置

```jsx
const handleMouseMove = (event: React.MouseEvent<HTMLElement>) => {
  mousePresentRef.current = true;

  // getBoundingClientRect() 提供了元素的大小及其相对于视口的位置
  const boundingRect = event.currentTarget.getBoundingClientRect();

  console.log('Element position:', {
    left: boundingRect.left,
    top: boundingRect.top,
    width: boundingRect.width,
    height: boundingRect.height
  });
};
```

## 13. useMountedState Hook

useMountedState() 是 ReactUse 库提供的一个 Hook，用来检测组件是否已经挂载。

### 基本用法
```jsx
import { useMountedState } from 'react-use';

function MyComponent() {
  const isMounted = useMountedState();

  useEffect(() => {
    const fetchData = async () => {
      const data = await api.getData();

      // 只有组件还挂载时才更新状态
      if (isMounted()) {
        setState(data);
      }
    };

    fetchData();
  }, [isMounted]);
}
```

### 使用场景
- 在异步操作中避免更新已卸载组件的状态
- 确保只有组件还在时才更新状态

## 14. useSessionStorage Hook

在 React 中，useSessionStorage 是一个常见的自定义 Hook，用于让组件的状态和浏览器的 sessionStorage 保持同步。

### 基本用法
```jsx
import { useSessionStorage } from 'react-use';

function Demo() {
  const [username, setUsername] = useSessionStorage('username', 'Guest');

  return (
    <div>
      <p>当前用户: {username}</p>
      <button onClick={() => setUsername('Ori')}>切换用户</button>
    </div>
  );
}
```

### 特性
- 数据在页面刷新后仍然存在
- 关闭浏览器标签页或窗口后会清除
- 同一个标签页内的不同页面可以共享数据
- 不同标签页/窗口不能共享数据

### 作用范围对比

| 存储方式 | 作用范围 | 持久性 |
|---------|---------|--------|
| sessionStorage | 当前标签页 | 关闭标签页清除 |
| localStorage | 所有标签页 | 持久化到浏览器关闭后 |

## 15. React vs react-use vs ahooks

### 对比表

| 特性 | React | react-use | ahooks |
|------|-------|-----------|--------|
| 定位 | 前端框架 | Hooks 工具库 | 企业级高质量 Hooks 库 |
| 提供内容 | 核心 API + 基础 Hooks | 100+ 通用 Hooks | 基础 + 高级业务 Hooks |
| 网络请求支持 | 基础 | 基础（useAsync） | 强大（useRequest） |
| 状态管理 | 基础 | 全面（useSetState, useMap） | 实用（useBoolean, useToggle） |
| 浏览器 API | 基础 | 丰富（媒体、存储、窗口） | 一般 |
| 适用场景 | 构建 UI、状态管理 | 快速开发、工具型项目 | 企业级项目、复杂业务逻辑 |
| 维护者 | Meta 官方 | 社区（streamich） | 阿里巴巴蚂蚁金服团队 |

### React
- 内置 Hooks（如 useState, useEffect, useContext）
- 负责虚拟 DOM 渲染和状态管理的基础能力
- 只提供最基础的功能

### react-use
- 提供超过 100 个常用的自定义 Hooks
- 封装了浏览器 API（如 useWindowSize, useSessionStorage, useMedia）
- 更偏向"工具箱"，覆盖面广

### ahooks
- 提供基础 Hooks（如 useBoolean, useToggle）
- 特别优化了异步请求、表单与表格联动、性能优化
- 支持 SSR，避免闭包陷阱
- 更贴近实际业务场景

## 16. Fluent UI 图标库

### 安装
```bash
yarn add @fluentui/react-icons
```

### 使用示例
```tsx
import { GripperBarVertical20Regular } from "@fluentui/react-icons";

export default function Demo() {
  return (
    <div>
      <GripperBarVertical20Regular />
    </div>
  );
}
```

### 命名规则
图标名后面通常会带上：
- **尺寸**：16、20、24、28 等
- **样式**：Regular、Filled、Color

**示例：**
- `News28Regular` - 28px，常规样式
- `Next24Filled` - 24px，填充样式
- `Alert16Color` - 16px，彩色样式

### 推荐网站
- **Flicon (flicon.io)**：专门的 Fluent UI 图标搜索和展示网站，收录了近 2000 个图标
- **Fluent 2 Design System 官方文档**：微软官方的图标使用指南

### 使用技巧
在 Flicon 网站里，你可以：
- 输入图标名字（如 Next24Regular）快速定位
- 按类别浏览，比如 "Arrows"、"Charts"、"Media Controls"
- 查看不同样式（Filled、Regular、Color）和尺寸（16、20、24、28）
