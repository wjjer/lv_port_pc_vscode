# Strict Coding & API Constraints (CONSTRAINTS.md)

## 1. LVGL v9.5 语法强约束 (全面禁止 8.x 旧语法)
为了确保代码在 LVGL 9.5 编译器下顺利通过，**必须死守以下新语法红线**：
- ❌ **严禁使用** `lv_scr_act()`
  👉 **必须使用** `lv_screen_active()`
- ❌ **严禁使用** `lv_obj_align_to()`
  👉 **必须使用** `lv_obj_align()` 并通过计算坐标或使用父容器布局（如 Flex/Grid）来实现相对对齐。
- ❌ **严禁使用** `lv_obj_set_style_local_...`
  👉 **必须使用** 标准的 `lv_style_t` 样式表或 `lv_obj_set_style_<property>()` 属性设置函数。
- ❌ **严禁使用** `lv_event_get_user_data(e)`
  👉 **必须严格遵循** 9.5 规范：如果回调中需要传递用户数据，使用 `lv_event_get_user_data(e)` 承接自定义指针，但严禁混淆事件发生的目标对象本身（始终用 `lv_event_get_target(e)` 获取触发组件）。

## 2. 视觉主题与色彩规范 (iOS 极简伴学机风格)
AI Agent 在为其构建新组件或新页面时，必须强制继承并遵循以下配色与弧度基准：
- 🎨 **背景基色 (COLOR_BG)**: `lv_color_hex(0xF5F5F7)` (清爽浅灰) 或 `lv_color_hex(0x3B82F6)` (Launcher 明亮蓝)
- 🎨 **卡片/列表项表面色 (COLOR_SURFACE)**: `lv_color_hex(0xFAFAFC)` 或 `lv_color_hex(0x1E293B)`
- 🎨 **主文字颜色 (COLOR_TEXT_MAIN)**: `lv_color_hex(0x111111)` (高对比黑)
- 🎨 **高亮/操作焦点色 (COLOR_ACCENT)**: `lv_color_hex(0x007AFF)` (标准 iOS 蓝)
- 🎨 **系统状态色**: 成功/使能用绿色 `lv_color_hex(0x34C759)`；警示/删除用红色 `lv_color_hex(0xFF3B30)`
- 📐 **圆角基准**:
  - 系统桌面图标圆角 (`IOS_ICON_RADIUS`): 固定为 `12`
  - 列表项/普通卡片圆角 (`UI_RADIUS`): 固定为 `12`
  - 底部操作按钮/功能按钮圆角: 固定为 `10`

## 3. 目录规范与多文件拆分命名空间
- **单文件代码量控制**: 严禁将一个复杂应用的所有子界面都堆死在一个 `.c` 文件里。必须按照页面和功能进行垂直拆分。
- **命名规范**: 遵循 `ui_<app_name>_<sub_page>.c` 格式。
  * 例如时钟应用 (Clock)：
    - 主界面: `src/ui/pages/system/clock/ui_clock.c`
    - 添加闹钟界面: `src/ui/pages/system/clock/ui_clock_create.c`
- **命名前缀隔离**: 某个应用特有的所有全局变量、函数、全局组件，必须统一加上 `ui_<app_name>_` 前缀，防止在最终固件链接时发生符号冲突（Symbol Collision）。

## 4. SPI 低带宽屏幕性能红线
- **减少无效重绘**: 真实硬件是 320x240 的 SPI 屏，总线带宽有限。AI Agent 建立布局时，应尽量减少不必要的容器嵌套。
- **避免过度透明覆盖**: 尽量保持对象不透明（`LV_OPA_COVER`）。避免使用大量全屏大面积的半透明（如 `LV_OPA_50`）图层叠加，否则会导致 ESP32-S3 刷新率暴跌、画面撕裂。

---

