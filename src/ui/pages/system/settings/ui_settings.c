#include "ui_settings.h"
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
    lv_obj_t * item2 = add_menu_item(main_list, LV_SYMBOL_WIFI, "无线网络", "微赞办公室", false);
    lv_obj_set_style_radius(item2, 8, 0);

    lv_obj_t * item3 = add_menu_item(main_list, LV_SYMBOL_BLUETOOTH, "蓝牙", NULL, true);
    lv_obj_set_style_radius(item3, 8, 0);
    // 隐藏本组最后一项的下划线
    if(lv_obj_get_child_cnt(item3) >= 4) {
        lv_obj_set_style_bg_opa(lv_obj_get_child(item3, 3), LV_OPA_TRANSP, 0);
    }

    // --- 分组 3: 系统 ---
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
