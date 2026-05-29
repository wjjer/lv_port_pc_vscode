#ifndef UI_ALARM_CREATE_H
#define UI_ALARM_CREATE_H

#include "lvgl.h"

// 显示创建闹钟页面
void ui_alarm_create_show(void);

// 隐藏创建闹钟页面
void ui_alarm_create_hide(void);

// 获取新闹钟数据（由主页面调用）
typedef struct {
    uint8_t hour;
    uint8_t minute;
    bool active;
    bool repeat;
    char label[64];
    char ringtone[64];
} alarm_data_t;

// 检查是否有新闹钟数据
bool ui_alarm_create_has_data(void);

// 获取新闹钟数据（获取后自动清空）
void ui_alarm_create_get_data(alarm_data_t * out);

// 隐藏铃声选择页面
void ui_ringtone_page_hide(void);

#endif // UI_ALARM_CREATE_H
