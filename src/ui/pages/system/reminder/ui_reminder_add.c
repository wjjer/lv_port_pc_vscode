#include "ui_reminder_add.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <string.h>
#include <time.h>

#define COLOR_BG lv_color_hex(0xF5F5F7)
#define COLOR_TEXT_MAIN lv_color_hex(0x111111)
#define COLOR_TEXT_SECOND lv_color_hex(0x8E8E93)
#define COLOR_ACCENT lv_color_hex(0x007AFF)

static lv_obj_t *add_page;
static lv_obj_t *title_ta;
static lv_obj_t *content_ta;
static reminder_save_cb_t save_callback;

static void save_btn_event_cb(lv_event_t *e) {
    LV_UNUSED(e);

    const char *title_text = lv_textarea_get_text(title_ta);
    const char *content_text = lv_textarea_get_text(content_ta);

    if(strlen(title_text) == 0) {
        return;
    }

    reminder_item_t item;
    strcpy(item.title, title_text);
    strcpy(item.content, content_text);
    item.completed = false;
    item.created_order = 0;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(item.time, "%02d:%02d", t->tm_hour, t->tm_min);

    if(save_callback) {
        save_callback(&item);
    }

    ui_reminder_add_hide();
}

static void cancel_btn_event_cb(lv_event_t *e) {
    LV_UNUSED(e);
    ui_reminder_add_hide();
}

void ui_reminder_add_show(reminder_save_cb_t save_cb) {
    save_callback = save_cb;

    add_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(add_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(add_page, COLOR_BG, 0);
    lv_obj_set_style_pad_all(add_page, 0, 0);
    lv_obj_set_style_border_width(add_page, 0, 0);
    lv_obj_clear_flag(add_page, LV_OBJ_FLAG_SCROLLABLE);

    if(status_bar) lv_obj_move_foreground(status_bar);

    ui_titlebar_create(
        add_page,
        "添加提醒",
        (lv_event_cb_t)cancel_btn_event_cb,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        false);

    lv_obj_t *scroll_area = lv_obj_create(add_page);
    lv_obj_set_size(scroll_area, UI_SCREEN_WIDTH, ui_get_content_height() - 60);
    lv_obj_set_pos(scroll_area, 0, ui_get_content_y());
    lv_obj_set_style_bg_opa(scroll_area, 0, 0);
    lv_obj_set_style_border_width(scroll_area, 0, 0);
    lv_obj_set_style_pad_all(scroll_area, 0, 0);
    lv_obj_set_scroll_dir(scroll_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll_area, LV_SCROLLBAR_MODE_OFF);

    int content_width = UI_SCREEN_WIDTH - 32;

    lv_obj_t *title_label = lv_label_create(scroll_area);
    lv_label_set_text(title_label, "标题");
    lv_obj_set_style_text_font(title_label, &font, 0);
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_pos(title_label, 16, 16);

    title_ta = lv_textarea_create(scroll_area);
    lv_textarea_set_placeholder_text(title_ta, "请输入提醒标题");
    lv_obj_set_size(title_ta, content_width, 40);
    lv_obj_set_pos(title_ta, 16, 40);
    lv_textarea_set_max_length(title_ta, 63);
    lv_textarea_set_one_line(title_ta, true);
    lv_obj_set_style_text_font(title_ta, &font, 0);

    lv_obj_t *content_label = lv_label_create(scroll_area);
    lv_label_set_text(content_label, "内容");
    lv_obj_set_style_text_font(content_label, &font, 0);
    lv_obj_set_style_text_color(content_label, COLOR_TEXT_MAIN, 0);
    lv_obj_set_pos(content_label, 16, 100);

    content_ta = lv_textarea_create(scroll_area);
    lv_textarea_set_placeholder_text(content_ta, "请输入提醒内容");
    lv_obj_set_size(content_ta, content_width, 100);
    lv_obj_set_pos(content_ta, 16, 124);
    lv_textarea_set_max_length(content_ta, 255);
    lv_obj_set_style_text_font(content_ta, &font, 0);

    lv_obj_t *button_container = lv_obj_create(add_page);
    lv_obj_set_size(button_container, UI_SCREEN_WIDTH, 60);
    lv_obj_set_pos(button_container, 0, UI_SCREEN_HEIGHT - 60);
    lv_obj_set_style_bg_opa(button_container, 0, 0);
    lv_obj_set_style_border_width(button_container, 0, 0);
    lv_obj_set_style_pad_all(button_container, 0, 0);
    lv_obj_clear_flag(button_container, LV_OBJ_FLAG_SCROLLABLE);

    int btn_width = (UI_SCREEN_WIDTH - 48) / 2;

    lv_obj_t *cancel_btn = lv_btn_create(button_container);
    lv_obj_set_size(cancel_btn, btn_width, 40);
    lv_obj_set_pos(cancel_btn, 16, 10);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xE5E5EA), 0);
    lv_obj_set_style_text_color(cancel_btn, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_radius(cancel_btn, 10, 0);
    lv_obj_add_event_cb(cancel_btn, cancel_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_set_style_text_font(cancel_label, &font, 0);
    lv_obj_center(cancel_label);

    lv_obj_t *save_btn = lv_btn_create(button_container);
    lv_obj_set_size(save_btn, btn_width, 40);
    lv_obj_set_pos(save_btn, 16 + btn_width + 16, 10);
    lv_obj_set_style_bg_color(save_btn, COLOR_ACCENT, 0);
    lv_obj_set_style_text_color(save_btn, lv_color_white(), 0);
    lv_obj_set_style_radius(save_btn, 10, 0);
    lv_obj_add_event_cb(save_btn, save_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, "保存");
    lv_obj_set_style_text_font(save_label, &font, 0);
    lv_obj_center(save_label);

    lv_obj_fade_in(add_page, 180, 0);
}

void ui_reminder_add_hide(void) {
    if(add_page != NULL) {
        lv_obj_del(add_page);
        add_page = NULL;
    }
}
