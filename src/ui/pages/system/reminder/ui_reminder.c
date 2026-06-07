#include "ui_reminder.h"
#include "ui_reminder_add.h"
#include "../../../components/ui_titlebar.h"
#include "../../../ui.h"
#include <string.h>
#include <time.h>

#define COLOR_BG lv_color_hex(0xF5F5F7)
#define COLOR_TEXT_MAIN lv_color_hex(0x111111)
#define COLOR_TEXT_SECOND lv_color_hex(0x8E8E93)
#define COLOR_DIVIDER lv_color_hex(0xE5E5EA)
#define COLOR_ACCENT lv_color_hex(0x007AFF)

#define MAX_REMINDERS 20
#define ITEM_HEIGHT 60

static lv_obj_t *reminder_page;
static lv_obj_t *content_area;
static lv_obj_t *titlebar;
static lv_obj_t *delete_msgbox;
static lv_obj_t *delete_cancel_btn;
static lv_obj_t *delete_confirm_btn;
static reminder_item_t reminders[MAX_REMINDERS];
static int reminder_count = 0;
static int delete_index = -1;
static int next_order = 0;

static void init_sample_data(void) {
    strcpy(reminders[0].title, "会议");
    strcpy(reminders[0].content, "与团队讨论项目进度");
    strcpy(reminders[0].time, "09:00");
    reminders[0].completed = false;
    reminders[0].created_order = 0;

    strcpy(reminders[1].title, "吃药");
    strcpy(reminders[1].content, "记得按时吃药");
    strcpy(reminders[1].time, "12:00");
    reminders[1].completed = false;
    reminders[1].created_order = 1;

    strcpy(reminders[2].title, "运动");
    strcpy(reminders[2].content, "跑步30分钟");
    strcpy(reminders[2].time, "18:00");
    reminders[2].completed = true;
    reminders[2].created_order = 2;

    reminder_count = 3;
    next_order = 3;
}

static void refresh_list(void);

static void delete_confirm_cb(lv_event_t *e) {
    LV_UNUSED(e);

    if(delete_index >= 0 && delete_index < reminder_count) {
        for(int i = delete_index; i < reminder_count - 1; i++) {
            reminders[i] = reminders[i + 1];
        }
        reminder_count--;
        refresh_list();
    }

    if(delete_msgbox) {
        lv_msgbox_close(delete_msgbox);
        delete_msgbox = NULL;
    }
    delete_index = -1;
}

static void delete_cancel_cb(lv_event_t *e) {
    LV_UNUSED(e);

    if(delete_msgbox) {
        lv_msgbox_close(delete_msgbox);
        delete_msgbox = NULL;
    }
    delete_index = -1;
}

