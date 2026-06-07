#include "ui_settings_wifi.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <string.h>

// iOS风格配色
#define COLOR_WIFI_BG       lv_color_hex(0xF2F2F7)  // 浅灰底
#define COLOR_ITEM_BG       lv_color_white()        // 卡片白底
#define COLOR_TEXT_MAIN     lv_color_hex(0x000000)  // 主文字黑
#define COLOR_TEXT_MUTED    lv_color_hex(0x8E8E93)  // 辅助文字灰
#define COLOR_LINE          lv_color_hex(0xE5E5EA)  // 分割线
#define COLOR_IOS_BLUE      lv_color_hex(0x007AFF)  // iOS蓝
#define COLOR_IOS_GREEN     lv_color_hex(0x34C759)  // iOS绿
#define COLOR_IOS_RED       lv_color_hex(0xFF3B30)  // iOS红

static lv_obj_t * wifi_page = NULL;
static bool wifi_visible = false;
static lv_obj_t * wifi_switch = NULL;
static lv_obj_t * network_list_container = NULL;
static lv_obj_t * password_dialog = NULL;
static char selected_ssid[64] = {0};

// 模拟WiFi网络数据结构
typedef struct {
    char ssid[64];
    int8_t rssi;  // 信号强度 (-100 ~ 0 dBm)
    bool is_connected;
    bool require_password;
} wifi_network_t;

// 模拟WiFi网络列表（实际应用中应从ESP32获取）
static wifi_network_t mock_networks[] = {
    {"微赞办公室", -45, true, true},
    {"TP-LINK_5G", -60, false, true},
    {"ChinaNet-ABC", -72, false, true},
    {"Guest_WiFi", -55, false, false},
    {"Home_Network", -68, false, true},
};
static const int mock_network_count = sizeof(mock_networks) / sizeof(wifi_network_t);

/**
 * @brief 根据RSSI获取信号强度图标
 */
static const char * get_wifi_signal_icon(int8_t rssi) {
    if(rssi >= -50) return LV_SYMBOL_WIFI;  // 强信号
    if(rssi >= -70) return LV_SYMBOL_WIFI;  // 中等信号
    return LV_SYMBOL_WIFI;  // 弱信号
}

/**
 * @brief 关闭密码输入对话框
 */
static void close_password_dialog(void) {
    if(password_dialog && lv_obj_is_valid(password_dialog)) {
        lv_obj_delete(password_dialog);
        password_dialog = NULL;
    }
}

/**
 * @brief 密码对话框取消按钮回调
 */
static void password_cancel_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        close_password_dialog();
    }
}

/**
 * @brief 密码对话框连接按钮回调
 */
static void password_connect_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * textarea = lv_event_get_user_data(e);
        const char * password = lv_textarea_get_text(textarea);

        // TODO: 实际应用中应调用ESP32 WiFi连接API
        // 这里模拟连接逻辑
        printf("尝试连接WiFi: %s, 密码: %s\n", selected_ssid, password);

        close_password_dialog();

        // 模拟连接结果（实际应等待ESP32返回结果）
        // 这里简单显示提示
    }
}

/**
 * @brief 显示密码输入对话框
 */
