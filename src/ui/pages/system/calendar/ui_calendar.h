#ifndef UI_CALENDAR_H
#define UI_CALENDAR_H

#include "lvgl.h"

// 初始化并显示日历应用
void ui_calendar_show(void);

// 隐藏并销毁日历应用（回收内存）
void ui_calendar_hide(void);

#endif // UI_CALENDAR_H