## 5. 页面生命周期契约 (Page Lifecycle Contract)
每个页面 **必须且只** 对外暴露一对生命周期函数，签名固定，由 Launcher (`ui.c`) 的分发逻辑调用：
```c
void ui_<app>_show(void);   // 创建并显示页面
void ui_<app>_hide(void);   // 销毁页面、释放所有 LVGL 对象、复位静态状态
```
- ✅ **页面句柄用 static 持有**: 每个页面文件用一个 `static lv_obj_t * <app>_page;` 持有根容器，`show()` 创建、`hide()` 销毁。
- ✅ **`hide()` 必须幂等且安全**: 销毁前判空/判有效，销毁后立即置 `NULL`，防止重复调用或返回桌面后野指针。二选一统一写法：
  ```c
  // 写法 A（推荐，最稳）
  if(lv_obj_is_valid(<app>_page)) { lv_obj_delete(<app>_page); <app>_page = NULL; }
  // 写法 B（已有页面在用，可接受）
  if(<app>_page != NULL) { lv_obj_delete(<app>_page); <app>_page = NULL; }
  ```
- ✅ **`show()` 入口先自清理**: 如 `ui_poetry_show()` 先调 `ui_poetry_hide()` 再重建，避免重复进入残留两份页面。
- ❌ **严禁** 在页面文件里直接操作 Launcher 桌面对象或别的页面的静态变量；跨页面跳转一律通过对方的 `ui_<other>_show()` 进入。

## 6. 屏幕承载方式：统一使用 `lv_layer_top()` 浮层 (Screen Strategy)
本项目桌面 (Launcher) 常驻在活动屏幕上，**系统/工具类页面一律作为浮层叠加，而非切换 Screen**：
- 👉 **标准做法**: `<app>_page = lv_obj_create(lv_layer_top());` 然后铺满全屏。返回时 `lv_obj_delete` 即可露出底下的桌面，无需重新加载桌面 Screen。当前 calculator / calendar / clock / alarm_create / reminder / settings / weather 均采用此模式，新页面 **必须** 沿用。
- ⚠️ **独立 Screen 模式（仅特例）**: `lv_obj_create(NULL)` + `lv_screen_load()` 会整屏接管、盖掉全局状态栏（poetry 当前如此）。仅当页面要完全独占屏幕（如游戏、沉浸式阅读）时才允许；采用此模式的页面 **自行负责** 退出时 `lv_screen_load` 回到桌面 Screen。
- ❌ **严禁** 混用：不要在一个 `lv_layer_top()` 浮层页面里再去 `lv_screen_load` 切换底层 Screen。

## 7. 全局状态栏协作规则 (Status Bar Coordination)
全局状态栏 `status_bar`（`extern`，由 `ui_statusbar_create()` 创建）位于桌面之上。浮层页面盖上来后必须显式表态，二选一，且 **必须在 `hide()` 里精确还原**：
- **要保留状态栏**（页面顶部留出状态栏高度）：`show()` 中 `if(status_bar) lv_obj_move_foreground(status_bar);` 把它顶到浮层之上。`hide()` 无需特殊处理（页面销毁后它自然可见）。—— reminder / settings / clock / alarm_create 用此模式。
- **要全屏隐藏状态栏**：`show()` 中 `if(status_bar) lv_obj_add_flag(status_bar, LV_OBJ_FLAG_HIDDEN);`，并且 `hide()` 中 **务必** `if(status_bar) lv_obj_remove_flag(status_bar, LV_OBJ_FLAG_HIDDEN);` 还原。—— calculator / calendar / weather 用此模式。
- 📐 **取消隐藏统一用 `lv_obj_remove_flag`**（9.5 标准）；`lv_obj_clear_flag` 是 8.x 兼容别名，新代码 **不要再用**（calculator/calendar 历史代码里的 `clear_flag` 后续应替换为 `remove_flag`）。
- ❌ **严禁** “只隐藏不还原”：返回桌面后状态栏丢失就是典型回归 Bug。

