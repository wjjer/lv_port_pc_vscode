#ifndef SYSTEM_UI_REMINDER_ADD_H
#define SYSTEM_UI_REMINDER_ADD_H

#include "lvgl.h"
#include "ui_reminder.h"

typedef void (*reminder_save_cb_t)(const reminder_item_t *item);

void ui_reminder_add_show(reminder_save_cb_t save_cb);
void ui_reminder_add_hide(void);

#endif
