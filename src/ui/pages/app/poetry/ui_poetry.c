#include "ui_poetry.h"
#include "ui_poetry_data.h"
#include "../../../ui.h"
#include "../../../components/ui_titlebar.h"
#include <string.h>

/* ========== 颜色常量（遵循项目 iOS-minimalist 规范） ========== */
#define COLOR_BG           lv_color_hex(0xF5F5F7)
#define COLOR_SURFACE      lv_color_hex(0xFAFAFC)
#define COLOR_CARD         lv_color_hex(0xFFFFFF)
#define COLOR_TEXT_MAIN    lv_color_hex(0x111111)
#define COLOR_TEXT_SECOND  lv_color_hex(0x8E8E93)
#define COLOR_ACCENT       lv_color_hex(0x007AFF)
#define COLOR_DIVIDER      lv_color_hex(0xE5E5EA)
#define UI_RADIUS          12

/* ========== 全局变量 ========== */
static lv_obj_t *poetry_page = NULL;
static lv_obj_t *list_view = NULL;
static lv_obj_t *detail_view = NULL;
static uint16_t current_poem_index = 0;

/* ========== 前向声明 ========== */
static void show_list_view(void);

/* ========== 回调：返回列表 ========== */
static void back_btn_event_cb(lv_event_t *e) {
    show_list_view();
}

