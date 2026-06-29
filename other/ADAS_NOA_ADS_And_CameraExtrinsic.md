# 相机外参与 ADAS / NOA / ADS 解析

## 一、相机外参结果解析

原始数据：

```json
{
    "f32Pitch": 2.2104527950286865,
    "f32R1": -1.1799156665802002,
    "f32R2": 1.1812087297439575,
    "f32R3": -1.226181149482727,
    "f32Roll": 0.06999969482421875,
    "f32T1": 1813.0,
    "f32T2": -18.0,
    "f32T3": 1325.0,
    "f32Yaw": -0.010000228881835938,
    "f32crtConf": 0.0,
    "s32Reserved": [],
    "s32crtError": 0,
    "s32crtRoadState": 0,
    "s32crtState": 1,
    "u8CameraSource": 0
}
```

---

## 二、Pitch / Yaw / Roll

### Pitch（俯仰角）

```text
Pitch = 2.21045°
```

表示摄像头相对于车辆坐标系：

* 正值通常表示向下俯视
* 负值表示向上抬起

当前值：

```text
约 2.21°
```

属于前视ADAS相机非常常见的安装角度。

---

### Yaw（偏航角）

```text
Yaw = -0.01°
```

表示左右偏转。

当前值接近：

```text
0°
```

说明：

```text
摄像头左右安装基本居中
```

---

### Roll（横滚角）

```text
Roll = 0.07°
```

表示左右倾斜。

当前结果：

```text
≈ 0°
```

说明：

```text
摄像头水平安装较好
```

---

### 综合分析

当前姿态：

```text
Pitch = 2.21°
Yaw   = -0.01°
Roll  = 0.07°
```

符合典型ADAS前视相机安装状态：

```text
Pitch : 1°~5°
Yaw   : ≈0°
Roll  : ≈0°
```

属于合理结果。

---

# 三、R1 R2 R3 的含义

结果：

```text
R1 = -1.179915
R2 =  1.181209
R3 = -1.226181
```

它们通常来自：

```python
rvec, _ = cv2.Rodrigues(R)
```

即：

```text
Rodrigues Rotation Vector
```

旋转向量表示。

---

## Rodrigues 向量

对应：

```python
rvec =
[
    -1.179915,
     1.181209,
    -1.226181
]
```

计算长度：

```python
theta = np.linalg.norm(rvec)
```

结果约：

```text
theta ≈ 2.07 rad
```

转换为角度：

```text
≈118.7°
```

含义：

```text
绕某个单位旋转轴旋转118.7°
```

这是OpenCV内部常用的旋转表达方式。

---

## 与旋转矩阵关系

```text
Rotation Matrix
      ↓
Rodrigues()
      ↓
Rotation Vector
```

可以互相转换：

```python
R, _ = cv2.Rodrigues(rvec)
```

或者：

```python
rvec, _ = cv2.Rodrigues(R)
```

---

# 四、T1 T2 T3 的含义

结果：

```text
T =
[
    1813,
    -18,
    1325
]
```

表示平移向量：

```text
Translation Vector
```

---

## 单位

通常是：

```text
mm
```

因此：

```text
X = 1813 mm
Y = -18 mm
Z = 1325 mm
```

换算：

```text
X = 1.813 m
Y = -0.018 m
Z = 1.325 m
```

---

## 实际意义

可理解为：

```text
摄像头相对于车辆参考坐标系：

前方 1.813m
左/右偏移 1.8cm
高度 1.325m
```

对于车载前视相机：

```text
非常合理
```

---

# 五、Pitch/Yaw/Roll 与 R1/R2/R3 的关系

很多人容易误解：

```text
Pitch/Yaw/Roll

和

R1/R2/R3
```

是不是两套不同的姿态。

实际上：

```text
Pitch/Yaw/Roll
        ↓
Rotation Matrix
        ↓
Rodrigues
        ↓
R1/R2/R3
```

本质表达的是：

```text
同一个旋转
```

只是表示形式不同。

---

## 使用场景

### 人类阅读

通常使用：

```text
Pitch
Yaw
Roll
```

因为容易理解。

---

### OpenCV

通常使用：

```text
rvec
```

即：

```text
R1
R2
R3
```

因为：

```text
3个数即可表达旋转
```

比9个数的矩阵更方便。

---

### 投影计算

通常使用：

```text
Rotation Matrix
```

例如：

```python
cv2.projectPoints()
```

内部实际使用矩阵形式。

---

# 六、pybind11 返回值对应关系

Python侧：