static void show_password_dialog(const char * ssid) {
    if(password_dialog != NULL) return;

    strncpy(selected_ssid, ssid, sizeof(selected_ssid) - 1);

    // 创建半透明遮罩
    password_dialog = lv_obj_create(lv_layer_top());
    lv_obj_set_size(password_dialog, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(password_dialog, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(password_dialog, LV_OPA_50, 0);
    lv_obj_set_style_border_width(password_dialog, 0, 0);
    lv_obj_set_style_pad_all(password_dialog, 0, 0);

    // 对话框卡片
    lv_obj_t * card = lv_obj_create(password_dialog);
    lv_obj_set_size(card, 280, 200);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, COLOR_ITEM_BG, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 20, 0);

    // 标题
    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text_fmt(title, "连接到 %s", ssid);
    lv_obj_set_style_text_font(title, &font, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_MAIN, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    // 密码输入框
    lv_obj_t * textarea = lv_textarea_create(card);
    lv_obj_set_size(textarea, LV_PCT(100), 40);
    lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 40);
    lv_textarea_set_placeholder_text(textarea, "输入密码");
    lv_textarea_set_password_mode(textarea, true);
    lv_textarea_set_one_line(textarea, true);
    lv_obj_set_style_text_font(textarea, &font, 0);

    // 按钮容器
    lv_obj_t * btn_container = lv_obj_create(card);
    lv_obj_set_size(btn_container, LV_PCT(100), 40);
    lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_set_style_pad_all(btn_container, 0, 0);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 取消按钮
    lv_obj_t * btn_cancel = lv_button_create(btn_container);
    lv_obj_set_size(btn_cancel, 100, 40);
    lv_obj_set_style_bg_color(btn_cancel, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_radius(btn_cancel, 10, 0);
    lv_obj_add_event_cb(btn_cancel, password_cancel_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, "取消");
    lv_obj_set_style_text_font(label_cancel, &font, 0);
    lv_obj_center(label_cancel);

    // 连接按钮
    lv_obj_t * btn_connect = lv_button_create(btn_container);
    lv_obj_set_size(btn_connect, 100, 40);
    lv_obj_set_style_bg_color(btn_connect, COLOR_IOS_BLUE, 0);
    lv_obj_set_style_radius(btn_connect, 10, 0);
    lv_obj_add_event_cb(btn_connect, password_connect_cb, LV_EVENT_CLICKED, textarea);
    lv_obj_t * label_connect = lv_label_create(btn_connect);
    lv_label_set_text(label_connect, "连接");
    lv_obj_set_style_text_font(label_connect, &font, 0);
    lv_obj_center(label_connect);
}

/**
 * @brief WiFi网络项点击回调
 */
static void network_item_clicked(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        wifi_network_t * network = (wifi_network_t *)lv_event_get_user_data(e);
        if(network == NULL) return;

        // 如果已连接，不做处理
        if(network->is_connected) return;

        // 如果需要密码，弹出密码输入对话框
        if(network->require_password) {
            show_password_dialog(network->ssid);
        } else {
            // 不需要密码，直接连接
            printf("尝试连接开放WiFi: %s\n", network->ssid);
            // TODO: 调用ESP32 WiFi连接API
        }
    }
}

/**
 * @brief 创建WiFi网络列表项
 */
static lv_obj_t * create_network_item(lv_obj_t * parent, wifi_network_t * network, bool is_first, bool is_last) {
    lv_obj_t * item = lv_obj_create(parent);
    lv_obj_set_size(item, LV_PCT(100), 50);
    lv_obj_set_style_bg_color(item, COLOR_ITEM_BG, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_radius(item, 0, 0);
    lv_obj_set_style_pad_all(item, 0, 0);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);

    // 第一项和最后一项设置圆角
    if(is_first) lv_obj_set_style_radius(item, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    if(is_last) lv_obj_set_style_radius(item, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    // WiFi图标
    lv_obj_t * icon = lv_label_create(item);
    lv_label_set_text(icon, get_wifi_signal_icon(network->rssi));
    lv_obj_set_style_text_color(icon, COLOR_IOS_BLUE, 0);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 12, 0);

    // 网络名称
    lv_obj_t * ssid_label = lv_label_create(item);
    lv_label_set_text(ssid_label, network->ssid);
    lv_obj_set_style_text_color(ssid_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(ssid_label, &font, 0);
    lv_obj_align(ssid_label, LV_ALIGN_LEFT_MID, 40, 0);

    // 右侧状态/箭头
    if(network->is_connected) {
        lv_obj_t * status = lv_label_create(item);
        lv_label_set_text(status, "已连接");
        lv_obj_set_style_text_color(status, COLOR_IOS_BLUE, 0);
        lv_obj_set_style_text_font(status, &font, 0);
        lv_obj_align(status, LV_ALIGN_RIGHT_MID, -30, 0);

        lv_obj_t * checkmark = lv_label_create(item);
        lv_label_set_text(checkmark, LV_SYMBOL_OK);
        lv_obj_set_style_text_color(checkmark, COLOR_IOS_BLUE, 0);
        lv_obj_align(checkmark, LV_ALIGN_RIGHT_MID, -12, 0);
    } else {
        lv_obj_t * arrow = lv_label_create(item);
        lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_color(arrow, COLOR_TEXT_MUTED, 0);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -12, 0);
    }

    // 分割线（最后一项不显示）
    if(!is_last) {
        lv_obj_t * line = lv_obj_create(item);
        lv_obj_set_size(line, LV_PCT(100), 1);
        lv_obj_align(line, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(line, COLOR_LINE, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_add_flag(line, LV_OBJ_FLAG_IGNORE_LAYOUT);
    }

    // 添加点击事件
    lv_obj_add_event_cb(item, network_item_clicked, LV_EVENT_CLICKED, network);

    return item;
}

/**
 * @brief 刷新WiFi网络列表
 */
static void refresh_network_list(void) {
    if(network_list_container == NULL) return;

    // 清空现有列表
    lv_obj_clean(network_list_container);

    // 创建网络列表项
    for(int i = 0; i < mock_network_count; i++) {
        bool is_first = (i == 0);
        bool is_last = (i == mock_network_count - 1);
        create_network_item(network_list_container, &mock_networks[i], is_first, is_last);
    }
}

/**
 * @brief WiFi开关事件回调
 */
static void wifi_switch_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * sw = lv_event_get_target(e);
        bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);

        if(is_checked) {
            // 打开WiFi
            printf("打开WiFi\n");
            // TODO: 调用ESP32 WiFi开启API
            // esp_wifi_start();

            // 显示网络列表
            if(network_list_container) {
                lv_obj_remove_flag(network_list_container, LV_OBJ_FLAG_HIDDEN);
                refresh_network_list();
            }
        } else {
            // 关闭WiFi
            printf("关闭WiFi\n");
            // TODO: 调用ESP32 WiFi关闭API
            // esp_wifi_stop();

            // 隐藏网络列表
            if(network_list_container) {
                lv_obj_add_flag(network_list_container, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

/**
 * @brief 创建WiFi开关项
 */
static lv_obj_t * create_wifi_switch_item(lv_obj_t * parent) {
    lv_obj_t * item = lv_obj_create(parent);
    lv_obj_set_size(item, LV_PCT(100), 50);
    lv_obj_set_style_bg_color(item, COLOR_ITEM_BG, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_radius(item, 8, 0);
    lv_obj_set_style_pad_all(item, 0, 0);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

    // WiFi图标
    lv_obj_t * icon = lv_label_create(item);
    lv_label_set_text(icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(icon, COLOR_IOS_BLUE, 0);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 12, 0);

    // WiFi文本
    lv_obj_t * label = lv_label_create(item);
    lv_label_set_text(label, "无线局域网");
    lv_obj_set_style_text_color(label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(label, &font, 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 40, 0);

    // WiFi开关
    wifi_switch = lv_switch_create(item);
    lv_obj_set_size(wifi_switch, 42, 24);
    lv_obj_align(wifi_switch, LV_ALIGN_RIGHT_MID, -12, 0);
    lv_obj_set_style_bg_color(wifi_switch, COLOR_IOS_GREEN, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(wifi_switch, wifi_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 默认打开
    lv_obj_add_state(wifi_switch, LV_STATE_CHECKED);

    return item;
}

/**
 * @brief 页面初始化
 */
static void ui_settings_wifi_init(void) {
    // 主页面容器
    wifi_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(wifi_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_x(wifi_page, UI_SCREEN_WIDTH);
    lv_obj_set_style_bg_color(wifi_page, COLOR_WIFI_BG, 0);
    lv_obj_set_style_pad_all(wifi_page, 0, 0);
    lv_obj_set_style_border_width(wifi_page, 0, 0);
    lv_obj_add_flag(wifi_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(wifi_page, LV_OBJ_FLAG_SCROLLABLE);

    // 状态栏保持可见
    if(status_bar) lv_obj_move_foreground(status_bar);

    // 标题栏
    ui_titlebar_create(wifi_page, "无线网络", (lv_event_cb_t)ui_settings_wifi_hide, NULL, NULL, NULL, NULL, NULL, NULL, false);

    // 主内容区域
    lv_obj_t * content = lv_obj_create(wifi_page);
    lv_obj_set_size(content, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(content, 0, ui_get_content_y());
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_hor(content, 14, 0);
    lv_obj_set_style_pad_ver(content, 8, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, 8, 0);

    // WiFi开关项
    create_wifi_switch_item(content);

    // 网络列表分组标题
    lv_obj_t * list_title = lv_label_create(content);
    lv_label_set_text(list_title, "可用网络");
    lv_obj_set_style_text_color(list_title, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(list_title, &font, 0);
    lv_obj_set_width(list_title, LV_PCT(100));
    lv_obj_set_style_text_align(list_title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_margin_top(list_title, 10, 0);
    lv_obj_set_style_margin_bottom(list_title, 4, 0);
    lv_obj_set_style_margin_left(list_title, 4, 0);

    // 网络列表容器
    network_list_container = lv_obj_create(content);
    lv_obj_set_size(network_list_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(network_list_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(network_list_container, 0, 0);
    lv_obj_set_style_pad_all(network_list_container, 0, 0);
    lv_obj_set_flex_flow(network_list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(network_list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(network_list_container, 0, 0);

    // 初始化网络列表
    refresh_network_list();
}

/**
 * @brief 隐藏动画完成回调
 */
static void hide_anim_ready_cb(lv_anim_t * a) {
    wifi_visible = false;
    lv_obj_add_flag(wifi_page, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief 显示WiFi页面
 */
void ui_settings_wifi_show(void) {
    if(wifi_page == NULL) {
        ui_settings_wifi_init();
    }

    if(lv_obj_has_flag(wifi_page, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(wifi_page, LV_OBJ_FLAG_HIDDEN);

        // 滑入动画
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, wifi_page);
        lv_anim_set_values(&a, UI_SCREEN_WIDTH, 0);
        lv_anim_set_time(&a, 250);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_start(&a);

        wifi_visible = true;
    }
}

/**
 * @brief 隐藏WiFi页面
 */
void ui_settings_wifi_hide(void) {
    if(wifi_page == NULL || !wifi_visible) return;

    // 关闭可能打开的密码对话框
    close_password_dialog();

    // 滑出动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, wifi_page);
    lv_anim_set_values(&a, 0, UI_SCREEN_WIDTH);
    lv_anim_set_time(&a, 250);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_ready_cb(&a, hide_anim_ready_cb);
    lv_anim_start(&a);
}

