#include "ui_statusbar.h"
#include <string.h>

// 前向声明
extern void ui_settings_wifi_show(void);
extern void ui_settings_show(void);
extern void ui_lockscreen_show(void);

lv_obj_t * bar_time_label = NULL;
lv_obj_t * status_bar = NULL;
lv_obj_t * bar_right_icons = NULL;

static lv_obj_t * quick_panel = NULL;
static bool panel_visible = false;
static int current_brightness = 80;
static bool btn_states[4] = {true, false, false, false};  // WiFi, 4G, 电量, 计算器
static lv_obj_t * wifi_btn = NULL;  // 保存WiFi快捷面板按钮引用
// 状态栏图标
static lv_obj_t * status_icon_wifi = NULL;
static lv_obj_t * status_icon_4g = NULL;

static void update_status_bar_icons(void);


static void toggle_quick_panel_by_bg(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED && panel_visible) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_var(&a, quick_panel);
        lv_anim_set_values(&a, 0, -180);
        lv_anim_set_duration(&a, 200);
        lv_anim_start(&a);
        panel_visible = false;
    }
}

static void wifi_btn_long_press_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_LONG_PRESSED) {
        // 关闭快捷面板
        if (panel_visible) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_var(&a, quick_panel);
            lv_anim_set_values(&a, 0, -180);
            lv_anim_set_duration(&a, 200);
            lv_anim_start(&a);
            panel_visible = false;
        }
        // 跳转到WiFi设置页面
        ui_settings_wifi_show();
    }
}

// 快捷面板第三个按钮：锁屏（不切换颜色）
static void lockscreen_btn_click_cb(lv_event_t * e) {
    (void)e;
    // 收起快捷面板
    if (panel_visible) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_var(&a, quick_panel);
        lv_anim_set_values(&a, 0, -180);
        lv_anim_set_duration(&a, 200);
        lv_anim_start(&a);
        panel_visible = false;
    }
    ui_lockscreen_show();
}

static void toggle_btn_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    int id = (int)(intptr_t)lv_obj_get_user_data(btn);
    // 跳过第3个锁屏按钮和第4个设置按钮（不切换颜色）
    if (id == 2 || id == 3) return;
    // WiFi(0) 和 4G(1) 互斥
    if (id == 0 || id == 1) {
        int other = (id == 0) ? 1 : 0;
        // 如果当前已经是打开状态，点击则关闭自身（同时开启对方）
        if (btn_states[id]) {
            btn_states[id] = false;
            btn_states[other] = false;  // 都关闭
        } else {
            btn_states[id] = true;
            btn_states[other] = false;
        }
        // 更新另一个按钮的颜色
        lv_obj_t * parent = lv_obj_get_parent(btn);
        lv_obj_t * other_btn = lv_obj_get_child(parent, other);
        lv_obj_set_style_bg_color(other_btn,
            lv_color_hex(btn_states[other] ? 0x007AFF : 0x3A3A3C), 0);
    } else {
        btn_states[id] = !btn_states[id];
    }
    lv_color_t color = lv_color_hex(btn_states[id] ? 0x007AFF : 0x3A3A3C);
    lv_obj_set_style_bg_color(btn, color, 0);
    // 同步状态栏图标颜色
    update_status_bar_icons();
}

static void settings_btn_click_cb(lv_event_t * e) {
    (void)e;
    // 关闭快捷面板
    if (panel_visible) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_var(&a, quick_panel);
        lv_anim_set_values(&a, 0, -180);
        lv_anim_set_duration(&a, 200);
        lv_anim_start(&a);
        panel_visible = false;
    }
    // 跳转到系统设置页面
    ui_settings_show();
}

static void brightness_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    current_brightness = lv_slider_get_value(slider);
    // 更新百分比标签
    lv_obj_t * lbl = (lv_obj_t *)lv_event_get_user_data(e);
    if (lbl != NULL) {
        lv_label_set_text_fmt(lbl, "%d%%", current_brightness);
    }
}

