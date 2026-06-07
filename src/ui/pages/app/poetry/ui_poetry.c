#include "ui_poetry.h"
#include "ui_poetry_data.h"
#include "../../../ui.h"
#include "../../../components/ui_titlebar.h"
#include <string.h>

static lv_obj_t *poetry_page = NULL;
static lv_obj_t *list_view = NULL;
static lv_obj_t *detail_view = NULL;
static uint16_t current_poem_index = 0;

// 前向声明
static void show_list_view(void);

// 回调：返回列表
static void back_btn_event_cb(lv_event_t *e) {
    show_list_view();
}

// 回调：列表项点击
static void list_item_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    uint16_t idx = (uint16_t)(uintptr_t)lv_obj_get_user_data(btn);
    current_poem_index = idx;

    if (detail_view != NULL) {
        lv_obj_delete(detail_view);
        detail_view = NULL;
    }
    if (list_view != NULL) {
        lv_obj_delete(list_view);
        list_view = NULL;
    }

    detail_view = lv_obj_create(poetry_page);
    lv_obj_set_size(detail_view, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(detail_view, lv_color_hex(0xF8F8F8), 0);
    lv_obj_set_style_border_side(detail_view, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(detail_view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(detail_view, 0, 0);

    poem_t *poem = ui_poetry_get_poem(idx);
    if (!poem) return;

    // 系统标题栏
    ui_titlebar_create(detail_view, poem->title, back_btn_event_cb, NULL, NULL, NULL, NULL, NULL, NULL, false);

    // 内容区域
    lv_obj_t *content_area = lv_obj_create(detail_view);
    lv_obj_set_size(content_area, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(content_area, 0, ui_get_content_y());
    lv_obj_set_style_bg_color(content_area, lv_color_hex(0xF8F8F8), 0);
    lv_obj_set_style_border_side(content_area, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(content_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(content_area, 15, 0);
    lv_obj_set_style_pad_gap(content_area, 12, 0);

    // 作者
    lv_obj_t *author_label = lv_label_create(content_area);
    lv_label_set_text(author_label, poem->author);
    lv_obj_set_style_text_font(author_label, &font, 0);
    lv_obj_set_style_text_color(author_label, lv_color_hex(0x999999), 0);

    // 诗句内容卡片
    lv_obj_t *content_card = lv_obj_create(content_area);
    lv_obj_set_size(content_card, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(content_card, 1);
    lv_obj_set_style_bg_color(content_card, lv_color_white(), 0);
    lv_obj_set_style_border_color(content_card, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_border_width(content_card, 1, 0);
    lv_obj_set_style_radius(content_card, 12, 0);
    lv_obj_set_style_pad_all(content_card, 15, 0);
    lv_obj_set_style_shadow_width(content_card, 2, 0);
    lv_obj_set_style_shadow_color(content_card, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(content_card, 10, 0);

    lv_obj_t *content_label = lv_label_create(content_card);
    lv_label_set_text(content_label, poem->content);
    lv_obj_set_style_text_font(content_label, &font, 0);
    lv_obj_set_style_text_color(content_label, lv_color_hex(0x333333), 0);
    lv_label_set_long_mode(content_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(content_label, LV_SIZE_CONTENT);
}

// 回调：返回到桌面
static void back_to_home_event_cb(lv_event_t *e) {
    ui_poetry_hide();
}

// 显示列表视图
static void show_list_view(void) {
    if (list_view != NULL) {
        lv_obj_delete(list_view);
    }
    if (detail_view != NULL) {
        lv_obj_delete(detail_view);
        detail_view = NULL;
    }

    list_view = lv_obj_create(poetry_page);
    lv_obj_set_size(list_view, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(list_view, lv_color_hex(0xF8F8F8), 0);
    lv_obj_set_style_border_side(list_view, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(list_view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(list_view, 0, 0);

    // 系统标题栏
    ui_titlebar_create(list_view, "唐诗三百首", back_to_home_event_cb, NULL, NULL, NULL, NULL, NULL, NULL, false);

    // 列表容器
    lv_obj_t *list = lv_list_create(list_view);
    lv_obj_set_size(list, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(list, 0, ui_get_content_y());
    lv_obj_set_style_bg_color(list, lv_color_hex(0xF8F8F8), 0);
    lv_obj_set_style_border_side(list, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_pad_all(list, 10, 0);
    lv_obj_set_style_pad_gap(list, 8, 0);

    uint16_t count = ui_poetry_get_count();
    for (uint16_t i = 0; i < count; i++) {
        poem_t *poem = ui_poetry_get_poem(i);
        if (!poem) continue;

        lv_obj_t *item = lv_list_add_button(list, NULL, poem->title);
        lv_obj_set_user_data(item, (void *)(uintptr_t)i);
        lv_obj_add_event_cb(item, list_item_event_cb, LV_EVENT_CLICKED, NULL);

        // iOS 风格：圆角、阴影、白色背景
        lv_obj_set_style_bg_color(item, lv_color_white(), 0);
        lv_obj_set_style_radius(item, 10, 0);
        lv_obj_set_style_shadow_width(item, 1, 0);
        lv_obj_set_style_shadow_color(item, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(item, 5, 0);
        lv_obj_set_style_border_side(item, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_pad_all(item, 12, 0);

        // 文字颜色和大小
        lv_obj_t *label = lv_obj_get_child(item, 0);
        if (label) {
            lv_obj_set_style_text_color(label, lv_color_hex(0x333333), 0);
            lv_obj_set_style_text_font(label, &font, 0);
        }
    }
}

void ui_poetry_show(void) {
    ui_poetry_hide();
    ui_poetry_data_init();

    poetry_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(poetry_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(poetry_page, lv_color_hex(0xF5F5F7), 0);
    lv_obj_set_style_border_side(poetry_page, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_pad_all(poetry_page, 0, 0);

    show_list_view();
}

void ui_poetry_hide(void) {
    if (lv_obj_is_valid(poetry_page)) {
        lv_obj_delete(poetry_page);
        poetry_page = NULL;
    }
    list_view = NULL;
    detail_view = NULL;
}
