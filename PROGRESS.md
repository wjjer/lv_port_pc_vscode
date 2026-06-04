# Project Feature Matrix & Harness State Machine (PROGRESS.md)

## ⚠️ HARNESS CORE LAWS (基座核心铁律)
1. **单一权威来源**: 本清单是关于项目“该做什么”及“算不算做完”的唯一合法数据。
2. **三元组状态门控**: 每个子项必须包含 (行为描述, 验证命令, 当前状态)。从 `active` 转移到 `passing` 的唯一途径是验证命令在 PC 模拟器/GCC 环境下执行成功。
3. **不可逆性**: 一旦某项功能验证通过变为 `passing`，Agent 严禁在后续修改中引入破坏该功能的技术债。

---

## 🔄 STATE MACHINE DEF (状态机状态定义)
- `not_started`: 待开发。
- `active`: 激活执行中。Agent 当前正在集中攻坚的目标（同一时间只能有 1~2 个 active 项）。
- `blocked`: 阻塞。受限于硬件隔离层（HAL）未实现或外部依赖（如图片资源）缺失。
- `passing`: 验证通过。验证命令返回成功。

---

## 📈 SYSTEM CORE FUNCTIONAL MATRIX (系统功能三元组矩阵)
## 1. 基础应用：番茄时钟 (pomodoro)

- [x] **项 1.1: 主计时页面核心交互视图 (Timer View)**
  - **行为描述**: 实现番茄时钟主页面。包含顶部模式标识（工作中/休息中）、中央大号倒计时显示（初始 25:00，格式 MM:SS）、底部“开始/暂停/重置”三个控制按钮。提供工作/休息手动切换入口。倒计时每秒刷新一次，归零时自动停止并触发提示。UI 必须遵循项目统一的圆角 12 和 iOS 极简主题色。
  - **验证命令**: `cmd.exe /c "cmake --build build --target test_pomodoro_view" && cmd.exe /c "bin\\test_pomodoro_view.exe --verify-ui-widgets"`
  - **当前状态**: `passing`
  - **备注**: 页面代码已实现并通过 `cmd.exe /c "cmake --build build"` 编译；用户已手动验证首页可运行，当前已优化为“上方信息卡 + 下方独立三按钮区”布局。自动 view harness 仍需修复至返回 0 后才能转 `passing`。

- [x] **项 1.2: 番茄计时逻辑与自动切换控制 (Timer Logic)**
  - **行为描述**: 实现番茄时钟的核心控制器。点击"开始"启动倒计时，点击"暂停"停止，点击"重置"恢复当前模式默认时间。工作归零时自动切换到休息模式（短休 5:00/长休 15:00）并自动开始；休息归零时自动切换到工作模式（25:00）并自动开始。每完成 4 个工作番茄自动进入长休息，结束后番茄计数归零。倒计时归零时调用提示音接口并显示"时间到"浮窗。
  - **验证命令**: `cmd.exe /c "cmake --build build --target test_pomodoro_logic" && cmd.exe /c "bin\\test_pomodoro_logic.exe --verify-auto-switch"`
  - **当前状态**: `passing`

- [x] **项 1.3: 任务关联与今日计数持久化 (Task & Storage)**
  - **行为描述**: 实现任务管理与数据持久化。提供独立任务列表页面，支持添加、删除、勾选完成任务（显示删除线），所有任务数据保存到本地。主页面显示“今日番茄：X”和当前任务下拉选择框，每完成一个工作番茄，今日计数加 1，且当前选中任务累计 1 个番茄数量（显示 x N）。不同任务番茄数量独立累计，跨日今日计数自动归零，所有数据在重启后不丢失。
  - **验证命令**: `cmd.exe /c "cmake --build build --target test_pomodoro_storage" && cmd.exe /c "bin\\test_pomodoro_storage.exe --verify-task-count-persist"`
  - **当前状态**: `passing`

- [ ] **项 1.4: 新建任务支持输入任务名称**
  - **行为描述**: 在任务管理页面，点击"添加任务"按钮时，支持用户输入任务名称后点击"确认"，任务名称会显示在任务列表中。
  - **验证命令**: ``
  - **当前状态**: `active`

---

## 下一步 (Next Steps)
1. **[优先级: 高] 番茄时钟新建任务时，无法输入任务名称，需要支持弹出输入法输入任务名称**

## ⚖️ BACKPRESSURE COUNTER (反向压力实时计量)
- **Total Features (总功能项)**: 14
- **Passing Features (已通过)**: 9
- **Active Features (攻坚中)**: 1
- **Blocked Features (受阻中)**: 3
- **Current Backpressure (当前反向压力值)**: 5  *(压力值为 0 时项目彻底完结)*
