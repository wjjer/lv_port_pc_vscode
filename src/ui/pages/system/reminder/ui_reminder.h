#ifndef SYSTEM_UI_REMINDER_H
#define SYSTEM_UI_REMINDER_H

#include "lvgl.h"

typedef struct {
    char title[64];
    char content[256];
    char time[32];
    bool completed;
    int created_order;
} reminder_item_t;

void ui_reminder_show(void);
void ui_reminder_hide(void);

#endif
