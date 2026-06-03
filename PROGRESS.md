# Project Development Progress (PROGRESS.md)

> 状态图例：`[x]` 已实现并接线 · `[~]` 已实现但有待完善项 · `[ ]` 未开始
> 行数仅作实现规模参考，非质量指标。

## 📈 基础框架建设状态
- [x] 桌面 Launcher 核心主控 (`src/ui/ui.c`, 599 行) — 仿 iOS 网格滑动布局，含 3 页分页、页面指示圆点、底部 Dock（AI/时钟/天气/日历）、翻页时钟、APP 启动回调分发
- [~] 硬件隔离抽象层接口定义 (`src/hal/`, hal.c 34 行) — 基础接口已搭，需随硬件移植继续填充
- [x] 公用自定义 UI 组件库 (`src/ui/components/`)
  - [x] 全局状态栏 (`ui_statusbar.c`, 238 行)
  - [x] 全局标题栏 (`ui_titlebar.c`, 97 行)
- [x] 全局自适应配置 (`src/ui/ui_config.h`) — 按屏宽动态计算栏高/图标/间距/圆角，128→480+ 自适应
- [x] FreeRTOS 适配层 (`src/freertos/`) + 程序入口 (`src/main.c` / `src/freertos_main.c`)

## 🛠 系统内置核心应用 (`src/ui/pages/system/`)
- [x] **计算器** (`calculator/ui_calculator.c`, 311 行)
- [x] **日历** (`calendar/ui_calendar.c`, 122 行) — 基于 LVGL 内置 `lv_calendar`，启用农历模式，今日高亮
- [x] **时钟与闹钟** (`clock/`)
  - [x] 时钟主界面仪表盘 (`ui_clock.c`, 668 行)
  - [x] 新增闹钟/时间编辑页 (`ui_alarm_create.c`, 1073 行)
- [~] **备忘提醒** (`reminder/ui_reminder.c`, 145 行) — 列表/勾选/新增已实现；当前为内置示例数据，缺持久化与编辑/删除
- [x] **系统设置** (`settings/ui_settings.c`, 273 行) + WiFi 子页 (`lvgl_9_5_wi_fi.cpp`, 75 行)
- [x] **天气预报** (`weather/ui_weather.c`, 363 行)

## 🚀 扩展学习应用 (`src/ui/pages/app/`)
- [~] **古诗词伴读** (`poetry/`, ui_poetry.c 105 行 + ui_poetry_view.c 259 行) — Presenter/View 分层路由（首页/列表/详情）完整；详情数据当前写死（《静夜思》），音频播放为 TODO 接口

## 🎮 益智高刷新率游戏 (`src/ui/pages/games/`)
- [x] **复古飞机大战** (`airplane/flygame.c`, 612 行) — 含敌机/Boss/爆炸等美术资源
- [x] **植物大战僵尸微型版** (`pvz/pvz.c`, 1531 行) — 含豌豆/坚果/樱桃/核弹/阳光/僵尸/地图等美术资源

## 🤖 AI 助手
- [~] 桌面图标已接线，模拟器端通过 `app_launch_callback_t` 回调 `"ai_assistant"` 交由硬件端「AI 小智」实现；模拟器内无独立页面（设计如此）

---

## ⚠️ 已知待完善 / 跟进项
- **桌面图标资源缺失**：`诗词园地`、`飞机游戏`、`pvz游戏` 三项在 `ios_icons[]` 中为 `NULL` 占位，功能可启动但桌面无图标显示，需补 `assets/` 图片。
- **数据持久化**：提醒事项、闹钟、诗词详情均为内存/写死数据，尚无 Flash/NVS 持久化层。
- ~~架构文档偏差：`ARCHITECTURE.md` 目录树未列实际新增文件~~ → 已于 2026-06-03 补齐全量文件清单 + 分层依赖规则 + 数据流。
- **airplane 旧 API 技术债**：`games/airplane/flygame.c` 仍用 8.x 的 `LV_IMG_DECLARE` / `lv_img_create`，应迁移到 9.5 的 `LV_IMAGE_DECLARE` / `lv_image_create`（见 CONSTRAINTS.md §9）。
- **状态栏 `clear_flag` 历史写法**：calculator / calendar 用了 8.x 别名 `lv_obj_clear_flag`，应统一替换为 `lv_obj_remove_flag`（见 CONSTRAINTS.md §7）。

## 📅 修订与审计日志
- **2026-06-03 14:50**: 全量核对 `src/` 实际实现，将 PROGRESS 由「全部未开始」同步为真实状态——10 个应用均已实现并接线至 Launcher，标注 reminder/poetry/hal/AI 的待完善项及图标资源缺口。
- **2026-06-03 11:00**: 针对 LVGL 9.5 正式版标准、ESP32-S3R8 硬件边界及用户指定的树状目录结构，初始化全面 Harness 约束规则。
