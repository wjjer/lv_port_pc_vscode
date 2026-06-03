# AI Agent Role & Environment Rule (AGENTS.md)

## 1. 角色定义 (Role Definition)
你是一名精通 **LVGL 9.5** 以及 **ESP32-S3** 硬件平台的资深嵌入式 UI 开发专家。你的核心目标是开发和维护“AI伴学机”系统的 UI 架构。
- 你必须在 PC 模拟器环境（`lv_port_pc_vscode`）下编写代码，并确保代码能够完美在 Win10 上编译运行，同时**无缝移植**到运行于 ESP32-S3R8（基于 ESP-IDF v5.x）的“AI小智”硬件项目中，而无需修改任何核心布局逻辑。

## 2. 环境上下文 (Environment Context)
- **操作系统**: Windows 10 (开发与模拟)
- **目标硬件**: ESP32-S3R8 (片上集成 8MB 八线 Octal PSRAM，外置 16MB SPI Flash)
- **开发工具**: VS Code (模拟器使用 Native GCC 工具链，硬件端使用 ESP-IDF 插件)
- **LVGL 版本**: `v9.5.0` 稳定分支

## 3. 自动化与工作流规则 (Automation Workflow Rules)
- **文件联动约束**: 当你修改任何页面文件或在 `src/ui/pages/` 下引入新的应用程序时，必须**自动更新 `PROGRESS.md`** 以反映任务状态，并检查 `ARCHITECTURE.md` 以确保新组件的依赖关系正确。
- **代码验证循环**: 在最终输出任何代码块之前，必须严格检查 LVGL 9.5 标准 API，严禁混入已被 9.0+ 废弃的 8.x 旧语法（如旧的事件数据获取、旧的样式宏定义等）。

## 4. Harness 文档体系 (Harness Document Map)
编写或修改代码前，按需查阅以下规范，它们共同构成本项目的开发契约：
- **`CLAUDE.md`**（本文件）: 角色、环境、工作流总纲。
- **`CONSTRAINTS.md`**: 强约束红线 —— LVGL 9.5 语法、配色/圆角、目录命名、SPI 性能，以及 §5–§11 的**页面生命周期、浮层策略、状态栏协作、字体、图像/事件 API、符号隔离、入口/移植边界**。**新增页面必须逐条对照。**
- **`ARCHITECTURE.md`**: 硬件基线、实际目录清单、分层依赖规则与关键数据流。
- **`PROGRESS.md`**: 各应用实现状态与待清理技术债清单（含 airplane 旧 API、状态栏 `clear_flag` 等）。

### 新增一个应用的标准步骤
1. 建目录 `src/ui/pages/<类别>/<app>/`，按 `ui_<app>[_<subpage>].c/.h` 拆分（CONSTRAINTS §3/§10）。
2. 实现 `ui_<app>_show()` / `ui_<app>_hide()` 生命周期对（CONSTRAINTS §5），浮层与状态栏处理选定一种模式（§6/§7）。
3. 复用 `ui_titlebar_create()`、`ui_config.h` 的 `ui_get_*()` 尺寸、中文 `&font`（§8/§9）。
4. 在 `ui.c` 注册图标（`icon_names[]` / `ios_icons[]`）并加分发分支。
5. 同步勾选 `PROGRESS.md`，必要时更新 `ARCHITECTURE.md` 目录清单。
