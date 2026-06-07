#include "ui_settings.h"
#include "ui_settings_wifi.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"


#define COLOR_SETTING_BG     lv_color_hex(0xF2F2F7) // 标准系统浅灰底
#define COLOR_ITEM_BG        lv_color_white()       // 卡片纯白背景
#define COLOR_TEXT_MAIN      lv_color_hex(0x000000) // 主标题纯黑
#define COLOR_TEXT_MUTED     lv_color_hex(0x8E8E93) // 右侧说明文本/分组标题灰色
#define COLOR_LINE           lv_color_hex(0xE5E5EA) // 1px 极细纳米分割线
#define COLOR_IOS_GREEN      lv_color_hex(0x34C759) // iOS 标准安全绿

static lv_obj_t * settings_page = NULL;
static bool settings_visible = false;

/**
 * @brief 核心像素级对齐函数：构建无原生污染的 iOS 风格卡片条目
 */
static lv_obj_t * add_menu_item(lv_obj_t * parent, const char * icon, const char * name, const char * value, bool has_switch) {
    // 1. 创建基础容器，彻底摆脱原生按钮的 padding 束缚
    lv_obj_t * item = lv_obj_create(parent);
    lv_obj_set_size(item, LV_PCT(100), 38);
    lv_obj_set_style_bg_color(item, COLOR_ITEM_BG, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_radius(item, 0, 0);
    lv_obj_set_style_pad_all(item, 0, 0);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 左侧图标：科技蓝，绝对垂直居中
    lv_obj_t * icon_label = lv_label_create(item);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_color(icon_label, lv_color_hex(0x007AFF), 0);
    lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 12, 0);

    // 3. 核心文本：紧跟图标，使用中文字体，绝对垂直居中
    lv_obj_t * name_label = lv_label_create(item);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_color(name_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(name_label, &font, 0);
    lv_obj_align(name_label, LV_ALIGN_LEFT_MID, 36, 0);

    // 4. 右侧附件区域对齐优化
    if(has_switch) {
        lv_obj_t * sw = lv_switch_create(item);
        lv_obj_set_size(sw, 32, 18);
        lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -12, 0);
        lv_obj_set_style_bg_color(sw, COLOR_IOS_GREEN, LV_PART_INDICATOR | LV_STATE_CHECKED);
    } else if(value != NULL) {
        lv_obj_t * val_label = lv_label_create(item);
        lv_label_set_text(val_label, value);
        lv_obj_set_style_text_color(val_label, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(val_label, &font, 0);
        lv_obj_align(val_label, LV_ALIGN_RIGHT_MID, -12, 0);
    }

    // 5. 视觉点睛：容器底部植入一条 1px 极细下划线
    lv_obj_t * line = lv_obj_create(item);
    lv_obj_set_size(line, LV_PCT(100), 1);
    lv_obj_align(line, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(line, COLOR_LINE, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_add_flag(line, LV_OBJ_FLAG_IGNORE_LAYOUT);

    return item;
}

/**
 * @brief 创建可点击的菜单项（支持回调）
 */
static lv_obj_t * add_menu_item_with_callback(lv_obj_t * parent, const char * icon, const char * name, const char * value, lv_event_cb_t callback) {
    lv_obj_t * item = add_menu_item(parent, icon, name, value, false);

    // 设置为可点击
    lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);

    // 添加点击事件
    if(callback != NULL) {
        lv_obj_add_event_cb(item, callback, LV_EVENT_CLICKED, NULL);
    }

    return item;
}



static void roller_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * roller = lv_event_get_target(e);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        // 开始滑动：增加可见行数，让选项浮现
        lv_roller_set_visible_row_count(roller, 3);
        // 恢复所有文字透明度
        lv_obj_set_style_text_opa(roller, LV_OPA_COVER, LV_PART_MAIN);
    }
    else if(code == LV_EVENT_SCROLL_END || code == LV_EVENT_VALUE_CHANGED) {
        // 停止滑动：缩回一行
        lv_roller_set_visible_row_count(roller, 1);
        // 仅显示选中项（设为透明）
        lv_obj_set_style_text_opa(roller, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_text_opa(roller, LV_OPA_COVER, LV_PART_SELECTED);
    }
}

static lv_obj_t * add_menu_roller_item(lv_obj_t * parent, const char * icon, const char * name, const char * options, uint16_t default_idx) {

    // 1. 创建基础容器
    lv_obj_t * item = lv_obj_create(parent);
    lv_obj_set_size(item, LV_PCT(100), 38); // 如果你改回了 38，这里就是 38。建议保持 45-56 之间给手指留一点滑动空间
    lv_obj_set_style_bg_color(item, COLOR_ITEM_BG, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_radius(item, 0, 0);
    lv_obj_set_style_pad_all(item, 0, 0);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

    // ✅ 强制启用边界裁切，任何超出卡片边缘（包括圆角）的子对象都会被切断
    lv_obj_set_style_clip_corner(item, true, 0);
    // 2. 左侧图标
    lv_obj_t * icon_label = lv_label_create(item);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_color(icon_label, lv_color_hex(0x007AFF), 0);
    lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 12, 0);

    // 3. 核心文本
    lv_obj_t * name_label = lv_label_create(item);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_color(name_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(name_label, &font, 0);
    lv_obj_align(name_label, LV_ALIGN_LEFT_MID, 36, 0);

   // 4. 右侧滚轮设置
    lv_obj_t * roller = lv_roller_create(item);
    lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(roller, default_idx, LV_ANIM_OFF);
    lv_obj_remove_style_all(roller);

    // 绑定事件
    lv_obj_add_event_cb(roller, roller_event_handler, LV_EVENT_ALL, NULL);

    // 设置初始透明度：仅显示选中项
    lv_obj_set_style_text_opa(roller, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_text_opa(roller, LV_OPA_COVER, LV_PART_SELECTED);

    // 样式微调
    lv_obj_set_style_bg_opa(roller, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(roller, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(roller, &font, LV_PART_MAIN);
    lv_obj_set_style_text_color(roller, lv_color_hex(0x007AFF), LV_PART_SELECTED);
    lv_obj_set_style_text_align(roller, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    // 强制滚轮宽度，避免被拉伸
    lv_obj_set_width(roller, 80);
    lv_obj_align(roller, LV_ALIGN_RIGHT_MID, -12, 0);
    // 5. 下划线
    lv_obj_t * line = lv_obj_create(item);
    lv_obj_set_size(line, LV_PCT(100), 1);
    lv_obj_align(line, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(line, COLOR_LINE, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_add_flag(line, LV_OBJ_FLAG_IGNORE_LAYOUT);

    return item;
}

/**
 * @brief WiFi菜单项点击回调
 */
static void wifi_item_clicked(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        ui_settings_wifi_show();
    }
}

static void add_menu_section_title(lv_obj_t * parent, const char * text) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, text);
    lv_obj_set_style_text_color(title, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(title, &font, 0);
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_margin_top(title, 10, 0);
    lv_obj_set_style_margin_bottom(title, 4, 0);
    lv_obj_set_style_margin_left(title, 4, 0);
}
/**
 * @brief 页面初始化定义
 */
void ui_settings_init(void) {
    // 1. 主页面顶层容器
    settings_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(settings_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_x(settings_page, UI_SCREEN_WIDTH);
    lv_obj_set_style_bg_color(settings_page, COLOR_SETTING_BG, 0);
    lv_obj_set_style_pad_all(settings_page, 0, 0);
    lv_obj_set_style_border_width(settings_page, 0, 0);
    lv_obj_add_flag(settings_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(settings_page, LV_OBJ_FLAG_SCROLLABLE);


     if(status_bar) lv_obj_move_foreground(status_bar);

    // 2. 顶部通用导航栏
    ui_titlebar_create(settings_page, "设置", (lv_event_cb_t)ui_settings_hide, NULL, NULL, NULL, NULL, NULL, NULL, false);

    // 3. 使用规范级基础容器 + Flex 布局搭建滚动区
    lv_obj_t * main_list = lv_obj_create(settings_page);
    lv_obj_set_size(main_list, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(main_list, 0, ui_get_content_y());
    lv_obj_set_style_bg_opa(main_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_list, 0, 0);
    lv_obj_set_style_pad_hor(main_list, 14, 0);
    lv_obj_set_style_pad_ver(main_list, 4, 0);

    lv_obj_set_flex_flow(main_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(main_list, 0, 0);

    // --- 分组 1: 账户 ---
    add_menu_section_title(main_list, "账户管理");
    lv_obj_t * item1 = add_menu_item(main_list, LV_SYMBOL_IMAGE, "当前用户", "123456 >", false);
    lv_obj_set_style_radius(item1, 8, 0);

    // --- 分组 2: 网络 ---
    add_menu_section_title(main_list, "网络与无线连接");
    lv_obj_t * item2_1 = add_menu_item_with_callback(main_list, LV_SYMBOL_WIFI, "无线网络", "微赞办公室", wifi_item_clicked);
    lv_obj_set_style_radius(item2_1, 8, 0);

    lv_obj_t * item2_2 = add_menu_item(main_list, LV_SYMBOL_BLUETOOTH, "蓝牙", NULL, true);
    lv_obj_set_style_radius(item2_2, 8, 0);

    lv_obj_t * item2_3 = add_menu_item(main_list, LV_SYMBOL_UPLOAD, "4G网络", NULL, true);
    lv_obj_set_style_radius(item2_3, 8, 0);
    // 隐藏本组最后一项的下划线
    if(lv_obj_get_child_cnt(item2_3) >= 4) {
        lv_obj_set_style_bg_opa(lv_obj_get_child(item2_3, 3), LV_OPA_TRANSP, 0);
    }

    // --- 分组 3: 闹钟设置 ---
    add_menu_section_title(main_list, "时钟设置");
// 参数说明：(父对象, 图标, 标题, 滚轮选项, 默认选中索引)
    lv_obj_t * item3_1 = add_menu_roller_item(main_list, LV_SYMBOL_SETTINGS, "贪睡时间", "5分钟\n10分钟\n15分钟\n30分钟", 1); // 默认选 10分钟
    lv_obj_set_style_radius(item3_1, 8, 0);

    lv_obj_t * item3_2 = add_menu_roller_item(main_list, LV_SYMBOL_SETTINGS, "响铃时间", "1分钟\n3分钟\n5分钟", 0); // 默认选 1分钟
    lv_obj_set_style_radius(item3_2, 8, 0);

    lv_obj_t * item3_3 = add_menu_roller_item(main_list, LV_SYMBOL_SETTINGS, "重复次数", "1次\n2次\n3次\n5次", 2); // 默认选 3次
    lv_obj_set_style_radius(item3_3, 8, 0);

    // --- 分组 5: 系统 ---
    add_menu_section_title(main_list, "系统信息");
    lv_obj_t * item4 = add_menu_item(main_list, LV_SYMBOL_SETTINGS, "系统版本", "v1.0.0", false);
    lv_obj_set_style_radius(item4, 8, 0);
}

/**
 * @brief 隐藏动画结束后的静态内部回调
 */
static void hide_anim_ready_cb(lv_anim_t * a) {
    settings_visible = false;
    lv_obj_add_flag(settings_page, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief 导出全局可见：显示设置页面动画（无 static）
 */
void ui_settings_show(void) {
    if(settings_page == NULL) {
        ui_settings_init();
    }
    if(lv_obj_has_flag(settings_page, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(settings_page, LV_OBJ_FLAG_HIDDEN);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, settings_page);
        lv_anim_set_values(&a, UI_SCREEN_WIDTH, 0);
        lv_anim_set_time(&a, 250);                         // 250ms，契合小屏，速度更加跟手
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_start(&a);
        settings_visible = true;
    }
}

/**
 * @brief 导出全局可见：隐藏设置页面动画（无 static，供 titlebar 绑定）
 */
void ui_settings_hide(void) {
    if(settings_page == NULL || !settings_visible) return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, settings_page);
    lv_anim_set_values(&a, 0, UI_SCREEN_WIDTH);
    lv_anim_set_time(&a, 250);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_ready_cb(&a, hide_anim_ready_cb);
    lv_anim_start(&a);
}
