#include "lvgl/lvgl.h"

/**
 * @brief 创建 iOS 风格的列表项
 */
static lv_obj_t * create_list_item(lv_obj_t * parent, const char * text, bool show_switch)
{
    lv_obj_t * item = lv_obj_create(parent);
    lv_obj_set_size(item, 280, 40);
    lv_obj_set_style_bg_color(item, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(item, LV_OPA_100, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_pad_hor(item, 15, 0);
    lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(item, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * label = lv_label_create(item);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

    if (show_switch) {
        lv_obj_t * sw = lv_switch_create(item);
        lv_obj_set_size(sw, 36, 20);
        lv_obj_set_style_bg_color(sw, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    } else {
        lv_obj_t * arrow = lv_label_create(item);
        lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_color(arrow, lv_palette_main(LV_PALETTE_GREY), 0);
    }

    return item;
}

void ui_wifi_settings_init(void)
{
    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF2F2F7), 0); // iOS 系统灰底色

    // 1. 顶部标题栏
    lv_obj_t * header = lv_label_create(screen);
    lv_label_set_text(header, "Wi-Fi");
    lv_obj_set_style_text_font(header, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(header, lv_color_black(), 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 10);

    // 2. Wi-Fi 开关分组
    lv_obj_t * group1 = lv_obj_create(screen);
    lv_obj_set_size(group1, 300, 50);
    lv_obj_set_style_radius(group1, 12, 0);
    lv_obj_set_style_bg_color(group1, lv_color_white(), 0);
    lv_obj_set_style_border_width(group1, 0, 0);
    lv_obj_align(group1, LV_ALIGN_TOP_MID, 0, 40);
    create_list_item(group1, "Wi-Fi", true);

    // 3. 网络列表分组
    lv_obj_t * group2 = lv_obj_create(screen);
    lv_obj_set_size(group2, 300, 130);
    lv_obj_set_style_radius(group2, 12, 0);
    lv_obj_set_style_bg_color(group2, lv_color_white(), 0);
    lv_obj_set_style_border_width(group2, 0, 0);
    lv_obj_set_flex_flow(group2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(group2, 0, 0);
    lv_obj_align(group2, LV_ALIGN_TOP_MID, 0, 100);

    create_list_item(group2, "My_Home_WiFi", false);
    create_list_item(group2, "Office_Guest", false);
    create_list_item(group2, "Other...", false);

    // 4. 底部说明
    lv_obj_t * desc = lv_label_create(screen);
    lv_label_set_text(desc, "自动加入已知网络");
    lv_obj_set_style_text_font(desc, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(desc, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align(desc, LV_ALIGN_TOP_LEFT, 20, 240);
}