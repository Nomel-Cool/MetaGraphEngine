# Qt Graph Engine Kit —— 界面设计说明

## 1. 设计目标

旧界面的核心问题不是“不够花哨”，而是**信息架构不可读**：

- 固定 800×600，窗口不能调整；顶部两个 Tab 占掉约 1/5 高度。
- 搜索框只有 80px；模型被拆到 26 个字母 ComboBox 里，用户不知道下一步该点哪里。
- 预览、画布、模型列表、引擎操作没有主次，按钮是 `Compile / Launch / RTR / Ceize / GLPlay` 这类不直观文案。
- 自动机表单只有五个无标签输入框，用户必须懂五字段协议。
- 失败只写控制台；预览失败甚至会从列表里删除模型。
- 3D 窗口关闭后按钮状态不复位，默认相机对小型模型取景不合适。

新界面要解决三件事：

1. **先看懂**：模型库 + 预览 + 能力徽标，知道每个模型是什么、能不能动。
2. **再做出来**：模板 → 画布 → 属性检查器 → 保存，不要求先写 JSON。
3. **再放出来**：动画引擎用步骤条和快捷操作栏，状态、错误、结果都在界面内。

## 2. 信息架构

~~~text
应用
├── 模型库（默认）
│   ├── 搜索 / 类型筛选
│   ├── 模型列表（预览、编辑、加入引擎、删除）
│   └── 大预览 + 元数据 + 操作
├── 模型设计
│   ├── 模板条
│   ├── 画布（拖拽控制点）
│   ├── 属性检查器（名称/外观/几何/能力）
│   └── 高级模式（五字段 JSON）
├── 动画引擎
│   ├── 参演模型
│   ├── 输出窗口 / 相机 / 2D-3D
│   ├── 影片名与模式
│   ├── 影片库
│   └── 活动日志
└── 帮助
    ├── 四步上手
    ├── 五字段解释
    ├── 能力矩阵
    └── 3D 操作 / 排障入口
~~~

左侧常驻模型栏（帮助页也可用），底部状态栏显示 MySQL 状态、模型数、当前操作、选中模型。

## 3. 视觉规范

| 类别 | 取值 |
|---|---|
| 背景 | `#0B0E14` |
| 面板 | `#121722` / `#182030` |
| 悬停 | `#1E283A` |
| 边框 | `#263247` / `#34445E` |
| 主文字 | `#E8EDF7` |
| 次文字 | `#8A96AC` / `#5F6B80` |
| 主色 | `#5B8CFF`，悬停 `#7AA2FF` |
| 成功/警告/危险 | `#34D399` / `#FBBF24` / `#F87171` |
| 字体 | Microsoft YaHei UI；代码 Cascadia Mono |
| 字号 | 12 / 13 / 14 / 16 / 20 / 24 / 32 |
| 间距 | 4 / 8 / 12 / 16 / 24 / 32 |
| 圆角 | 8 / 12 / 16 |
| 控件高度 | 按钮/输入 36–40；图标按钮 32；模型行 72 |

原则：

- 不用纯装饰性渐变；用层级、边框和留白区分区域。
- 所有可点击按钮都有文字或 Tooltip；不使用只有图标、没有语义的主操作。
- 状态色只用于状态，不作为大面积主色。
- 中文标签为主，算法名（`BresenhamLine`）保留英文，降低对开发者的理解成本。

## 4. 关键交互

### 新建/保存

~~~text
模板 → 画布拖拽 → 检查器数值 → 生成五字段 JSON
     → GraphFactory.Registry4Model
     → XML + MySQL + Redis DEL
     → 刷新目录 → 选中新模型
~~~

### 预览

~~~text
选中模型 → Request4ModelPreview
        → 后端自愈 XML（必要时按数据库重建）
        → 归一化 layers/bounds
        → QML Canvas 自动适配、网格、坐标轴
~~~

### 生成与播放

~~~text
选择模型 → InitWindow / SetFilmName / RoleEmplacement
        → modelsLoaded
        → Launch（离线）或 RealTimeRender（实时）
        → Ceize → filmSaved → 影片库
        → Display → Graphs Stage → 关闭 → playbackStopped
~~~

- 相机默认 Z=200；保持默认且 X/Y=0 时，启动会按参演模型范围自动取景。
- 实时窗口会阻塞主界面，状态栏和按钮在关闭后复位。
- 失败、超时、空帧都通过 Toast/活动日志显示，不再只写控制台。

## 5. 组件

- `AppButton`：primary / secondary / ghost / danger，统一高度、圆角、hover。
- `AppTextField`：统一背景、焦点/错误边框。
- `StatusChip`：MySQL 连接状态。
- `Toast`：顶部短消息，成功/错误色。
- `ConfirmDialog`：删除等破坏性操作二次确认。
- `ModelIcon`：按类型绘制点/线/椭圆/贝塞尔/元胞图标。
- `PreviewCanvas`：后端预览点集、网格、坐标轴、自动适配。
- `ShapeCanvas`：模板编辑、控制点拖拽、世界坐标 y 向上。
- `AdvancedAutomataEditor`：五字段 JSON 编辑器、格式化/校验。

## 6. 前后对比

| 维度 | 旧界面 | 新界面 |
|---|---|---|
| 窗口 | 固定 800×600 | 默认 1440×900（按屏幕限制），最小 1100×700，可缩放 |
| 导航 | 两个巨型 Tab | 四页顶部导航 + 左侧模型栏 |
| 模型检索 | 26 个字母 ComboBox | 搜索 + 类型筛选 + 列表 |
| 预览 | 无坐标、逐点、点模型会丢字段 | 自动适配、网格、坐标轴、归一化点集 |
| 建模 | 手写自动机表单 | 模板 + 画布 + 检查器；高级模式可选 |
| 引擎 | Compile/Launch/RTR/Ceize/GLPlay | 选模型 → 设置 → 开始/停止 → 影片库播放 |
| 错误 | 控制台 | Toast + 活动日志 + 启动错误页 |
| 3D | 窗口关闭后状态不复位 | engineStarted/Stopped、关闭窗口、自动取景 |

## 7. 已知设计限制

- 实时渲染仍使用独立 GLFW 窗口，未嵌入主界面；这是现有渲染内核的边界。
- 实时预览目前只对元胞自动机开放；点/线使用离线生成与播放。
- 元胞自动机初始状态随机；UI 的 10×10 编辑只是模板示意，后端不使用该初始格子。
- 椭圆/贝塞尔没有动态 Co 算法，UI 明确标注“仅静态预览”。
- 影片只保存在内存，重启后清空；没有影片删除/重命名。
- 没有浅色主题与多语言切换。

## 8. 实现映射

| 设计 | 文件 |
|---|---|
| 应用壳/状态/导航 | `resources/qmls/MainPage.qml` |
| 主题变量 | `resources/qmls/Theme.js` |
| 模型序列化/解析 | `resources/qmls/ModelFactory.js` |
| 模型库/预览 | `MainPage.qml` + `preview/PreviewCanvas.qml` |
| 设计器 | `views/DesignerView.qml` + `editor/ShapeCanvas.qml` + `editor/AdvancedAutomataEditor.qml` |
| 引擎 | `views/EngineView.qml` |
| 帮助 | `views/HelpView.qml` |
| 启动错误页 | `StartupError.qml` |
| 后端 API/自愈 | `cpps/RenderKernel.cpp`、`cpps/DataBaseManager.cpp` |
| 3D 状态/窗口 | `cpps/PixelSpace.cpp`、`cpps/GLScreen.cpp`、`cpps/GLContext.cpp` |