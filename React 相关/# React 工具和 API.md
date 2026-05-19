# React 工具和 API

## 1. Fluent UI 图标库

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
图标名后面通常会带上尺寸（16、20、24、28 等）和样式（Regular、Filled、Color）。

### 推荐网站
- **Flicon (flicon.io)**：专门的 Fluent UI 图标搜索和展示网站，收录了近 2000 个图标
  - 可以按类别（箭头、文档、媒体控制、天气等）浏览
  - 也可以搜索具体图标名称，比如 News28Regular 或 Next24Filled
- **Fluent 2 Design System 官方文档**：在 Fluent 2 React Icon 页面可以看到微软官方的图标使用指南和部分展示
- **npm 包页面**：@fluentui/react-icons 提供了安装方法和使用示例

### 使用技巧
在 Flicon 网站里，你可以：
- 输入图标名字（如 Next24Regular）快速定位
- 按类别浏览，比如 "Arrows"、"Charts"、"Media Controls"
- 查看不同样式（Filled、Regular、Color）和尺寸（16、20、24、28）

### 常见图标命名规则对照表

| 样式 | 说明 | 示例 |
|------|------|------|
| Filled | 填充样式 | Next24Filled |
| Regular | 常规样式 | Next24Regular |
| Color | 彩色样式 | News28Color |
| 尺寸 | 16、20、24、28 | GripperBarVertical20Regular |

## 2. JSON Editor API

### 常用 API

#### 1. 内容操作

| 方法 | 用途 | 示例 |
|------|------|------|
| set(content) | 设置新的 JSON 内容（覆盖原有内容） | editor.set({ json: { hello: "world" } }) |
| get() | 获取当前编辑器内容 | const data = editor.get() |
| update(content) | 更新部分内容（不会覆盖全部） | editor.update({ json: { foo: "bar" } }) |

#### 2. 属性更新

| 方法 | 用途 | 示例 |
|------|------|------|
| updateProps(props) | 更新编辑器属性（模式、只读等） | editor.updateProps({ mode: "text", readOnly: true }) |

#### 3. 展开/折叠

| 方法 | 用途 | 示例 |
|------|------|------|
| expand() | 展开所有节点（仅在 tree 模式下有效） | editor.expand() |
| collapse() | 折叠所有节点（仅在 tree 模式下有效） | editor.collapse() |

#### 4. 生命周期

| 方法 | 用途 | 示例 |
|------|------|------|
| destroy() | 销毁编辑器实例，释放资源 | editor.destroy() |

### 常用 Props
```typescript
props: {
  content: { json: { hello: "world" } }, // 初始内容
  mode: "tree",                          // 模式: "tree" | "text" | "table" | "code"
  readOnly: false,                       // 是否只读
  indentation: 2,                        // 缩进空格数
  onChange: (updated) => {               // 内容变化回调
    console.log("更新后的 JSON:", updated);
  },
  onError: (err) => {                    // 错误回调
    console.error("编辑器错误:", err);
  }
}
```

### 总结
- **内容相关**：set、get、update
- **属性相关**：updateProps
- **树形操作**：expand、collapse
- **生命周期**：destroy

## 3. MUI Tooltip 问题

### 问题描述
在使用MUI的Tooltip时，如果在一个很小的面板里，当点击一个边缘区域时，Tooltip可能直接会脱离面板，显示在了整个页面的左上角。

### 问题原因
MUI Tooltip 的定位逻辑：
- Tooltip 默认是基于它的 锚点元素（children） 来计算位置的
- 如果你的 <Tooltip> 包裹的子元素在点击时没有实际 DOM 尺寸（比如是空标签、或者渲染在边缘导致定位失败），MUI 就会退回到 (0,0)，也就是页面左上角

### 解决思路

#### 1. 确保 Tooltip 有有效的锚点元素
Tooltip 必须包裹一个实际的 DOM 节点（如 <span>、<div>），不能直接包裹 null 或空的 React Fragment。

#### 2. 使用 PopperProps 限制 Tooltip 的容器
默认情况下，Tooltip 会挂载到 body 下。如果你希望它只在小面板内显示，可以指定 container：

```tsx
<Tooltip
  PopperProps={{
    disablePortal: true,   // ✅ 正确写法：放在 PopperProps 里
  }}
>
  {/* 子元素 */}
</Tooltip>
```

## 4. MUI 升级

### 升级步骤
```bash
yarn add @mui/material@latest @emotion/react @emotion/styled
```

## 5. MUI Stack 组件

在 React + MUI 里，<Stack> 是一个非常常用的布局组件。它的作用就是帮你快速实现 flexbox 布局，而不用自己写一堆 display:flex、flexDirection、gap 等 CSS。

## 6. useMountedState()

### 定义
useMountedState() 是 ReactUse 库提供的一个 Hook，用来检测组件是否已经挂载。它返回一个函数，调用时会返回布尔值 => boolean，表示当前组件的挂载状态。

### 用法
```typescript
import { useMountedState } from 'react-use';

const isMounted = useMountedState();
```

### 使用场景
在异步操作中避免更新已卸载组件的状态。

### 总结
useMountedState() 就是一个挂载状态检测工具，常用于异步请求或定时器里，确保只有组件还在时才更新状态。

## 7. useSessionStorage

### 定义
在 React 中，useSessionStorage 是一个常见的自定义 Hook（或者来自第三方库，如 react-use/ahooks），用于让组件的状态和浏览器的 sessionStorage 保持同步。这样数据在页面刷新后仍然存在，但在关闭浏览器标签页或窗口后会清除，因为 sessionStorage 属于会话 session。

### 基本用法（React + react-use）
```bash
yarn install react-use
```