static void show_delete_msgbox(int index) {
    if(delete_msgbox) {
        lv_msgbox_close(delete_msgbox);
    }

    delete_index = index;

    delete_msgbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(delete_msgbox, "确认删除");
    lv_msgbox_add_text(delete_msgbox, "是否删除此提醒?");

    delete_cancel_btn = lv_msgbox_add_footer_button(delete_msgbox, "取消");
    lv_obj_set_style_text_font(delete_cancel_btn, &font, 0);
    lv_obj_add_event_cb(delete_cancel_btn, delete_cancel_cb, LV_EVENT_CLICKED, NULL);

    delete_confirm_btn = lv_msgbox_add_footer_button(delete_msgbox, "删除");
    lv_obj_set_style_text_font(delete_confirm_btn, &font, 0);
    lv_obj_add_event_cb(delete_confirm_btn, delete_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *content = lv_msgbox_get_content(delete_msgbox);
    if(content) {
        lv_obj_set_style_text_font(content, &font, 0);
    }

    lv_obj_t *title = lv_msgbox_get_title(delete_msgbox);
    if(title) {
        lv_obj_set_style_text_font(title, &font, 0);
    }
}

static void checkbox_event_cb(lv_event_t *e) {
    lv_obj_t *checkbox = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_event_get_user_data(e);

    if(index >= 0 && index < reminder_count) {
        bool checked = lv_obj_has_state(checkbox, LV_STATE_CHECKED);
        reminders[index].completed = checked;
        refresh_list();
    }
}

static void item_long_press_cb(lv_event_t *e) {
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    show_delete_msgbox(index);
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

    lv_obj_add_event_cb(obj, item_long_press_cb, LV_EVENT_LONG_PRESSED, (void *)(intptr_t)index);

    lv_obj_t *cb = lv_checkbox_create(obj);
    lv_checkbox_set_text(cb, item->title);
    lv_obj_set_style_text_font(cb, &font, 0);
    lv_obj_set_style_text_color(cb, item->completed ? COLOR_TEXT_SECOND : COLOR_TEXT_MAIN, 0);
    if(item->completed) {
        lv_obj_set_state(cb, LV_STATE_CHECKED, true);
        lv_obj_set_style_text_decor(cb, LV_TEXT_DECOR_STRIKETHROUGH, 0);
    }
    lv_obj_align(cb, LV_ALIGN_LEFT_MID, 16, 0);
    lv_obj_add_event_cb(cb, checkbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)index);

    if(!item->completed && strlen(item->time) > 0) {
        lv_obj_t *time_lbl = lv_label_create(obj);
        lv_label_set_text(time_lbl, item->time);
        lv_obj_set_style_text_font(time_lbl, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(time_lbl, COLOR_TEXT_SECOND, 0);
        lv_obj_align(time_lbl, LV_ALIGN_RIGHT_MID, -16, 0);
    }

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

    int sorted_indices[MAX_REMINDERS];
    for(int i = 0; i < reminder_count; i++) {
        sorted_indices[i] = i;
    }

    for(int i = 0; i < reminder_count - 1; i++) {
        for(int j = i + 1; j < reminder_count; j++) {
            int idx1 = sorted_indices[i];
            int idx2 = sorted_indices[j];

            bool is_completed_1 = reminders[idx1].completed;
            bool is_completed_2 = reminders[idx2].completed;

            if(is_completed_1 && !is_completed_2) {
                int temp = sorted_indices[i];
                sorted_indices[i] = sorted_indices[j];
                sorted_indices[j] = temp;
            } else if(is_completed_1 == is_completed_2) {
                if(reminders[idx1].created_order < reminders[idx2].created_order) {
                    int temp = sorted_indices[i];
                    sorted_indices[i] = sorted_indices[j];
                    sorted_indices[j] = temp;
                }
            }
        }
    }

    for(int i = 0; i < reminder_count; i++) {
        add_reminder_item(content_area, sorted_indices[i], i * ITEM_HEIGHT);
    }
}

static void save_reminder_cb(const reminder_item_t *item) {
    if(reminder_count >= MAX_REMINDERS) return;

    memcpy(&reminders[reminder_count], item, sizeof(reminder_item_t));
    reminders[reminder_count].created_order = next_order++;
    reminder_count++;
    refresh_list();
}

static void add_btn_event_cb(lv_event_t *e) {
    LV_UNUSED(e);
    ui_reminder_add_show(save_reminder_cb);
}

void ui_reminder_show(void) {
    if(reminder_page != NULL) {
        ui_reminder_hide();
    }

    reminder_page = lv_obj_create(lv_layer_top());
    lv_obj_set_size(reminder_page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(reminder_page, COLOR_BG, 0);
    lv_obj_set_style_pad_all(reminder_page, 0, 0);
    lv_obj_set_style_border_width(reminder_page, 0, 0);
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

    if(reminder_count == 0) {
        init_sample_data();
    }
    refresh_list();

    lv_obj_fade_in(reminder_page, 180, 0);
}

void ui_reminder_hide(void) {
    if(delete_msgbox != NULL) {
        lv_msgbox_close(delete_msgbox);
        delete_msgbox = NULL;
    }

    if(lv_obj_is_valid(reminder_page)) {
        lv_obj_del(reminder_page);
        reminder_page = NULL;
    }

    delete_index = -1;
}
