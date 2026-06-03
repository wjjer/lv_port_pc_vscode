# Project Architecture Guide (ARCHITECTURE.md)

## 1. 硬件规格基线 (Hardware Specification Baseline)
- **MCU**: ESP32-S3R8 (240MHz 双核 Xtensa LX7)
- **PSRAM**: 8MB 片上八线 PSRAM (在 ESP-IDF 中开启，用于存放 UI 显存及大资源缓存)
- **Flash**: 16MB 外部 SPI Flash
- **显示屏**: 320x240 分辨率，SPI 接口驱动
- **色彩深度**: RGB565 (16-bit)
- **触摸屏**: 电容式触摸屏

## 2. 系统目录架构 (System Architecture Layout)
UI 项目框架必须严格按下述树状结构组织，保持高度模块化（下方为 `src/` 实际文件清单）：
```text
project_root/
├─lvgl/                 # LVGL 核心库 (v9.5.0 稳定分支)
├─SDL2-2.32.6/          # SDL2 跨平台模拟库
└─src/
   ├─main.c               # 模拟器入口：lv_init → sdl_hal_init(320,240) → ui_init → 主循环
   ├─freertos_main.c      # FreeRTOS 模式入口（LV_USE_OS == LV_OS_FREERTOS 时）
   ├─mouse_cursor_icon.c  # 模拟器鼠标光标资源
   ├─hal/                 # HAL 层：唯一的硬件差异点
   │  ├─hal.c             #   sdl_hal_init() — SDL2 显示/输入/tick
   │  └─hal.h
   ├─freertos/            # FreeRTOS 适配层
   │  └─freertos_posix_port.c
   └─ui/                  # 纯净、可移植的 UI 层（移植时一行不改）
       ├─ui.c             # 系统 Launcher 入口（网格滑动桌面 + Dock + APP 分发）
       ├─ui.h             #   对外：ui_init() / create_icon()
       ├─ui_config.h      # ★ 分辨率适配唯一来源：ui_get_*() 动态尺寸 + APP 启动回调类型
       ├─assets/          # 图片与字库（ai/calculator/calendar/clock/reminder/settings/weather + font）
       ├─components/      # 封装好的复用 UI 组件
       │  ├─ui_statusbar.c/.h  # 全局状态栏（含快捷面板，extern status_bar）
       │  └─ui_titlebar.c/.h   # 全局标题栏（ui_titlebar_create 通用工厂）
       └─pages/           # 具体业务页面（每个目录一个应用）
           ├─app/                       # 用户级别应用
           │  └─poetry/                 # 诗词学习（Presenter/View 分层）
           │     ├─ui_poetry.c/.h       #   路由+生命周期（show/hide, route_to_*）
           │     └─ui_poetry_view.c/.h  #   纯渲染（home/list/detail）
           ├─games/                     # 游戏类应用（独立 Screen / 高刷新）
           │  ├─airplane/  # 飞机大战：flygame.c/.h + 敌机/Boss/爆炸 等美术 .c
           │  └─pvz/       # 植物大战僵尸：pvz.c/.h + 豌豆/坚果/阳光/僵尸/地图 等美术 .c
           └─system/                    # 系统类应用
               ├─calculator/ ui_calculator.c/.h
               ├─calendar/   ui_calendar.c/.h     # 基于 lv_calendar，启用农历
               ├─clock/      ui_clock.c/.h        # 时钟仪表盘（翻页时钟）
               │             ui_alarm_create.c/.h #   新增/编辑闹钟 + 铃声页 + 拼音输入
               ├─reminder/   ui_reminder.c/.h
               ├─settings/   ui_settings.c/.h
               │             lvgl_9_5_wi_fi.cpp   #   WiFi 子页（C++，注意 extern "C" 边界）
               └─weather/    ui_weather.c/.h      # 含定位选择对话框
```

## 3. 分层与依赖规则 (Layering & Dependency Rules)
依赖方向 **单向向下**，严禁反向或跨层穿透：
```text
main.c / freertos_main.c   （入口，只认 hal + ui_init）
        │
        ▼
   hal/  ◄──────── 硬件差异全部收敛于此（SDL2 ↔ ESP32-S3 SPI/触摸/tick 仅换此层）
        │
        ▼
   ui/ui.c (Launcher)  ── 持有桌面、Dock、status_bar；通过 strcmp(name) 分发到各页面 ui_<app>_show()
        │                 ── AI 类应用经 app_launch_callback_t 回调交由硬件端实现
        ▼
   ui/pages/<类别>/<app>/   ── 各页面互相独立，跨页跳转只调对方 ui_<other>_show()
        │
        ▼
   ui/components/ (titlebar/statusbar) + ui_config.h   ── 页面复用，组件不反向依赖具体页面
```
- **Launcher 是唯一的页面调度中心**：新增应用 = 在 `ui.c` 的 `icon_names[]` / `ios_icons[]` 注册 + 分发分支里加一条 `ui_<app>_show()`。
- **页面之间零耦合**：A 页面不得 `#include` B 页面内部、不得直接动 B 的静态变量；需要跳转就调 `ui_<b>_show()`。
- **组件层只被依赖、不依赖页面**：`components/` 与 `ui_config.h` 是叶子，禁止反向 `#include pages/...`。
- **硬件调用收敛 `hal/`**：GPIO/SPI/WiFi/音频/RTC 等只能出现在 `hal/`，`ui/` 层通过接口或回调访问。

## 4. 关键数据流 (Key Data Flows)
- **APP 启动**：桌面图标点击 → `ui.c` 比对 `name` → 命中本地页面则 `ui_<app>_show()`；命中 AI 则 `app_launch_cb("ai_assistant")` 上抛硬件层（回调由 `ui_set_app_launch_callback()` 注入）。
- **页面浮层**：系统页面 `lv_obj_create(lv_layer_top())` 叠在桌面上 → `hide()` 删除浮层即露出桌面（详见 CONSTRAINTS.md §6/§7 浮层与状态栏协作）。
- **尺寸适配**：所有页面向 `ui_config.h` 的 `ui_get_*()` 取尺寸 → 换屏只改该文件，UI 代码不动。
