#ifndef UI_COLOR_PICKER_H
#define UI_COLOR_PICKER_H

#include "lvgl.h"

/**
 * @brief 显示颜色选择器弹窗
 * @param initial_color 初始颜色
 * @param callback 用户选择颜色后的回调（用户传入的原始数据）
 * @param user_data 用户自定义数据
 */
void ui_color_picker_show(lv_color_t initial_color,
    void (*callback)(lv_color_t chosen_color, void * user_data),
    void * user_data);

#endif