## 8. 字体使用红线 (Font Usage)
项目存在三类字体，混用会导致中文显示成方块或丢字：
- 🈶 **任何含中文的文本** → **必须** 用项目中文字库 `&font`（`extern const lv_font_t font;`，已在 `ui.h`/各组件头声明）。设置：`lv_obj_set_style_text_font(obj, &font, 0);`
- 🔢 **纯数字 / 拉丁字母 / LVGL 符号 (`LV_SYMBOL_*`)** → 可用内置 `&lv_font_montserrat_16`（及其它字号），体积小、清晰。
- 📅 **日历控件内 CJK** → 使用 `&lv_font_source_han_sans_sc_14_cjk`（`ui_calendar.c` 已用，勿改）。
- ❌ **严禁** 给中文标签设 `montserrat`（只含 ASCII，中文会变 `□`）。拿不准就用 `&font`。

## 9. 图像与事件 API 红线（补充 9.5 校验）
- 🖼 **图片声明/创建必须用 9.5 新名**:
  - ❌ `LV_IMG_DECLARE` / `lv_img_create` / `lv_img_set_src`（8.x 旧别名）
  - 👉 `LV_IMAGE_DECLARE` / `lv_image_create` / `lv_image_set_src`
  - ⚠️ **已知技术债**: `src/ui/pages/games/airplane/flygame.c` 仍在使用 `LV_IMG_DECLARE` / `lv_img_create`，属待清理项，新代码严禁照抄；改动该文件时应顺手迁移。
- 🎯 **事件回调取值** (重申 9.5)：触发对象用 `lv_event_get_target(e)`，自定义指针用 `lv_event_get_user_data(e)`，事件码用 `lv_event_get_code(e)`。严禁 8.x 的 `lv_event_get_target_obj` 误用或直接读 `e->`。
- 🧩 **复用全局组件**: 页面顶部统一用 `ui_titlebar_create(...)` 生成标题栏（含返回箭头+标题+可选右键），不要每个页面各画一套。其完整签名见 `components/ui_titlebar.h`，`bg_color`/`text_color` 传 `NULL` 即自动适配。

## 10. 命名与符号隔离 (Naming & Symbol Isolation)
- **生命周期**: `ui_<app>_show` / `ui_<app>_hide`（游戏类历史命名为 `flygame_*` / `pvz_*`，新应用一律走 `ui_<app>_` 规范）。
- **所有非 static 的全局符号**（函数、变量、`LV_IMAGE_DECLARE` 的图片名）必须带 `ui_<app>_` 或应用专属前缀，防止固件链接期符号冲突。
- **能 static 就 static**: 页面内部辅助函数、回调、状态变量一律 `static`，只把 `show`/`hide` 暴露到头文件。
- **头文件极简**: `ui_<app>.h` 只声明对外的 `show`/`hide` 和必要的 `extern`，不外泄内部实现细节。

## 11. 入口与移植边界 (Entry & Portability Boundary)
- **模拟器启动链路**（`src/main.c`）: `lv_init()` → `sdl_hal_init(320, 240)` → `ui_init()` → 主循环 `lv_timer_handler()`。新页面只接入 `ui_init()` 构建的 Launcher，**严禁** 在 `main.c` 里直插页面逻辑。
- **HAL 是唯一硬件差异点**: 模拟器用 `sdl_hal_init()`（SDL2 显示+鼠标输入+tick）；移植到 ESP32-S3 时 **只替换 `hal/` 实现**（换成 SPI 屏驱动 + 电容触摸 + esp_timer tick），`ui/` 层一行不改。任何硬件相关调用（GPIO/SPI/WiFi/音频）都必须收敛进 `hal/`，禁止散落到 `ui/`。
- **`ui_config.h` 是分辨率适配的唯一来源**: 所有尺寸（栏高、图标、间距、圆角、内容区）走其中的 `ui_get_*()` 动态函数，**严禁** 在页面里硬编码 320/240 等魔法数；屏幕换尺寸时只改 `ui_config.h`。