```typescript
import { useSessionStorage } from 'react-use';

function Demo() {
  // key: 'username'，默认值: 'Guest'
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
- 第一次渲染时，如果 sessionStorage 中没有 username，就会用默认值 'Guest'
- 当你调用 setUsername 更新状态时，它会同时更新 React 状态和 sessionStorage
- 刷新页面后，值仍然存在；关闭浏览器标签页后，值会消失
- 数据类型：存储时需要序列化（通常用 JSON.stringify），读取时要反序列化
- 安全性：不要在 sessionStorage 中存储敏感信息（如密码、token）

### 作用范围
- 同一个标签页内的不同页面：可以共享数据
- 不同标签页 / 窗口：不能共享
- 每个 tab 都有独立的 sessionStorage，打开新标签页时不会继承旧的值
- 关闭标签页或浏览器：数据会清除。和 localStorage 不同，sessionStorage 只在当前会话有效

### 跨标签页共享方案
如果你需要跨标签页共享：
1. 使用 localStorage：它在同一浏览器的所有标签页中共享，并且持久化到浏览器关闭后
2. 使用 BroadcastChannel API：可以在不同标签页之间实时传递消息
3. 使用后端存储（数据库 / API）：适合更复杂的跨页面、跨设备共享

## 8. React vs react-use

### React
- 内置 Hooks（如 useState, useEffect, useContext）
- 负责虚拟 DOM 渲染和状态管理的基础能力
- 特点：只提供最基础的功能，开发者需要自己封装或引入第三方库来满足复杂需求

### react-use
- 定位：一个社区维护的 React Hooks 集合库，由 streamich 开发
- 核心功能：提供超过 100 个常用的自定义 Hooks，例如：
  - useSessionStorage / useLocalStorage：和浏览器存储同步
  - useMedia：响应式媒体查询
  - useAudio / useVideo：管理 HTML5 媒体元素
  - useWindowSize：监听窗口大小变化
- 特点：开箱即用，减少重复造轮子，更偏向工具库，提升开发效率

### 对比表

| 特性 | React | react-use |
|------|-------|-----------|
| 定位 | 前端框架 | Hooks 工具库 |
| 提供内容 | 核心 API + 基础 Hooks | 大量自定义 Hooks（100+） |
| 使用场景 | 构建 UI、状态管理 | 快速调用浏览器 API、简化逻辑 |
| 维护者 | Meta 官方 | 社区（streamich 等） |
| 是否必须 | 必须，项目核心 | 可选，提升效率 |

## 9. react-use vs ahooks

### react-use
- 定位：由社区维护的 Hooks 工具库（streamich 开发）
- 特点：
  - 提供超过 100 个通用 Hooks
  - 封装了浏览器 API（如 useWindowSize, useSessionStorage, useMedia）
  - 更偏向"工具箱"，覆盖面广，适合快速开发
- 优势：
  - 功能全面，涵盖状态管理、DOM 操作、浏览器 API
  - 社区活跃，更新频繁
- 不足：
  - 对复杂业务场景支持较弱（如网络请求、表单联动）

### ahooks
- 定位：由蚂蚁金服团队开发，强调高质量和可靠性
- 特点：
  - 提供基础 Hooks（如 useBoolean, useToggle），也有高级业务 Hooks（如 useRequest, useAntdTable）
  - 特别优化了异步请求、表单与表格联动、性能优化
  - 支持 SSR，避免闭包陷阱
- 优势：
  - 在网络请求处理上很强（useRequest 支持轮询、缓存、错误重试等）
  - 更贴近实际业务场景，适合企业级项目
- 不足：
  - 功能更偏向业务，通用性不如 react-use

### 对比表

| 特性 | react-use | ahooks |
|------|-----------|--------|
| 定位 | 通用 Hooks 工具库 | 企业级高质量 Hooks 库 |
| 提供内容 | 100+ 通用 Hooks | 基础 + 高级业务 Hooks |
| 网络请求支持 | 基本（useAsync） | 强大（useRequest） |
| 状态管理 | 全面（useSetState, useMap） | 实用（useBoolean, useToggle） |
| 浏览器 API | 丰富（媒体、存储、窗口） | 一般 |
| 适用场景 | 快速开发、工具型项目 | 企业级项目、复杂业务逻辑 |
| 维护者 | 社区（streamich） | 阿里巴巴蚂蚁金服团队 |

## 10. 获取屏幕位置

```typescript
(event: React.MouseEvent<HTMLElement>) => {
  mousePresentRef.current = true;
  // getBoundingClientRect() 提供了元素的大小及其相对于视口的位置
  const boundingRect = event.currentTarget.getBoundingClientRect();
  // Element.getBoundingClientRect()
}
```

## 11. contextmenu 为什么代表右键

在浏览器的事件模型里，contextmenu 事件就是专门用来触发右键菜单的。

### 原理
当你在网页元素上右键点击时，浏览器默认会弹出上下文菜单（就是常见的"复制/粘贴/检查"等菜单）。这个动作在 DOM 事件里被定义为 contextmenu。

所以在 JavaScript 或 Leaflet 里监听 contextmenu，就等价于监听右键点击事件。

## 12. Promise.resolve()

Promise.resolve 方法可以将现有对象转为 Promise 对象，根据对象的类型和状态返回不同的 Promise 实例。

### 基本用法
```javascript
Promise.resolve(value)
```

根据参数的不同，返回不同的 Promise：
- 如果参数是 Promise 实例，则原封不动地返回这个实例
- 如果参数是一个 thenable 对象（具有 then 方法的对象），则将这个对象转为 Promise 对象，并立即执行它的 then 方法
- 如果参数不是具有 then 方法的对象，或根本就不是对象，则返回一个新的 Promise 对象，状态为 resolved
- 如果不提供参数，直接返回一个 resolved 状态的 Promise 对象
