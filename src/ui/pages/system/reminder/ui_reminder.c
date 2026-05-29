#include "ui_reminder.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <string.h>

#define COLOR_BG lv_color_hex(0xF5F5F7)
#define COLOR_TEXT_MAIN lv_color_hex(0x111111)
#define COLOR_TEXT_SECOND lv_color_hex(0x8E8E93)
#define COLOR_DIVIDER lv_color_hex(0xE5E5EA)
#define COLOR_ACCENT lv_color_hex(0x007AFF)
#define COLOR_GREEN lv_color_hex(0x34C759)

#define MAX_REMINDERS 10
#define ITEM_HEIGHT 60

typedef struct {
    char title[64];
    char time[32];
    bool completed;
} reminder_item_t;

static lv_obj_t *reminder_page;
static lv_obj_t *content_area;
static lv_obj_t *titlebar;
static reminder_item_t reminders[MAX_REMINDERS];
static int reminder_count = 3;

static void init_sample_data(void) {
    strcpy(reminders[0].title, "会议");
    strcpy(reminders[0].time, "09:00");
    reminders[0].completed = false;

    strcpy(reminders[1].title, "吃药");
    strcpy(reminders[1].time, "12:00");
    reminders[1].completed = false;

    strcpy(reminders[2].title, "运动");
    strcpy(reminders[2].time, "18:00");
    reminders[2].completed = true;

    reminder_count = 3;
}

static void add_reminder_item(lv_obj_t *parent, int index, int y) {
    reminder_item_t *item = &reminders[index];

    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_size(obj, lv_pct(100), ITEM_HEIGHT);
    lv_obj_set_pos(obj, 0, y);
    lv_obj_set_style_bg_opa(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *cb = lv_checkbox_create(obj);
    lv_checkbox_set_text(cb, item->title);
    lv_obj_set_style_text_font(cb, &font, 0);
    lv_obj_set_style_text_color(cb, item->completed ? COLOR_TEXT_SECOND : COLOR_TEXT_MAIN, 0);
    if(item->completed) {
        lv_obj_set_style_text_decor(cb, LV_TEXT_DECOR_STRIKETHROUGH, 0);
    }
    lv_obj_align(cb, LV_ALIGN_LEFT_MID, 16, 0);

    lv_obj_t *time_lbl = lv_label_create(obj);
    lv_label_set_text(time_lbl, item->time);
    lv_obj_set_style_text_font(time_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(time_lbl, item->completed ? COLOR_TEXT_SECOND : COLOR_TEXT_SECOND, 0);
    lv_obj_align(time_lbl, LV_ALIGN_RIGHT_MID, -16, 0);

    lv_obj_t *line = lv_obj_create(obj);
    lv_obj_set_size(line, lv_pct(100), 1);
    lv_obj_align(line, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(line, COLOR_DIVIDER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
}

static void refresh_list(void) {
    lv_obj_clean(content_area);
    lv_obj_set_scroll_dir(content_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_left(content_area, 0, 0);
    lv_obj_set_style_pad_right(content_area, 0, 0);
    lv_obj_set_style_pad_top(content_area, 10, 0);

    for(int i = 0; i < reminder_count; i++) {
        add_reminder_item(content_area, i, i * ITEM_HEIGHT);
    }
}

static void add_btn_event_cb(lv_event_t *e) {
    LV_UNUSED(e);

    if(reminder_count >= MAX_REMINDERS) return;

    strcpy(reminders[reminder_count].title, "新提醒");
    sprintf(reminders[reminder_count].time, "%02d:00", 8 + reminder_count);
    reminders[reminder_count].completed = false;
    reminder_count++;

    refresh_list();
}

void ui_reminder_show(void) {
    reminder_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(reminder_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(reminder_page, COLOR_BG, 0);
    lv_obj_set_style_pad_all(reminder_page, 0, 0);
    lv_obj_set_style_border_width(reminder_page, 0, 0);
    lv_obj_set_scrollbar_mode(reminder_page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(reminder_page, LV_OBJ_FLAG_SCROLLABLE);

    if(status_bar) lv_obj_move_foreground(status_bar);

    titlebar = ui_titlebar_create(
        reminder_page,
        "提醒事项",
        (lv_event_cb_t)ui_reminder_hide,
        NULL,
        LV_SYMBOL_PLUS,
        add_btn_event_cb,
        NULL,
        NULL,
        NULL,
        false);

    content_area = lv_obj_create(reminder_page);
    lv_obj_set_size(content_area, UI_SCREEN_WIDTH, ui_get_content_height());
    lv_obj_set_pos(content_area, 0, ui_get_content_y());
    lv_obj_set_style_bg_opa(content_area, 0, 0);
    lv_obj_set_style_border_width(content_area, 0, 0);
    lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_OFF);

    init_sample_data();
    refresh_list();

    lv_obj_fade_in(reminder_page, 180, 0);
}

void ui_reminder_hide(void) {
    if(reminder_page != NULL) {
        lv_obj_del(reminder_page);
        reminder_page = NULL;
    }
}