```python
rV, _ = cv2.Rodrigues(R_total)

return rV.flatten()
```

返回：

```python
[
    -1.179915,
     1.181209,
    -1.226181
]
```

对应：

```json
{
    "f32R1": -1.179915,
    "f32R2": 1.181209,
    "f32R3": -1.226181
}
```

---

C++侧：

```cpp
std::array<float, 3> rv;
```

对应关系：

```cpp
rv[0] = f32R1;
rv[1] = f32R2;
rv[2] = f32R3;
```

完全一致。

---

# 七、ADAS 的定义

ADAS：

```text
Advanced Driver Assistance Systems
```

中文：

```text
高级驾驶辅助系统
```

特点：

```text
驾驶员始终负责驾驶
```

系统只是辅助。

---

## 常见功能

### ACC

Adaptive Cruise Control

```text
自适应巡航
```

---

### AEB

Automatic Emergency Braking

```text
自动紧急制动
```

---

### LKA

Lane Keep Assist

```text
车道保持
```

---

### BSD

Blind Spot Detection

```text
盲区检测
```

---

### TJA

Traffic Jam Assist

```text
拥堵辅助
```

---

## 等级

通常属于：

```text
L0 ~ L2
```

---

# 八、NOA 的定义

NOA：

```text
Navigate On Autopilot
```

中文：

```text
导航辅助驾驶
```

---

## 核心能力

融合：

```text
导航地图
+
感知
+
规划决策
```

实现：

```text
自动跟车
自动变道
自动超车
自动上下匝道
```

---

## 场景

例如：

```text
北京 → 上海
```

系统根据导航：

```text
提前变道
进入匝道
驶出高速
```

---

## 驾驶体验

```text
上高速
    ↓
开启NOA
    ↓
车辆自动行驶
    ↓
到达出口提醒接管
```

---

## 本质

NOA依然属于：

```text
L2+
```

驾驶员负责。

---

# 九、ADS 的定义

ADS：

```text
Automated Driving System
```

中文：

```text
自动驾驶系统
```

---

## SAE等级

| 等级 | 含义      |
| -- | ------- |
| L0 | 无辅助     |
| L1 | 辅助驾驶    |
| L2 | 组合辅助驾驶  |
| L3 | 有条件自动驾驶 |
| L4 | 高度自动驾驶  |
| L5 | 完全自动驾驶  |

---

## ADS对应

通常指：

```text
L3+
```

甚至：

```text
L4
```

---

## 最大区别

责任主体发生变化。

### L2

```text
事故责任：
驾驶员承担
```

---

### L3

```text
事故责任：
系统承担部分责任
```

这是法律层面的核心区别。

---

# 十、华为 ADS 与 SAE ADS 的区别

很多人容易混淆。

---

## 华为 ADS

全称：

```text
Advanced Driving System
```

这是：

```text
产品名称
```

不是法规定义。

---

例如：

```text
ADS SE
ADS Pro
ADS Max
ADS Ultra
```

本质仍然属于：

```text
L2/L2+
```

驾驶员必须：

```text
持续监控道路
随时接管
```

---

## SAE ADS

指：

```text
Automated Driving System
```

对应：

```text
L3+
```

甚至：

```text
L4
```

是真正意义上的自动驾驶系统。

---

# 十一、ADAS、NOA、ADS关系总结

可理解为：

```text
ADAS
 └── NOA
      └── ADS
```

或者：

```text
ADAS
    ↓
高阶ADAS
    ↓
NOA
    ↓
ADS
```

---

## 功能对比

| 功能     | ADAS  | NOA  | ADS   |
| ------ | ----- | ---- | ----- |
| ACC    | √     | √    | √     |
| AEB    | √     | √    | √     |
| LKA    | √     | √    | √     |
| 自动变道   | ×     | √    | √     |
| 自动超车   | ×     | √    | √     |
| 上下匝道   | ×     | √    | √     |
| 红绿灯通行  | ×     | 部分支持 | √     |
| 城区复杂博弈 | ×     | 部分支持 | √     |
| 驾驶责任   | 司机    | 司机   | 系统    |
| SAE等级  | L0-L2 | L2+  | L3-L5 |

---

# 十二、一句话总结

ADAS：

```text
帮你开车
```

NOA：

```text
替你开一部分路
```

ADS：

```text
真正意义上的自动驾驶
```

当前市面上大部分量产方案：

```text
华为ADS
小鹏XNGP
理想AD Max
特斯拉FSD
```

本质仍属于：

```text
L2/L2+
```

驾驶责任依然在驾驶员。
