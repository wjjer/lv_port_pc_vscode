#include "ui_poetry_view.h"
#include "ui_poetry.h"

// ==========================================
// 内部事件包装器：转换 LVGL 事件为业务路由/行为
// ==========================================

static void home_stage_btn_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        const char *stage_title = lv_label_get_text(label);
        ui_poetry_route_to_list(stage_title); // 触发业务路由
    }
}

static void list_item_btn_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_poetry_route_to_detail(); // 触发业务路由
    }
}

static void back_to_home_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_poetry_route_to_home();
    }
}

static void back_to_list_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_poetry_route_to_list("古诗列表");
    }
}

static void detail_play_btn_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        ui_poetry_toggle_play(label); // 移交业务层处理状态
    }
}

static void detail_notes_btn_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *notes_panel = (lv_obj_t *)lv_event_get_user_data(e);
        ui_poetry_toggle_notes(notes_panel); // 移交业务层处理显隐
    }
}

// ==========================================
// UI 界面纯绘制实现
// ==========================================

void ui_poetry_view_render_home(void) {
    current_page = lv_obj_create(app_screen);
    lv_obj_set_size(current_page, 320, 240);
    lv_obj_set_style_bg_color(current_page, lv_color_hex(0xFDFBF7), 0);
    lv_obj_remove_style(current_page, NULL, LV_PART_SCROLLBAR);
    lv_obj_set_style_border_width(current_page, 0, 0);

    lv_obj_t *title = lv_label_create(current_page);
    lv_label_set_text(title, "古韵诗华");
    lv_obj_set_style_text_color(title, lv_color_hex(0xA62B2B), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 25);

    lv_obj_t *subtitle = lv_label_create(current_page);
    lv_label_set_text(subtitle, "中华经典诗词学习");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x666666), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 55);

    lv_obj_t *btn_container = lv_obj_create(current_page);
    lv_obj_set_size(btn_container, 280, 100);
    lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_remove_style(btn_container, NULL, LV_PART_SCROLLBAR);

    const char *stages[] = {"小学诗词", "初中诗词", "高中诗词"};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_button_create(btn_container);
        lv_obj_set_size(btn, 80, 50);
        lv_obj_add_style(btn, &style_btn_classic, 0); // 链接全局样式
        lv_obj_add_event_cb(btn, home_stage_btn_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, stages[i]);
        lv_obj_center(label);
    }
}

void ui_poetry_view_render_list(const char *stage_title) {
    current_page = lv_obj_create(app_screen);
    lv_obj_set_size(current_page, 320, 240);
    lv_obj_set_style_bg_color(current_page, lv_color_hex(0xFDFBF7), 0);
    lv_obj_set_style_border_width(current_page, 0, 0);

    lv_obj_t *header = lv_obj_create(current_page);
    lv_obj_set_size(header, 320, 40);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x2C4A3E), 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t *back_btn = lv_button_create(header);
    lv_obj_set_size(back_btn, 40, 30);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x1E332A), 0);
    lv_obj_add_event_cb(back_btn, back_to_home_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);

    lv_obj_t *header_title = lv_label_create(header);
    lv_label_set_text(header_title, stage_title);
    lv_obj_set_style_text_color(header_title, lv_color_white(), 0);
    lv_obj_align(header_title, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *list = lv_obj_create(current_page);
    lv_obj_set_size(list, 320, 200);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 10, 0);
    lv_obj_set_style_pad_row(list, 8, 0);

    const char *mock_poems[] = {"1. 静夜思 (李白)", "2. 春晓 (孟浩然)", "3. 赠汪伦 (李白)", "4. 登鹳雀楼 (王之涣)", "5. 咏鹅 (骆宾王)"};
    for (int i = 0; i < 5; i++) {
        lv_obj_t *item = lv_button_create(list);
        lv_obj_set_width(item, lv_pct(100));
        lv_obj_set_height(item, 40);
        lv_obj_set_style_bg_color(item, lv_color_white(), 0);
        lv_obj_set_style_text_color(item, lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0xE0DCD3), 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_radius(item, 6, 0);
        lv_obj_set_style_shadow_width(item, 0, 0);
        lv_obj_add_event_cb(item, list_item_btn_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *item_text = lv_label_create(item);
        lv_label_set_text(item_text, mock_poems[i]);
        lv_obj_align(item_text, LV_ALIGN_LEFT_MID, 10, 0);
    }
}

