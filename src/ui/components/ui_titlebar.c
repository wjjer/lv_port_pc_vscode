#include "ui_titlebar.h"
#include "ui_statusbar.h"

int ui_titlebar_get_statusbar_height(void) {
    return STATUSBAR_HEIGHT;
}

int ui_titlebar_get_titlebar_height(void) {
    return TITLEBAR_HEIGHT;
}

//亮度计算
static uint8_t calc_lightness(lv_color_t color) {
    return lv_color_brightness(color);
}

lv_obj_t * ui_titlebar_create(lv_obj_t * parent, const char * title,
                              lv_event_cb_t back_cb, void * back_user_data,
                              const char * right_text, lv_event_cb_t right_cb, void * right_user_data,
                              const lv_color_t * bg_color, const lv_color_t * text_color,
                              bool transparent_bg) {
    // 顶部导航栏（自动偏移避开状态栏，若状态栏隐藏则置顶）
    int32_t header_y = (status_bar && lv_obj_has_flag(status_bar, LV_OBJ_FLAG_HIDDEN)) ? 0 : STATUSBAR_HEIGHT;
    lv_obj_t * header = lv_obj_create(parent);
    lv_obj_set_size(header, lv_pct(100), TITLEBAR_HEIGHT);
    lv_obj_set_pos(header, 0, header_y);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // 背景颜色处理
    lv_color_t actual_bg_color;
    if(bg_color != NULL) {
        actual_bg_color = *bg_color;
    } else {
        actual_bg_color = lv_obj_get_style_bg_color(parent, 0);
    }

    if(transparent_bg) {
        lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    } else {
        lv_obj_set_style_bg_color(header, actual_bg_color, 0);
    }
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_border_opa(header, 0, 0);
    // 计算背景亮度，决定文字颜色
    lv_color_t actual_text_color;
    if(text_color != NULL) {
        actual_text_color = *text_color;
    } else {
        //如果是透明背景，直接去计算父容器的背景色亮度
        uint8_t bg_lightness = calc_lightness(actual_bg_color);
        actual_text_color = (bg_lightness > 128) ? lv_color_black() : lv_color_white();
    }

    // 标题
    lv_obj_t * title_label = lv_label_create(header);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, &font, 0);
    lv_obj_set_style_text_color(title_label, actual_text_color, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);

    // 返回按钮（如果有回调）
    if(back_cb != NULL) {
        lv_obj_t * back_btn = lv_button_create(header);
        lv_obj_set_size(back_btn, 50, 30);
        lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 5, 0);
        lv_obj_set_style_bg_opa(back_btn, 0, 0);
        lv_obj_set_style_shadow_opa(back_btn, 0, 0);
        lv_obj_t * back_sym = lv_label_create(back_btn);
        lv_label_set_text(back_sym, LV_SYMBOL_LEFT);
        lv_obj_set_style_text_color(back_sym, actual_text_color, 0);
        lv_obj_set_style_pad_all(back_btn, 0, 0);
        lv_obj_align(back_sym, LV_ALIGN_CENTER, 0, 0);

        lv_obj_add_event_cb(back_btn, back_cb, LV_EVENT_CLICKED, back_user_data);
    }

    // 右侧按钮（如果有文字和回调）
    if(right_text != NULL && right_cb != NULL) {
        lv_obj_t * right_btn = lv_button_create(header);
        lv_obj_set_size(right_btn, 50, 30);
        lv_obj_align(right_btn, LV_ALIGN_RIGHT_MID, -5, 0);
        lv_obj_set_style_bg_opa(right_btn, 0, 0);
        lv_obj_set_style_shadow_opa(right_btn, 0, 0);
        lv_obj_t * right_label = lv_label_create(right_btn);
        lv_label_set_text(right_label, right_text);
        lv_obj_set_style_text_font(right_label, &font, 0);
        lv_obj_set_style_text_color(right_label, actual_text_color, 0);

        lv_obj_set_style_pad_all(right_btn, 0, 0);
        lv_obj_align(right_label, LV_ALIGN_CENTER, 0, 0);

        lv_obj_add_event_cb(right_btn, right_cb, LV_EVENT_CLICKED, right_user_data);
    }

    return header;
}