// 控制中心
static void create_quick_panel(void) {
    quick_panel = lv_obj_create(lv_layer_top());
    lv_obj_set_size(quick_panel, UI_SCREEN_WIDTH, 180);
    lv_obj_set_pos(quick_panel, 0, -180);
    lv_obj_set_style_bg_color(quick_panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(quick_panel, LV_OPA_40, 0);  // 40%透明度
    lv_obj_set_scrollbar_mode(quick_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_blur_backdrop(quick_panel, true, 0);  // 启用背景模糊
    lv_obj_set_style_blur_radius(quick_panel, 12, 0);      // 模糊半径
    lv_obj_set_style_blur_quality(quick_panel, LV_BLUR_QUALITY_AUTO, 0);
    lv_obj_clear_flag(quick_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(quick_panel, toggle_quick_panel_by_bg, LV_EVENT_CLICKED, NULL);

    // 把手
    lv_obj_t * handle = lv_obj_create(quick_panel);
    lv_obj_set_size(handle, 36, 5);
    lv_obj_set_style_bg_color(handle, lv_color_hex(0x555555), 0);
    lv_obj_set_style_radius(handle, 3, 0);
    lv_obj_align(handle, LV_ALIGN_BOTTOM_MID, 0, 8);

    // 第1行：功能按钮
    lv_obj_t * row1 = lv_obj_create(quick_panel);
    lv_obj_set_size(row1, lv_pct(100), 64);
    lv_obj_set_pos(row1, 0, 22);
    lv_obj_set_style_bg_opa(row1, 0, 0);
    lv_obj_set_style_border_width(row1, 0, 0);
    lv_obj_set_scrollbar_mode(row1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 按钮配置
    static const char * icons[4] = {LV_SYMBOL_WIFI, "4G", LV_SYMBOL_POWER, LV_SYMBOL_SETTINGS};
    static const int font_sizes[4] = {24, 20, 24, 24};

    for (int i = 0; i < 4; i++) {
        lv_obj_t * btn = lv_obj_create(row1);
        lv_obj_set_size(btn, 55, 55);
        lv_obj_set_style_bg_color(btn, btn_states[i] ? lv_color_hex(0x007AFF) : lv_color_hex(0x3A3A3C), 0);
        lv_obj_set_style_radius(btn, 30, 0);
        lv_obj_set_style_bg_opa(btn, 180, 0);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        lv_obj_add_event_cb(btn, toggle_btn_cb, LV_EVENT_CLICKED, NULL);

        // WiFi按钮添加长按事件
        if (i == 0) {
            wifi_btn = btn;
            lv_obj_add_event_cb(btn, wifi_btn_long_press_cb, LV_EVENT_LONG_PRESSED, NULL);
        }

        // 第三个按钮（索引2）改为锁屏：点击显示锁屏UI + 收起面板，不切换颜色
        if (i == 2) {
            lv_obj_add_event_cb(btn, lockscreen_btn_click_cb, LV_EVENT_CLICKED, NULL);
            // 恢复原始颜色，不参与切换
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x3A3A3C), 0);
        }

        // 第4个设置按钮：跳转到系统设置页面（收起面板）
        if (i == 3) {
            lv_obj_add_event_cb(btn, settings_btn_click_cb, LV_EVENT_CLICKED, NULL);
        }

        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text(lbl, icons[i]);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
    }

    // 第 2 行：亮度滑块
    lv_obj_t * bright_bar = lv_obj_create(quick_panel);
    lv_obj_set_size(bright_bar, lv_pct(100), 50);
    lv_obj_set_pos(bright_bar, 0, 88);
    lv_obj_set_style_bg_opa(bright_bar, 0, 0);
    lv_obj_set_style_border_width(bright_bar, 0, 0);
    lv_obj_set_style_bg_color(bright_bar, lv_color_hex(0x3A3A3C), 0);
    lv_obj_set_style_radius(bright_bar, 30, 0);
    lv_obj_set_style_bg_opa(bright_bar, 180, 0);
    lv_obj_set_style_pad_hor(bright_bar, 20, 0);
    lv_obj_set_style_pad_ver(bright_bar, 12, 0);
    lv_obj_set_scrollbar_mode(bright_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(bright_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(bright_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bright_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * bright_slider = lv_slider_create(bright_bar);
    lv_obj_set_size(bright_slider, lv_pct(88), 6);
    lv_obj_set_style_bg_color(bright_slider, lv_color_hex(0x555555), 0);
    lv_obj_set_style_bg_color(bright_slider, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bright_slider, 3, 0);
    lv_slider_set_range(bright_slider, 0, 100);
    lv_slider_set_value(bright_slider, current_brightness, LV_ANIM_OFF);

    // 亮度百分比标签
    lv_obj_t * bright_lbl = lv_label_create(bright_bar);
    lv_label_set_text_fmt(bright_lbl, "%d%%", current_brightness);
    lv_obj_set_style_text_font(bright_lbl, &font, 0);
    lv_obj_set_style_text_color(bright_lbl, lv_color_white(), 0);
    lv_obj_set_style_text_align(bright_lbl, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(bright_lbl, 40);

    lv_obj_add_event_cb(bright_slider, brightness_cb, LV_EVENT_VALUE_CHANGED, bright_lbl);
}

// 显示/隐藏快捷面板
static void toggle_quick_panel(void) {
    if (quick_panel == NULL) {
        create_quick_panel();
    }

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_var(&a, quick_panel);
    lv_anim_set_values(&a, panel_visible ? 0 : -180, panel_visible ? -180 : 0);
    lv_anim_set_duration(&a, 200);
    lv_anim_start(&a);
    panel_visible = !panel_visible;
}

// 状态栏点击事件
static void status_bar_click_cb(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        toggle_quick_panel();
    }
}

// 公共 API 实现

void ui_statusbar_create(void) {
    status_bar = lv_obj_create(lv_layer_top());
    lv_obj_set_size(status_bar, UI_SCREEN_WIDTH, UI_STATUSBAR_HEIGHT);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_30, 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);  // 无圆角
    lv_obj_set_scrollbar_mode(status_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(status_bar, LV_DIR_NONE);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(status_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(status_bar, status_bar_click_cb, LV_EVENT_CLICKED, NULL);

    bar_time_label = lv_label_create(status_bar);
    lv_label_set_text(bar_time_label, "10:24");
    lv_obj_set_style_text_font(bar_time_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(bar_time_label, lv_color_white(), 0);
    lv_obj_align(bar_time_label, LV_ALIGN_LEFT_MID, 0, 0);

    bar_right_icons = lv_obj_create(status_bar);
    lv_obj_set_size(bar_right_icons, lv_pct(60), UI_STATUSBAR_HEIGHT - 4);
    lv_obj_set_style_bg_opa(bar_right_icons, 0, 0);
    lv_obj_set_style_border_width(bar_right_icons, 0, 0);
    lv_obj_set_scrollbar_mode(bar_right_icons, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(bar_right_icons, LV_DIR_NONE);  // 禁止滚动
    lv_obj_clear_flag(bar_right_icons, LV_OBJ_FLAG_SCROLLABLE);  // 清除可滚动标志
    lv_obj_align(bar_right_icons, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_flex_flow(bar_right_icons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar_right_icons, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(bar_right_icons, 6, 0);

    lv_obj_t * icon_alarm = lv_label_create(bar_right_icons);
    lv_label_set_text(icon_alarm, LV_SYMBOL_BELL);
    lv_obj_set_style_text_font(icon_alarm, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(icon_alarm, lv_color_white(), 0);

    lv_obj_t * icon_bt = lv_label_create(bar_right_icons);
    lv_label_set_text(icon_bt, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_font(icon_bt, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(icon_bt, lv_color_white(), 0);

    lv_obj_t * icon_4g = lv_label_create(bar_right_icons);
    lv_label_set_text(icon_4g, "4G");
    lv_obj_set_style_text_font(icon_4g, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(icon_4g, lv_color_white(), 0);
    lv_obj_set_style_opa(icon_4g, LV_OPA_0, 0);  // 默认关闭，隐藏
    lv_obj_set_style_margin_right(icon_4g, 3, 0);
    status_icon_4g = icon_4g;

    lv_obj_t * icon_wifi = lv_label_create(bar_right_icons);
    lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(icon_wifi, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(icon_wifi, lv_color_white(), 0);  // 默认打开，白色
    status_icon_wifi = icon_wifi;

    lv_obj_t * icon_bat = lv_label_create(bar_right_icons);
    lv_label_set_text(icon_bat, LV_SYMBOL_BATTERY_3);
    lv_obj_set_style_text_font(icon_bat, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(icon_bat, lv_color_white(), 0);

    lv_obj_t * bat_percent = lv_label_create(bar_right_icons);
    lv_label_set_text(bat_percent, "85%");
    lv_obj_set_style_text_font(bat_percent, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(bat_percent, lv_color_white(), 0);
    update_status_bar_color(lv_color_hex(0x1a3a6d));
}

void update_status_bar_icons(void) {
    if (status_icon_wifi) {
        lv_obj_set_style_text_color(status_icon_wifi,
            btn_states[0] ? lv_color_white() : lv_color_hex(0x3A3A3C), 0);
        lv_obj_set_style_opa(status_icon_wifi, btn_states[0] ? LV_OPA_100 : LV_OPA_0, 0);
    }
    if (status_icon_4g) {
        lv_obj_set_style_text_color(status_icon_4g,
            btn_states[1] ? lv_color_white() : lv_color_hex(0x3A3A3C), 0);
        lv_obj_set_style_opa(status_icon_4g, btn_states[1] ? LV_OPA_100 : LV_OPA_0, 0);
    }
}

void update_status_bar_color(lv_color_t bg_color) {
    if(status_bar == NULL) return;
    lv_color_t text_color =  lv_color_white();
    if(bar_time_label) lv_obj_set_style_text_color(bar_time_label, text_color, 0);
    if(bar_right_icons) {
        lv_obj_t * child;
        for(int i = 0; (child = lv_obj_get_child(bar_right_icons, i)) != NULL; i++) {
            lv_obj_set_style_text_color(child, text_color, 0);
        }
    }
}