/* ========== 首页列表项点击回调 ========== */
static void list_item_event_cb(lv_event_t *e) {
    lv_obj_t *item = lv_event_get_target(e);
    uint16_t idx = (uint16_t)(uintptr_t)lv_obj_get_user_data(item);
    current_poem_index = idx;

    if (detail_view != NULL) {
        lv_obj_delete(detail_view);
        detail_view = NULL;
    }
    if (list_view != NULL) {
        lv_obj_delete(list_view);
        list_view = NULL;
    }

    /* ---- 创建详情页 ---- */
    detail_view = lv_obj_create(poetry_page);
    lv_obj_set_size(detail_view, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(detail_view, lv_color_hex(0xF8F8F8), 0);
    lv_obj_set_style_border_side(detail_view, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(detail_view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(detail_view, 0, 0);

    poem_t *poem = ui_poetry_get_poem(idx);
    if (!poem) return;

    /* 标题栏 */
    ui_titlebar_create(detail_view, poem->title, back_btn_event_cb, NULL, NULL, NULL, NULL, NULL, NULL, false);

    /* 可滚动内容区 */
    lv_obj_t *scroll_area = lv_obj_create(detail_view);
    lv_obj_set_size(scroll_area, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(scroll_area, 0, ui_get_content_y());
    lv_obj_set_style_bg_color(scroll_area, lv_color_hex(0xF8F8F8), 0);
    lv_obj_set_style_border_side(scroll_area, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(scroll_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(scroll_area, 16, 0);
    lv_obj_set_style_pad_top(scroll_area, 8, 0);
    lv_obj_set_style_pad_bottom(scroll_area, 8, 0);
    lv_obj_set_scroll_dir(scroll_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll_area, LV_SCROLLBAR_MODE_ACTIVE);

    /* ---- 顶部标题区（固定高度） ---- */
    lv_obj_t *header = lv_obj_create(scroll_area);
    lv_obj_set_size(header, lv_pct(100), 110);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xF8F8F8), 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(header, 4, 0);

    /* 蓝色装饰线条 */
    lv_obj_t *accent_line = lv_obj_create(header);
    lv_obj_set_size(accent_line, 60, 3);
    lv_obj_set_style_bg_color(accent_line, COLOR_ACCENT, 0);
    lv_obj_set_style_radius(accent_line, 3, 0);
    lv_obj_set_style_border_width(accent_line, 0, 0);

    /* 诗词标题 */
    lv_obj_t *title_label = lv_label_create(header);
    lv_label_set_text(title_label, poem->title);
    lv_obj_set_style_text_font(title_label, &font, 0);
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_MAIN, 0);

    /* 副标题（可选） */
    if (poem->subtitle[0] != '\0') {
        lv_obj_t *subtitle_label = lv_label_create(header);
        lv_label_set_text(subtitle_label, poem->subtitle);
        lv_obj_set_style_text_font(subtitle_label, &font, 0);
        lv_obj_set_style_text_color(subtitle_label, COLOR_TEXT_SECOND, 0);
        lv_obj_set_style_opa(subtitle_label, LV_OPA_70, 0);
    }

    /* 作者 */
    lv_obj_t *author_label = lv_label_create(header);
    lv_label_set_text(author_label, poem->author);
    lv_obj_set_style_text_font(author_label, &font, 0);
    lv_obj_set_style_text_color(author_label, COLOR_TEXT_SECOND, 0);

    /* ---- 内容卡片 ---- */
    lv_obj_t *content_card = lv_obj_create(scroll_area);
    lv_obj_set_size(content_card, UI_SCREEN_WIDTH - 20, LV_SIZE_CONTENT);
    lv_obj_align(content_card, LV_ALIGN_TOP_MID, 10, 0);
    lv_obj_set_style_bg_color(content_card, COLOR_CARD, 0);
    lv_obj_set_style_radius(content_card, UI_RADIUS, 0);
    lv_obj_set_style_border_width(content_card, 1, 0);
    lv_obj_set_style_border_color(content_card, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_shadow_width(content_card, 10, 0);
    lv_obj_set_style_shadow_color(content_card, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(content_card, LV_OPA_10, 0);
    lv_obj_clear_flag(content_card, LV_OBJ_FLAG_SCROLLABLE);

    /* 内容文本 */
    lv_obj_t *content_label = lv_label_create(content_card);
    lv_label_set_text(content_label, poem->content);
    lv_obj_set_style_text_font(content_label, &font, 0);
    lv_obj_set_style_text_color(content_label, lv_color_hex(0x333333), 0);
    lv_label_set_long_mode(content_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_pad_all(content_label, 20, 0);
    lv_obj_set_style_text_line_space(content_label, 6, 0);
    lv_obj_set_width(content_label, lv_pct(100));
}

/* ========== 回调：返回首页 ========== */
static void back_to_home_event_cb(lv_event_t *e) {
    ui_poetry_hide();
}

/* ========== 显示列表视图 ========== */
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

    /* 系统标题栏 */
    ui_titlebar_create(list_view, "唐诗三百首", back_to_home_event_cb, NULL, NULL, NULL, NULL, NULL, NULL, false);

    /* 可滚动列表容器 */
    lv_obj_t *scroll_list = lv_obj_create(list_view);
    lv_obj_set_size(scroll_list, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(scroll_list, 0, ui_get_content_y());
    lv_obj_set_style_bg_color(scroll_list, COLOR_BG, 0);
    lv_obj_set_style_border_side(scroll_list, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(scroll_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(scroll_list, 10, 0);
    lv_obj_set_style_pad_gap(scroll_list, 8, 0);
    lv_obj_set_scroll_dir(scroll_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll_list, LV_SCROLLBAR_MODE_AUTO);

    uint16_t count = ui_poetry_get_count();
    for (uint16_t i = 0; i < count; i++) {
        poem_t *poem = ui_poetry_get_poem(i);
        if (!poem) continue;

        /* 诗词卡片容器 */
        lv_obj_t *item = lv_obj_create(scroll_list);
        lv_obj_set_size(item, UI_SCREEN_WIDTH - 20, 56);
        lv_obj_set_style_bg_color(item, COLOR_CARD, 0);
        lv_obj_set_style_radius(item, UI_RADIUS, 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_color(item, COLOR_DIVIDER, 0);
        lv_obj_set_style_pad_all(item, 14, 0);
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_user_data(item, (void *)(uintptr_t)i);
        lv_obj_add_event_cb(item, list_item_event_cb, LV_EVENT_CLICKED, NULL);

        /* 诗词标题（左对齐） */
        lv_obj_t *title_label = lv_label_create(item);
        lv_label_set_text(title_label, poem->title);
        lv_label_set_long_mode(title_label, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_font(title_label, &font, 0);
        lv_obj_set_style_text_color(title_label, COLOR_TEXT_MAIN, 0);
        lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, -7);

        /* 诗词作者（右对齐） */
        lv_obj_t *author_label = lv_label_create(item);
        lv_label_set_text(author_label, poem->author);
        lv_label_set_long_mode(author_label, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_font(author_label, &font, 0);
        lv_obj_set_style_text_color(author_label, COLOR_TEXT_SECOND, 0);
        lv_obj_align(author_label, LV_ALIGN_RIGHT_MID, 0, 7);
    }
}

/* ========== 生命周期：显示 ========== */
void ui_poetry_show(void) {
    ui_poetry_hide();
    ui_poetry_data_init();

    poetry_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(poetry_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(poetry_page, COLOR_BG, 0);
    lv_obj_set_style_border_side(poetry_page, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_pad_all(poetry_page, 0, 0);

    show_list_view();
}

/* ========== 生命周期：隐藏 ========== */
void ui_poetry_hide(void) {
    if (lv_obj_is_valid(poetry_page)) {
        lv_obj_delete(poetry_page);
        poetry_page = NULL;
    }
    list_view = NULL;
    detail_view = NULL;
}