void ui_poetry_view_render_detail(const char *title, const char *author, const char *content, const char *notes) {
    current_page = lv_obj_create(app_screen);
    lv_obj_set_size(current_page, 320, 240);
    lv_obj_set_style_bg_color(current_page, lv_color_hex(0xFDFBF7), 0);
    lv_obj_set_style_border_width(current_page, 0, 0);

    // 头部导航条
    lv_obj_t *header = lv_obj_create(current_page);
    lv_obj_set_size(header, 320, 35);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x2C4A3E), 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t *back_btn = lv_button_create(header);
    lv_obj_set_size(back_btn, 40, 26);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x1E332A), 0);
    lv_obj_add_event_cb(back_btn, back_to_list_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);

    lv_obj_t *header_title = lv_label_create(header);
    lv_label_set_text(header_title, "诗词正文");
    lv_obj_set_style_text_color(header_title, lv_color_white(), 0);
    lv_obj_align(header_title, LV_ALIGN_CENTER, 0, 0);

    // 正文区域
    lv_obj_t *text_area = lv_obj_create(current_page);
    lv_obj_set_size(text_area, 320, 155);
    lv_obj_align(text_area, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_flex_flow(text_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(text_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(text_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(text_area, 0, 0);
    lv_obj_set_style_pad_all(text_area, 15, 0);

    lv_obj_t *p_title = lv_label_create(text_area);
    lv_label_set_text(p_title, title);
    lv_obj_set_style_text_color(p_title, lv_color_hex(0xA62B2B), 0);

    lv_obj_t *p_author = lv_label_create(text_area);
    lv_label_set_text(p_author, author);
    lv_obj_set_style_text_color(p_author, lv_color_hex(0x666666), 0);
    lv_obj_set_style_pad_bottom(p_author, 10, 0);

    lv_obj_t *p_content = lv_label_create(text_area);
    lv_label_set_text(p_content, content);
    lv_obj_set_style_text_align(p_content, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(p_content, 6, 0);

    // 底部控制栏
    lv_obj_t *control_bar = lv_obj_create(current_page);
    lv_obj_set_size(control_bar, 320, 50);
    lv_obj_align(control_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(control_bar, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(control_bar, 0, 0);
    lv_obj_set_style_border_width(control_bar, 0, 0);

    lv_obj_t *play_btn = lv_button_create(control_bar);
    lv_obj_set_size(play_btn, 36, 36);
    lv_obj_align(play_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0xA62B2B), 0);
    lv_obj_set_style_radius(play_btn, 18, 0);
    lv_obj_add_event_cb(play_btn, detail_play_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *play_label = lv_label_create(play_btn);
    lv_label_set_text(play_label, " " LV_SYMBOL_PLAY " ");
    lv_obj_center(play_label);

    lv_obj_t *progress = lv_bar_create(control_bar);
    lv_obj_set_size(progress, 160, 6);
    lv_obj_align(progress, LV_ALIGN_CENTER, -10, 0);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress, lv_color_hex(0xA62B2B), LV_PART_INDICATOR);
    lv_bar_set_value(progress, 35, LV_ANIM_OFF);

    // 弹窗式注释面板（创建在后，图层在上）
    lv_obj_t *annotation_panel = lv_obj_create(current_page);
    lv_obj_set_size(annotation_panel, 280, 160);
    lv_obj_center(annotation_panel);
    lv_obj_set_style_bg_color(annotation_panel, lv_color_white(), 0);
    lv_obj_set_style_border_color(annotation_panel, lv_color_hex(0xA62B2B), 0);
    lv_obj_set_style_border_width(annotation_panel, 1, 0);
    lv_obj_set_style_radius(annotation_panel, 8, 0);
    lv_obj_set_style_shadow_width(annotation_panel, 15, 0);
    lv_obj_set_style_shadow_opa(annotation_panel, LV_OPA_30, 0);
    lv_obj_add_flag(annotation_panel, LV_OBJ_FLAG_HIDDEN); // 默认隐藏

    lv_obj_t *notes_label = lv_label_create(annotation_panel);
    lv_obj_set_width(notes_label, 240);
    lv_label_set_text(notes_label, notes);
    lv_label_set_long_mode(notes_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(notes_label, LV_ALIGN_TOP_LEFT, 5, 5);

    // 译/注切换按钮
    lv_obj_t *notes_btn = lv_button_create(control_bar);
    lv_obj_set_size(notes_btn, 50, 32);
    lv_obj_align(notes_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(notes_btn, lv_color_hex(0x444444), 0);
    lv_obj_set_style_border_color(notes_btn, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(notes_btn, 1, 0);
    // 通过 user_data 把控制目标 panel 传给回调
    lv_obj_add_event_cb(notes_btn, detail_notes_btn_cb, LV_EVENT_CLICKED, (void *)annotation_panel);

    lv_obj_t *notes_btn_label = lv_label_create(notes_btn);
    lv_label_set_text(notes_btn_label, "译/注");
    lv_obj_center(notes_btn_label);
}
